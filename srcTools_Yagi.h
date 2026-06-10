#pragma once
#include "Ui.h"
#include <math.h>
#include <stdlib.h>

class ToolYagi {
public:
  explicit ToolYagi(Ui &u): ui(u) {}

  void draw() {
    keypadMode = false;
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("Yagi", 0x0010);

    char f[16]; dtostrf(freqMHz, 0, 3, f);
    ui.u8Text(12, 62, "Частота МГц", Ui::FONT_SMALL, ILI9341_WHITE);
    ui.button(12, 68, 108, 30, f, ILI9341_NAVY, ILI9341_CYAN, 2);
    ui.button(126, 68, 60, 30, unitsCm ? "См" : "М", ILI9341_DARKCYAN, ILI9341_WHITE, 2);

    ui.u8Text(196, 62, "Директоры", Ui::FONT_SMALL, ILI9341_WHITE);
    ui.button(196, 68, 44, 30, "-", ILI9341_MAROON);
    ui.button(244, 68, 28, 30, dirLabel(), ILI9341_DARKGREY, ILI9341_WHITE, 2);
    ui.button(276, 68, 32, 30, "+", ILI9341_DARKGREEN);

    drawDiagram(12, 106);
    render();
  }

  void onTouch(int x, int y) {
    if (keypadMode) { onPadTouch(x, y); return; }
    if (ui.hit(x, y, 12, 68, 108, 30)) { openPad(); return; }
    if (ui.hit(x, y, 126, 68, 60, 30)) { unitsCm = !unitsCm; draw(); return; }
    if (ui.hit(x, y, 196, 68, 44, 30)) { if (directors > 1) directors--; draw(); return; }
    if (ui.hit(x, y, 276, 68, 32, 30)) { if (directors < 6) directors++; draw(); return; }
  }

private:
  Ui &ui;
  float freqMHz = 145.0f;
  bool unitsCm = true;
  int directors = 1;

  bool keypadMode = false;
  char in[20] = "145.0";

  const char* dirLabel() {
    static char b[8];
    snprintf(b, sizeof(b), "%d", directors);
    return b;
  }

  void render() {
    float f = freqMHz; if (f < 0.01f) f = 0.01f;
    float lambda = 300.0f / f;

    float refl = 0.53f * lambda;
    float de   = 0.50f * lambda;
    float d1   = 0.47f * lambda;
    float sRefDe = 0.20f * lambda;
    float sDir = 0.15f * lambda;
    float gainDb = 4.8f + (directors - 1) * 1.1f;

    float k = unitsCm ? 100.0f : 1.0f;
    const char* unit = unitsCm ? "см" : "м";

    char b1[20], b2[20], b3[20], b4[20], b5[20], b6[20];
    dtostrf(refl * k, 0, unitsCm ? 1 : 3, b1);
    dtostrf(de   * k, 0, unitsCm ? 1 : 3, b2);
    dtostrf(d1   * k, 0, unitsCm ? 1 : 3, b3);
    dtostrf(sRefDe * k, 0, unitsCm ? 1 : 3, b4);
    dtostrf(sDir * k, 0, unitsCm ? 1 : 3, b5);
    dtostrf(gainDb, 0, 1, b6);

    ui.u8Text(170, 114, "Рефлектор:", Ui::FONT_SMALL, ILI9341_LIGHTGREY);
    ui.u8Text(170, 130, b1, Ui::FONT_SMALL, ILI9341_GREENYELLOW);
    ui.u8Text(232, 130, unit, Ui::FONT_SMALL, ILI9341_GREENYELLOW);

    ui.u8Text(170, 150, "Вибратор:", Ui::FONT_SMALL, ILI9341_LIGHTGREY);
    ui.u8Text(170, 166, b2, Ui::FONT_SMALL, ILI9341_GREENYELLOW);
    ui.u8Text(232, 166, unit, Ui::FONT_SMALL, ILI9341_GREENYELLOW);

    ui.u8Text(170, 186, "Директор 1:", Ui::FONT_SMALL, ILI9341_LIGHTGREY);
    ui.u8Text(170, 202, b3, Ui::FONT_SMALL, ILI9341_GREENYELLOW);
    ui.u8Text(232, 202, unit, Ui::FONT_SMALL, ILI9341_GREENYELLOW);

    ui.u8Text(12, 236, "R-DE:", Ui::FONT_SMALL, ILI9341_LIGHTGREY);
    ui.u8Text(52, 236, b4, Ui::FONT_SMALL, ILI9341_GREENYELLOW);
    ui.u8Text(92, 236, unit, Ui::FONT_SMALL, ILI9341_GREENYELLOW);
    ui.u8Text(120, 236, "D-D:", Ui::FONT_SMALL, ILI9341_LIGHTGREY);
    ui.u8Text(156, 236, b5, Ui::FONT_SMALL, ILI9341_GREENYELLOW);
    ui.u8Text(196, 236, unit, Ui::FONT_SMALL, ILI9341_GREENYELLOW);
    ui.u8Text(222, 236, "G:", Ui::FONT_SMALL, ILI9341_LIGHTGREY);
    ui.u8Text(238, 236, b6, Ui::FONT_SMALL, ILI9341_CYAN);
    ui.u8Text(272, 236, "dBi", Ui::FONT_SMALL, ILI9341_CYAN);
  }

