#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "TouchCalib.h"


class Ui {
public:
  enum FontId { FONT_SMALL, FONT_MED, FONT_BIG };

  Ui(Adafruit_ILI9341 &t, XPT2046_Touchscreen &touch, U8G2_FOR_ADAFRUIT_GFX &u, TouchCalib &c)
  : tft(t), ts(touch), u8g2(u), calib(c) {}

  void initFonts() {
    fSmall = u8g2_font_6x12_t_cyrillic;
    fMed   = u8g2_font_10x20_t_cyrillic;
    fBig   = u8g2_font_10x20_t_cyrillic; 
    u8g2.setFontMode(1);
  }

  Adafruit_ILI9341& screen() { return tft; }
  void line(int x0, int y0, int x1, int y1, uint16_t c) { tft.drawLine(x0, y0, x1, y1, c); }
  void rect(int x, int y, int W, int H, uint16_t c) { tft.drawRect(x, y, W, H, c); }
  void fillRect(int x, int y, int W, int H, uint16_t c) { tft.fillRect(x, y, W, H, c); }
  void circle(int x, int y, int r, uint16_t c) { tft.drawCircle(x, y, r, c); }
  void fillCircle(int x, int y, int r, uint16_t c) { tft.fillCircle(x, y, r, c); }

  void topBar(const char* title, uint16_t color, bool showBack = true) {
    tft.fillRect(0, 0, w(), 30, color);
    tft.drawFastHLine(0, 30, w(), ILI9341_WHITE);
    if (showBack) {
      button(6, 2, 74, 26, "Назад", ILI9341_DARKGREY, ILI9341_WHITE, 1);
      u8CenterText(22, title, FONT_MED, ILI9341_WHITE, 84, w() - 4);
    } else {
      u8CenterText(22, title, FONT_MED, ILI9341_WHITE);
    }
  }

  int w() const { return tft.width(); }
  int h() const { return tft.height(); }

  bool hit(int x, int y, int rx, int ry, int rw, int rh) {
    return (x >= rx && x < rx + rw && y >= ry && y < ry + rh);
  }

  void button(int x, int y, int W, int H, const char* label, uint16_t bg, uint16_t fg = ILI9341_WHITE, uint8_t textSize = 2) {
    tft.fillRoundRect(x, y, W, H, 8, bg);
    tft.drawRoundRect(x, y, W, H, 8, ILI9341_WHITE);

    if (isAscii(label)) {
      tft.setTextColor(fg);
      tft.setTextSize(textSize);
      int16_t x1, y1; uint16_t tw, th;
      tft.getTextBounds(label, 0, 0, &x1, &y1, &tw, &th);
      int tx = x + (W - (int)tw) / 2;
      int ty = y + (H - (int)th) / 2;
      if (tx < x + 2) tx = x + 2;
      if (ty < y + 2) ty = y + 2;
      tft.setCursor(tx, ty);
      tft.print(label);
    } else {
      u8CenterText(y + H/2 + 6, label, FONT_MED, fg, x, x + W);
    }
  }

  void u8Text(int x, int yBaseline, const char* s, FontId id, uint16_t color) {
    u8g2.setFont(fontFor(id));
    u8g2.setFontMode(1);
    u8g2.setForegroundColor(color);
    u8g2.setCursor(x, yBaseline);
    u8g2.print(s);
  }

  void u8CenterText(int yBaseline, const char* s, FontId id, uint16_t color, int xL = 0, int xR = -1) {
    if (xR < 0) xR = w();
    u8g2.setFont(fontFor(id));
    u8g2.setFontMode(1);
    u8g2.setForegroundColor(color);
    int16_t tw = u8g2.getUTF8Width(s);
    int x = xL + ((xR - xL) - tw) / 2;
    if (x < xL) x = xL;
    u8g2.setCursor(x, yBaseline);
    u8g2.print(s);
  }

  void waitRelease() {
    while (ts.touched()) delay(5);
    delay(60);
  }

  bool getTouch(int &sx, int &sy) {
    if (!ts.touched()) return false;

    const int N = 10;
    long sumX = 0, sumY = 0;
    int cnt = 0;

    for (int i = 0; i < N; i++) {
      if (ts.touched()) {
        TS_Point p = ts.getPoint();
        sumX += p.x;
        sumY += p.y;
        cnt++;
      }
      delay(2);
    }
    if (cnt < 4) return false;

    int rawX = (int)(sumX / cnt);
    int rawY = (int)(sumY / cnt);

    const auto &c = calib.get();
    int ax = c.swapXY ? rawY : rawX;
    int ay = c.swapXY ? rawX : rawY;

    int x = c.invX ? map(ax, c.minX, c.maxX, w() - 1, 0)
                   : map(ax, c.minX, c.maxX, 0, w() - 1);
    int y = c.invY ? map(ay, c.minY, c.maxY, h() - 1, 0)
                   : map(ay, c.minY, c.maxY, 0, h() - 1);

    if (x < 0) x = 0; if (x >= w()) x = w() - 1;
    if (y < 0) y = 0; if (y >= h()) y = h() - 1;

    sx = x; sy = y;
    return true;
  }

