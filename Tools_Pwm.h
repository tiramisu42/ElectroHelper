#pragma once
#include "Ui.h"
#include <math.h>

class ToolPwm {
public:
  ToolPwm(Ui &u, int pin, int ch): ui(u), pwmPin(pin), channel(ch) {}

  void begin() {
    ledcAttach(pwmPin, freqHz, 8); 
    apply();
  }

  void draw() {
    pinMenu = false;
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("ШИМ", ILI9341_DARKGREY);

    ui.u8Text(12, 62, "Пин GPIO:", Ui::FONT_MED, ILI9341_WHITE);
    ui.button(12, 68, 84, 30, pinLabel(), ILI9341_NAVY, ILI9341_CYAN, 2);
    ui.button(100, 68, 28, 30, "-", ILI9341_MAROON);
    ui.button(132, 68, 28, 30, "+", ILI9341_DARKGREEN);

    ui.u8Text(168, 62, "Частота (Гц):", Ui::FONT_MED, ILI9341_WHITE);
    ui.button(168, 68, 140, 30, freqLabel(), ILI9341_NAVY, ILI9341_CYAN, 2);

    ui.u8Text(12, 108, "Скважность (%):", Ui::FONT_MED, ILI9341_WHITE);
    ui.button(12, 114, 140, 34, dutyLabel(), ILI9341_NAVY, ILI9341_CYAN, 2);

    ui.button(168, 114, 70, 34, "-F", ILI9341_MAROON);
    ui.button(244, 114, 64, 34, "+F", ILI9341_DARKGREEN);
    ui.button(168, 154, 70, 34, "-D", ILI9341_MAROON);
    ui.button(244, 154, 64, 34, "+D", ILI9341_DARKGREEN);

    drawWaveform();
    apply();
  }

  void onTouch(int x,int y) {
    if (pinMenu) { onPinMenuTouch(x, y); return; }
    if (ui.hit(x,y,12,68,84,30)) { drawPinMenu(); return; }
    if (ui.hit(x,y,100,68,28,30)) { stepPin(-1); draw(); return; }
    if (ui.hit(x,y,132,68,28,30)) { stepPin(+1); draw(); return; }
    if (ui.hit(x,y,168,114,70,34)) { stepFreq(-1); draw(); return; }
    if (ui.hit(x,y,244,114,64,34)) { stepFreq(+1); draw(); return; }
    if (ui.hit(x,y,168,154,70,34)) { stepDuty(-5); draw(); return; }
    if (ui.hit(x,y,244,154,64,34)) { stepDuty(+5); draw(); return; }
  }

  void tick() {}

private:
  Ui &ui;
  int pwmPin, channel;
  int freqHz = 1000;
  int dutyPct = 50;
  bool pinMenu = false;
  int pinList[17] = {2,4,5,12,13,14,15,18,19,21,22,23,25,26,27,32,33};

  void apply() {
    ledcDetach(pwmPin);
    ledcAttach(pwmPin, freqHz, 8);
    int duty = (int)round((dutyPct / 100.0) * 255.0);
    if (duty < 0) duty = 0; if (duty > 255) duty = 255;
    ledcWrite(pwmPin, duty);
  }

  void stepPin(int dir) {
    int count = sizeof(pinList) / sizeof(pinList[0]);
    int idx = 0;
    for (int i = 0; i < count; i++) if (pinList[i] == pwmPin) { idx = i; break; }
    idx += dir;
    if (idx < 0) idx = count - 1;
    if (idx >= count) idx = 0;
    pwmPin = pinList[idx];
  }

  void stepFreq(int dir) {
    if (freqHz < 11) freqHz += dir * 1;
    else if (freqHz < 100) freqHz += dir * 10;
    else if (freqHz < 1000) freqHz += dir * 50;
    else if (freqHz < 10000) freqHz += dir * 200;
    else freqHz += dir * 1000;
    if (freqHz < 1) freqHz = 1;
    if (freqHz > 40000) freqHz = 40000;
  }

  void stepDuty(int d) {
    dutyPct += d;
    if (dutyPct < 0) dutyPct = 0;
    if (dutyPct > 100) dutyPct = 100;
  }

  const char* freqLabel() {
    static char b[16];
    snprintf(b, sizeof(b), "%d", freqHz);
    return b;
  }

  const char* dutyLabel() {
    static char b[16];
    snprintf(b, sizeof(b), "%d", dutyPct);
    return b;
  }

  const char* pinLabel() {
    static char b[8];
    snprintf(b, sizeof(b), "%d", pwmPin);
    return b;
  }

  void drawWaveform() {
    int x0 = 12, y0 = 194, W = ui.w()-24, H = 44;
    ui.rect(x0, y0, W, H, ILI9341_WHITE);

    int yHi = y0 + 10;
    int yLo = y0 + H - 10;
    int per = (W - 10) / 2;
    int on = (per * dutyPct) / 100;

    int x = x0 + 5;
    for (int p=0; p<2; p++) {
      ui.line(x, yLo, x, yHi, ILI9341_WHITE);
      ui.line(x, yHi, x+on, yHi, ILI9341_WHITE);
      ui.line(x+on, yHi, x+on, yLo, ILI9341_WHITE);
      ui.line(x+on, yLo, x+per, yLo, ILI9341_WHITE);
      x += per;
    }

    ui.u8Text(12, 238, "График меняется от частоты/скважности", Ui::FONT_SMALL, ILI9341_LIGHTGREY);
  }

  void drawPinMenu() {
    pinMenu = true;
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("Выбор GPIO", ILI9341_NAVY);
    int idx = 0;
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        if (idx >= 17) return;
        char b[8]; snprintf(b, sizeof(b), "%d", pinList[idx]);
        uint16_t col = (pinList[idx] == pwmPin) ? ILI9341_DARKGREEN : ILI9341_DARKCYAN;
        ui.button(18 + c * 74, 40 + r * 44, 68, 36, b, col);
        idx++;
      }
    }
    ui.button(18, 216, 290, 20, "OK", ILI9341_DARKGREY, ILI9341_WHITE, 1);
  }

  void onPinMenuTouch(int x, int y) {
    if (ui.hit(x, y, 18, 216, 290, 20)) { draw(); return; }
    int idx = 0;
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        if (idx >= 17) return;
        int bx = 18 + c * 74, by = 40 + r * 44;
        if (ui.hit(x, y, bx, by, 68, 36)) {
          pwmPin = pinList[idx];
          drawPinMenu();
          return;
        }
        idx++;
      }
    }
  }
};