  void drawDiagram(int x0, int y0) {
    ui.rect(x0, y0, 150, 126, ILI9341_WHITE);
    int by = y0 + 63;
    ui.line(x0 + 10, by, x0 + 140, by, ILI9341_WHITE);

    int xRef = x0 + 24;
    int xDe  = x0 + 56;
    ui.line(xRef, by - 34, xRef, by + 34, ILI9341_WHITE);
    ui.line(xDe,  by - 30, xDe,  by + 30, ILI9341_CYAN);

    for (int i = 0; i < directors; i++) {
      int xd = x0 + 86 + i * 16;
      if (xd > x0 + 138) break;
      int len = 26 - i * 2;
      if (len < 16) len = 16;
      ui.line(xd, by - len, xd, by + len, ILI9341_GREENYELLOW);
    }

    ui.u8Text(x0 + 8, y0 + 14, "R", Ui::FONT_SMALL, ILI9341_WHITE);
    ui.u8Text(x0 + 42, y0 + 14, "DE", Ui::FONT_SMALL, ILI9341_CYAN);
    ui.u8Text(x0 + 86, y0 + 14, "D1..Dn", Ui::FONT_SMALL, ILI9341_GREENYELLOW);
  }

  void openPad() {
    keypadMode = true;
    dtostrf(freqMHz, 0, 3, in);
    drawPad();
  }

  void drawPad() {
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("Ввод частоты", ILI9341_NAVY);
    ui.button(12, 38, 296, 34, in, ILI9341_DARKGREY, ILI9341_WHITE, 2);
    const char* keys[12] = {"1","2","3","4","5","6","7","8","9","DEL","0","OK"};
    int idx = 0;
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 3; c++) {
        ui.button(18 + c * 100, 70 + r * 40, 92, 34, keys[idx], ILI9341_DARKCYAN);
        idx++;
      }
    }
  }

  void onPadTouch(int x, int y) {
    const char* keys[12] = {"1","2","3","4","5","6","7","8","9","DEL","0","OK"};
    int idx = 0;
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 3; c++) {
        int bx = 18 + c * 100, by = 70 + r * 40;
        if (ui.hit(x, y, bx, by, 92, 34)) {
          const char* k = keys[idx];
          if (strcmp(k, "OK") == 0) {
            float v = atof(in);
            if (v < 1.0f) v = 1.0f;
            freqMHz = v;
            draw();
            return;
          }
          if (strcmp(k, "DEL") == 0) {
            int len = strlen(in);
            if (len > 0) in[len - 1] = 0;
            drawPad();
            return;
          }
          int len = strlen(in);
          if (len < (int)sizeof(in) - 2) { in[len] = k[0]; in[len + 1] = 0; drawPad(); }
          return;
        }
        idx++;
      }
    }
  }
};