  void runCalibrationWizard() {
    TouchCalibData &c = calib.getMutable();

    tft.fillScreen(ILI9341_BLACK);
    u8CenterText(80,  "Калибровка тача", FONT_BIG, ILI9341_CYAN);
    u8CenterText(115, "Нажимай крестики по углам", FONT_MED, ILI9341_WHITE);
    delay(800);

    Raw tl = captureCorner("Левый верх",  20, 20);
    Raw tr = captureCorner("Правый верх", w()-20, 20);
    Raw bl = captureCorner("Левый низ",   20, h()-20);
    Raw br = captureCorner("Правый низ",  w()-20, h()-20);

    auto scoreSwap = [&](bool swap)->long {
      int left  = ((swap?tl.y:tl.x) + (swap?bl.y:bl.x))/2;
      int right = ((swap?tr.y:tr.x) + (swap?br.y:br.x))/2;
      int top   = ((swap?tl.x:tl.y) + (swap?tr.x:tr.y))/2;
      int bot   = ((swap?bl.x:bl.y) + (swap?br.x:br.y))/2;
      return labs(right-left) + labs(bot-top);
    };

    bool swap = scoreSwap(true) > scoreSwap(false);
    c.swapXY = swap;

    int left  = ((swap?tl.y:tl.x) + (swap?bl.y:bl.x))/2;
    int right = ((swap?tr.y:tr.x) + (swap?br.y:br.x))/2;
    int top   = ((swap?tl.x:tl.y) + (swap?tr.x:tr.y))/2;
    int bot   = ((swap?bl.x:bl.y) + (swap?br.x:br.y))/2;

    c.invX = right < left;
    c.invY = bot < top;

    c.minX = min(left, right);
    c.maxX = max(left, right);
    c.minY = min(top, bot);
    c.maxY = max(top, bot);

    c.valid = true;
    calib.save();

    tft.fillScreen(ILI9341_BLACK);
    u8CenterText(120, "Готово", FONT_BIG, ILI9341_GREENYELLOW);
    delay(700);
  }

private:
  Adafruit_ILI9341 &tft;
  XPT2046_Touchscreen &ts;
  U8G2_FOR_ADAFRUIT_GFX &u8g2;
  TouchCalib &calib;

  const uint8_t *fSmall = nullptr;
  const uint8_t *fMed = nullptr;
  const uint8_t *fBig = nullptr;

  struct Raw { int x; int y; };

  const uint8_t* fontFor(FontId id) const {
    if (id == FONT_SMALL) return fSmall;
    if (id == FONT_MED) return fMed;
    return fBig;
  }

  bool isAscii(const char* s) {
    for (const unsigned char* p = (const unsigned char*)s; *p; p++) {
      if (*p >= 0x80) return false;
    }
    return true;
  }

  void drawCross(int x, int y, uint16_t c) {
    tft.drawLine(x-10, y, x+10, y, c);
    tft.drawLine(x, y-10, x, y+10, c);
    tft.drawCircle(x, y, 14, c);
  }

  Raw readStableRaw() {
    const int N = 15;
    long sx = 0, sy = 0; int cnt = 0; 
    while (!ts.touched()) delay(5);
    delay(35);
    for (int i = 0; i < N; i++) {
      if (ts.touched()) {
        TS_Point p = ts.getPoint();
        sx += p.x; sy += p.y; cnt++;
      }
      delay(3);
    }
    waitRelease();
    if (cnt == 0) return {0,0};
    return {(int)(sx/cnt), (int)(sy/cnt)};
  }

  Raw captureCorner(const char* name, int x, int y) {
    tft.fillScreen(ILI9341_BLACK);
    u8CenterText(40, "Калибровка", FONT_MED, ILI9341_WHITE);
    u8CenterText(70, name, FONT_MED, ILI9341_CYAN);
    u8Text(10, 110, "Нажми и удерживай крестик", FONT_SMALL, ILI9341_LIGHTGREY);
    drawCross(x, y, ILI9341_YELLOW);
    Raw r = readStableRaw();
    tft.fillCircle(x, y, 6, ILI9341_GREENYELLOW);
    delay(200);
    return r;
  }
};