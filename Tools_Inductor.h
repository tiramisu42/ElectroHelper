#pragma once
#include "Ui.h"
#include <stdlib.h>
#include <math.h>

class ToolInductor {
public:
  explicit ToolInductor(Ui &u): ui(u) {}

  void draw() {
    keypadMode = false;
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("Катушка (индуктивность)", ILI9341_OLIVE);

    drawCoilDiagram(10, 36);
    ui.button(176, 42, 132, 30, valD(), ILI9341_DARKGREY, ILI9341_CYAN, 2);
    ui.button(176, 78, 132, 30, valL(), ILI9341_DARKGREY, ILI9341_CYAN, 2);
    ui.button(176, 114, 132, 30, valN(), ILI9341_DARKGREY, ILI9341_CYAN, 2);

    ui.u8Text(178, 40, "D мм", Ui::FONT_SMALL, ILI9341_WHITE);
    ui.u8Text(178, 76, "L мм", Ui::FONT_SMALL, ILI9341_WHITE);
    ui.u8Text(178, 112, "N витков", Ui::FONT_SMALL, ILI9341_WHITE);

    renderResult();
  }

  void onTouch(int x, int y) {
    if (keypadMode) { onPadTouch(x, y); return; }
    if (ui.hit(x, y, 176, 42, 132, 30)) { field = 0; openPad(); return; }
    if (ui.hit(x, y, 176, 78, 132, 30)) { field = 1; openPad(); return; }
    if (ui.hit(x, y, 176, 114, 132, 30)) { field = 2; openPad(); return; }
  }

private:
  Ui &ui;
  float Dmm = 20.0f;
  float Lmm = 30.0f;
  int N = 20;
  bool keypadMode = false;
  int field = 0;
  char in[16] = "0";

  const char* valD() { static char b[16]; dtostrf(Dmm, 0, 2, b); return b; }
  const char* valL() { static char b[16]; dtostrf(Lmm, 0, 2, b); return b; }
  const char* valN() { static char b[16]; snprintf(b, sizeof(b), "%d", N); return b; }

  void drawCoilDiagram(int x0, int y0) {
    ui.rect(x0, y0, 160, 120, ILI9341_WHITE);
    int y = y0 + 58;
    ui.line(x0 + 8, y, x0 + 152, y, ILI9341_DARKGREY);
    for (int i = 0; i < 8; i++) {
      int cx = x0 + 20 + i * 16;
      ui.circle(cx, y, 10, ILI9341_WHITE);
    }
    ui.line(x0 + 20, y0 + 20, x0 + 132, y0 + 20, ILI9341_CYAN);
    ui.line(x0 + 20, y0 + 16, x0 + 20, y0 + 24, ILI9341_CYAN);
    ui.line(x0 + 132, y0 + 16, x0 + 132, y0 + 24, ILI9341_CYAN);
    ui.u8Text(x0 + 65, y0 + 16, "L", Ui::FONT_SMALL, ILI9341_CYAN);

    ui.line(x0 + 144, y0 + 44, x0 + 144, y0 + 74, ILI9341_GREENYELLOW);
    ui.line(x0 + 140, y0 + 44, x0 + 148, y0 + 44, ILI9341_GREENYELLOW);
    ui.line(x0 + 140, y0 + 74, x0 + 148, y0 + 74, ILI9341_GREENYELLOW);
    ui.u8Text(x0 + 148, y0 + 62, "D", Ui::FONT_SMALL, ILI9341_GREENYELLOW);
  }

  void renderResult() {
    float d_in = Dmm / 25.4f;
    float l_in = Lmm / 25.4f;
    float L_uH = (d_in * d_in * (float)N * (float)N) / (18.0f * d_in + 40.0f * l_in);
    char b[20]; dtostrf(L_uH, 0, 3, b);
    ui.u8Text(12, 184, "L ≈", Ui::FONT_BIG, ILI9341_GREENYELLOW);
    ui.u8Text(52, 184, b, Ui::FONT_BIG, ILI9341_GREENYELLOW);
    ui.u8Text(140, 184, "мкГн", Ui::FONT_BIG, ILI9341_GREENYELLOW);
  }

  void openPad() {
    keypadMode = true;
    if (field == 0) strncpy(in, valD(), sizeof(in));
    else if (field == 1) strncpy(in, valL(), sizeof(in));
    else strncpy(in, valN(), sizeof(in));
    in[sizeof(in) - 1] = 0;
    drawPad();
  }

  void drawPad() {
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("Ввод значения", ILI9341_NAVY);
    ui.button(12, 38, 296, 34, in, ILI9341_DARKGREY, ILI9341_WHITE, 2);
    const char* keys[12] = {"1","2","3","4","5","6","7","8","9","DEL","0","OK"};
    int idx = 0;
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 3; c++) {
        ui.button(18 + c * 100, 78 + r * 40, 92, 34, keys[idx], ILI9341_DARKCYAN);
        idx++;
      }
    }
  }

  void onPadTouch(int x, int y) {
    const char* keys[12] = {"1","2","3","4","5","6","7","8","9","DEL","0","OK"};
    int idx = 0;
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 3; c++) {
        int bx = 18 + c * 100, by = 78 + r * 40;
        if (ui.hit(x, y, bx, by, 92, 34)) {
          const char* k = keys[idx];
          if (strcmp(k, "OK") == 0) {
            float v = atof(in);
            if (field == 0) { if (v < 0.1f) v = 0.1f; Dmm = v; }
            else if (field == 1) { if (v < 0.1f) v = 0.1f; Lmm = v; }
            else { int n = (int)round(v); if (n < 1) n = 1; if (n > 999) n = 999; N = n; }
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
