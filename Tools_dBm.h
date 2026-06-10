#pragma once
#include "Ui.h"
#include <math.h>
#include <stdlib.h>

class ToolDbm {
public:
  explicit ToolDbm(Ui &u): ui(u) {}

  void draw() {
    keypadMode = false;
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("dBm <-> Ватты", ILI9341_PURPLE);

    drawPicture(12, 38);
    ui.u8Text(150, 62, "dBm:", Ui::FONT_MED, ILI9341_WHITE);
    ui.button(200, 44, 108, 30, dbmLabel(), ILI9341_NAVY, ILI9341_CYAN, 2);

    ui.u8Text(150, 104, "Вт:", Ui::FONT_MED, ILI9341_WHITE);
    ui.button(200, 86, 108, 30, wLabel(), ILI9341_NAVY, ILI9341_CYAN, 2);

    ui.u8Text(12, 168, "P(W)=10^((dBm-30)/10)", Ui::FONT_SMALL, ILI9341_LIGHTGREY);
    ui.u8Text(12, 186, "dBm=10*log10(P)+30", Ui::FONT_SMALL, ILI9341_LIGHTGREY);
  }

  void onTouch(int x, int y) {
    if (keypadMode) { onPadTouch(x, y); return; }
    if (ui.hit(x, y, 200, 44, 108, 30)) { editDbm = true; openPad(dbmLabel()); return; }
    if (ui.hit(x, y, 200, 86, 108, 30)) { editDbm = false; openPad(wLabel()); return; }
  }

private:
  Ui &ui;
  bool keypadMode = false;
  bool editDbm = true;
  char in[20] = "0";
  double dbm = 0.0;
  double w = 0.001;

  void drawPicture(int x0, int y0) {
    ui.rect(x0, y0, 126, 90, ILI9341_WHITE);
    int cx = x0 + 63;
    ui.line(cx, y0 + 16, cx - 22, y0 + 78, ILI9341_CYAN);
    ui.line(cx, y0 + 16, cx + 22, y0 + 78, ILI9341_CYAN);
    ui.line(cx - 18, y0 + 66, cx + 18, y0 + 66, ILI9341_CYAN);
    ui.line(cx - 14, y0 + 54, cx + 14, y0 + 54, ILI9341_CYAN);
    ui.line(cx - 10, y0 + 42, cx + 10, y0 + 42, ILI9341_CYAN);
    ui.line(cx, y0 + 8, cx, y0 + 16, ILI9341_CYAN);
    ui.circle(cx, y0 + 8, 8, ILI9341_GREENYELLOW);
    ui.circle(cx, y0 + 8, 13, ILI9341_GREENYELLOW);
    ui.u8Text(x0 + 18, y0 + 86, "РФ мощность", Ui::FONT_SMALL, ILI9341_LIGHTGREY);
  }

  void openPad(const char* start) {
    keypadMode = true;
    strncpy(in, start, sizeof(in));
    in[sizeof(in)-1] = 0;
    drawPad();
  }

  void drawPad() {
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar(editDbm ? "Ввод dBm" : "Ввод Вт", ILI9341_NAVY);
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
            double v = atof(in);
            if (editDbm) {
              dbm = v;
              w = pow(10.0, (dbm - 30.0) / 10.0);
            } else {
              if (v <= 1e-12) v = 1e-12;
              w = v;
              dbm = 10.0 * log10(w) + 30.0;
            }
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
          if (len < (int)sizeof(in)-2) { in[len] = k[0]; in[len+1] = 0; drawPad(); }
          return;
        }
        idx++;
      }
    }
  }

  const char* dbmLabel() {
    static char b[16];
    snprintf(b, sizeof(b), "%.2f", dbm);
    return b;
  }
  const char* wLabel() {
    static char b[24];
    if (w >= 1.0) snprintf(b, sizeof(b), "%.3f", w);
    else snprintf(b, sizeof(b), "%.6f", w);
    return b;
  }
};
