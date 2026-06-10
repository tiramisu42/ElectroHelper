#pragma once
#include "Ui.h"
#include <stdlib.h>
#include <string.h>

class ToolAdcPlot {
public:
  ToolAdcPlot(Ui &u, int pin): ui(u), adcPin(pin) {}
  void setSampleRateHz(int hz) { sampleHz = hz; }

  void draw() {
    probeEditMode = false;
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("Осциллограф", ILI9341_DARKCYAN);

    ui.button(12, 34, 70, 28, running ? "HOLD" : "RUN", ILI9341_MAROON, ILI9341_WHITE, 1);
    ui.button(88, 34, 40, 28, "-", ILI9341_DARKGREY);
    ui.button(132, 34, 96, 28, timebaseLabel(), ILI9341_NAVY, ILI9341_CYAN, 1);
    ui.button(232, 34, 40, 28, "+", ILI9341_DARKGREY);
    ui.button(276, 34, 32, 28, probeLabel(), ILI9341_DARKGREEN, ILI9341_WHITE, 1);

    gx = 12; gy = 68; gw = ui.w() - 24; gh = ui.h() - 80;
    drawGrid();
    idx = 0;
    triggered = false;
    lastMs = millis();
  }

  void onTouch(int xT, int yT) {
    if (probeEditMode) { onProbePadTouch(xT, yT); return; }
    if (ui.hit(xT, yT, 12, 34, 70, 28)) { running = !running; draw(); return; }
    if (ui.hit(xT, yT, 88, 34, 40, 28)) { stepTimebase(-1); draw(); return; }
    if (ui.hit(xT, yT, 232, 34, 40, 28)) { stepTimebase(+1); draw(); return; }
    if (ui.hit(xT, yT, 276, 34, 32, 28)) { openProbePad(); return; }
  }

  void tick() {
    if (!running || probeEditMode) return;

    uint32_t now = millis();
    uint32_t period = (sampleHz <= 0) ? 8 : (1000UL / (uint32_t)sampleHz);
    if (now - lastMs < period) return;
    lastMs = now;

    int adc = analogRead(adcPin); // 0..4095
    int y = mapAdcToY(adc);

    if (!triggered) {
      int mid = gy + gh / 2;
      if (prevRawY > mid && y <= mid) {
        triggered = true;
        idx = 0;
        clearTraceArea();
      }
      prevRawY = y;
      return;
    }

    plot(y);
  }

private:
  Ui &ui;
  int adcPin;
  int sampleHz = 120;
  bool running = true;

  int gx = 0, gy = 0, gw = 0, gh = 0;
  int idx = 0;
  int prevY = -1;
  int prevRawY = 0;
  bool triggered = false;
  uint32_t lastMs = 0;

  int tbIndex = 2; // 0..4
  int tbTable[5] = {80, 120, 180, 260, 360};

  int probeRatio = 10; // 1:10 
  bool probeEditMode = false;
  char probeInput[8] = "10";

  void stepTimebase(int d) {
    tbIndex += d;
    if (tbIndex < 0) tbIndex = 0;
    if (tbIndex > 4) tbIndex = 4;
    sampleHz = tbTable[tbIndex];
  }

  const char* timebaseLabel() {
    static char b[24];
    snprintf(b, sizeof(b), "TB %dHz", tbTable[tbIndex]);
    return b;
  }

  const char* probeLabel() {
    static char b[8];
    snprintf(b, sizeof(b), "x%d", probeRatio);
    return b;
  }

  int mapAdcToY(int adc) {

    int centered = adc - 2048;
    long scaledCentered = (long)centered * (long)probeRatio;
    long scaledAdc = 2048L + scaledCentered;
    if (scaledAdc < 0) scaledAdc = 0;
    if (scaledAdc > 4095) scaledAdc = 4095;

    int y = gy + gh - 2 - ((int)scaledAdc * (gh - 4) / 4095);
    if (y < gy + 1) y = gy + 1;
    if (y > gy + gh - 2) y = gy + gh - 2;
    return y;
  }

  void clearTraceArea() {
    ui.fillRect(gx + 1, gy + 1, gw - 2, gh - 2, ILI9341_BLACK);
    drawGridLines();
    prevY = -1;
  }

  void drawGrid() {
    ui.rect(gx, gy, gw, gh, ILI9341_WHITE);
    drawGridLines();
  }

  void drawGridLines() {
    uint16_t gridC = 0x39E7;
    for (int x = gx + 1; x < gx + gw - 1; x += (gw - 2) / 10) {
      ui.line(x, gy + 1, x, gy + gh - 2, gridC);
    }
    for (int y = gy + 1; y < gy + gh - 1; y += (gh - 2) / 8) {
      ui.line(gx + 1, y, gx + gw - 2, y, gridC);
    }
    ui.line(gx + 1, gy + gh / 2, gx + gw - 2, gy + gh / 2, ILI9341_DARKCYAN);
  }

  void plot(int y) {
    int x = gx + 1 + idx;
    if (x >= gx + gw - 1) {
      idx = 0;
      clearTraceArea();
      x = gx + 1;
    }
    if (prevY >= 0) ui.line(x - 1, prevY, x, y, ILI9341_GREENYELLOW);
    else ui.screen().drawPixel(x, y, ILI9341_GREENYELLOW);
    prevY = y;
    idx++;
  }

  void openProbePad() {
    probeEditMode = true;
    snprintf(probeInput, sizeof(probeInput), "%d", probeRatio);
    drawProbePad();
  }

  void drawProbePad() {
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("Щуп 1:X", ILI9341_NAVY);
    ui.button(12, 36, 296, 30, probeInput, ILI9341_DARKGREY, ILI9341_WHITE, 2);

    const char* keys[12] = {"1","2","3","4","5","6","7","8","9","DEL","0","OK"};
    int idxK = 0;
    int sx = 18, sy = 72, bw = 92, bh = 34, gap = 8;
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 3; c++) {
        ui.button(sx + c * (bw + gap), sy + r * (bh + gap), bw, bh, keys[idxK], ILI9341_DARKCYAN);
        idxK++;
      }
    }
    ui.u8Text(12, 232, "Пример: 1, 10, 100", Ui::FONT_SMALL, ILI9341_LIGHTGREY);
  }

  void onProbePadTouch(int x, int y) {
    const char* keys[12] = {"1","2","3","4","5","6","7","8","9","DEL","0","OK"};
    int idxK = 0;
    int sx = 18, sy = 72, bw = 92, bh = 34, gap = 8;
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 3; c++) {
        int bx = sx + c * (bw + gap);
        int by = sy + r * (bh + gap);
        if (ui.hit(x, y, bx, by, bw, bh)) {
          const char* k = keys[idxK];
          if (strcmp(k, "OK") == 0) {
            int v = atoi(probeInput);
            if (v < 1) v = 1;
            if (v > 1000) v = 1000;
            probeRatio = v;
            draw();
            return;
          }
          if (strcmp(k, "DEL") == 0) {
            int len = strlen(probeInput);
            if (len > 0) probeInput[len - 1] = 0;
            if (strlen(probeInput) == 0) strcpy(probeInput, "0");
            drawProbePad();
            return;
          }
          int len = strlen(probeInput);
          if (len < (int)sizeof(probeInput) - 2) {
            if (strcmp(probeInput, "0") == 0) probeInput[0] = 0;
            probeInput[len] = k[0];
            probeInput[len + 1] = 0;
          }
          drawProbePad();
          return;
        }
        idxK++;
      }
    }
  }
};
