#pragma once
#include "Ui.h"

class ToolResistor {
public:
  explicit ToolResistor(Ui &u): ui(u) {}

  void draw() {
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("Резистор", ILI9341_PURPLE);

    ui.button(12, 36, 64, 30, "<", ILI9341_MAROON);
    ui.button(82, 36, 64, 30, ">", ILI9341_DARKGREEN);
    ui.button(152, 36, 64, 30, "-", ILI9341_MAROON);
    ui.button(222, 36, 64, 30, "+", ILI9341_DARKGREEN);

    drawBody();
    drawInfo();
  }

  void onTouch(int x, int y) {
    int maxBand = is5band ? 5 : 4;
    if (ui.hit(x, y, 12, 206, 96, 28)) { is5band = !is5band; selBand = 0; draw(); return; }
    if (ui.hit(x, y, 12, 36, 64, 30)) { selBand = (selBand + maxBand - 1) % maxBand; draw(); return; }
    if (ui.hit(x, y, 82, 36, 64, 30)) { selBand = (selBand + 1) % maxBand; draw(); return; }
    if (ui.hit(x, y, 152, 36, 64, 30)) { change(-1); draw(); return; }
    if (ui.hit(x, y, 222, 36, 64, 30)) { change(+1); draw(); return; }
  }

private:
  Ui &ui;
  bool is5band = false;
  int selBand = 0;
  int d1 = 1, d2 = 0, d3 = 0, mul = 2, tol = 0; // tol: 0..3

  uint16_t dColor(int v) {
    uint16_t c[10] = {ILI9341_BLACK,0x8200,ILI9341_RED,0xFD20,ILI9341_YELLOW,ILI9341_GREEN,ILI9341_BLUE,0x8010,0xC618,ILI9341_WHITE};
    return c[v % 10];
  }
  uint16_t tColor(int t) {
    uint16_t c[4] = {0xFEA0,0xC618,0x8200,ILI9341_RED}; // gold, silver, brown, red
    return c[(t + 4) % 4];
  }
  const char* tText() {
    const char* t[4] = {"±5%", "±10%", "±1%", "±2%"};
    return t[(tol + 4) % 4];
  }

  void drawBody() {
    int x0 = 120, y0 = 82, bw = 170, bh = 66;
    ui.line(20, y0 + 33, x0, y0 + 33, ILI9341_LIGHTGREY);
    ui.line(x0 + bw, y0 + 33, 308, y0 + 33, ILI9341_LIGHTGREY);
    ui.screen().fillRoundRect(x0, y0, bw, bh, 16, 0xFD20);
    ui.screen().drawRoundRect(x0, y0, bw, bh, 16, ILI9341_WHITE);

    if (!is5band) {
      int bx[4] = {x0 + 18, x0 + 42, x0 + 72, x0 + 120};
      uint16_t bc[4] = {dColor(d1), dColor(d2), dColor(mul), tColor(tol)};
      for (int i=0;i<4;i++) ui.screen().fillRect(bx[i], y0 + 2, 12, bh - 4, bc[i]);
      ui.screen().drawRect(bx[selBand] - 2, y0 - 2, 16, bh + 4, ILI9341_CYAN);
    } else {
      int bx[5] = {x0 + 14, x0 + 34, x0 + 54, x0 + 84, x0 + 124};
      uint16_t bc[5] = {dColor(d1), dColor(d2), dColor(d3), dColor(mul), tColor(tol)};
      for (int i=0;i<5;i++) ui.screen().fillRect(bx[i], y0 + 2, 10, bh - 4, bc[i]);
      ui.screen().drawRect(bx[selBand] - 2, y0 - 2, 14, bh + 4, ILI9341_CYAN);
    }
  }

  void drawInfo() {
    unsigned long base = is5band ? (unsigned long)(d1 * 100 + d2 * 10 + d3) : (unsigned long)(d1 * 10 + d2);
    unsigned long r = base;
    for (int i=0;i<mul;i++) r *= 10UL;

    ui.u8Text(12, 78, is5band ? "5 полос" : "4 полосы", Ui::FONT_MED, ILI9341_WHITE);
    ui.u8Text(12, 106, "Выбор полосы: < >", Ui::FONT_SMALL, ILI9341_LIGHTGREY);
    ui.u8Text(12, 124, "Смена цвета: - +", Ui::FONT_SMALL, ILI9341_LIGHTGREY);

    char b[40];
    if (r >= 1000000UL) snprintf(b, sizeof(b), "R = %.3f МОм", r / 1000000.0f);
    else if (r >= 1000UL) snprintf(b, sizeof(b), "R = %.3f кОм", r / 1000.0f);
    else snprintf(b, sizeof(b), "R = %lu Ом", r);
    ui.u8Text(12, 170, b, Ui::FONT_MED, ILI9341_GREENYELLOW);
    ui.u8Text(12, 192, tText(), Ui::FONT_MED, ILI9341_CYAN);

    ui.button(12, 206, 96, 28, is5band ? "Перекл 4" : "Перекл 5", ILI9341_DARKCYAN, ILI9341_WHITE, 1);
  }

  void change(int d) {
    int maxBand = is5band ? 5 : 4;
    if (selBand >= maxBand) selBand = maxBand - 1;
    if (!is5band) {
      if (selBand == 0) d1 = (d1 + d + 10) % 10;
      else if (selBand == 1) d2 = (d2 + d + 10) % 10;
      else if (selBand == 2) mul = (mul + d + 10) % 10;
      else if (selBand == 3) tol = (tol + d + 4) % 4;
    } else {
      if (selBand == 0) d1 = (d1 + d + 10) % 10;
      else if (selBand == 1) d2 = (d2 + d + 10) % 10;
      else if (selBand == 2) d3 = (d3 + d + 10) % 10;
      else if (selBand == 3) mul = (mul + d + 10) % 10;
      else if (selBand == 4) tol = (tol + d + 4) % 4;
    }
  }
};
