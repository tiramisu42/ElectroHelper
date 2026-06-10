#pragma once
#include "Ui.h"
#include <string.h>
#include <stdlib.h>

class ToolCapacitor {
public:
  explicit ToolCapacitor(Ui &u): ui(u) {}

  void draw() {
    keypadMode = false;
    letterMode = false;
    helpMode = false;

    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("Конденсатор", ILI9341_MAROON);

    ui.u8Text(12, 66, "Код:", Ui::FONT_MED, ILI9341_WHITE);
    ui.button(12, 72, 126, 36, code, ILI9341_DARKGREY, ILI9341_CYAN, 2);

    ui.u8Text(146, 66, "Буква U:", Ui::FONT_MED, ILI9341_WHITE);
    ui.button(146, 72, 60, 36, vLetter, ILI9341_DARKGREY, ILI9341_CYAN, 2);
    ui.button(214, 72, 94, 36, "Справка", ILI9341_NAVY, ILI9341_WHITE, 1);

    drawCapPicture(12, 114);
    renderResult();
  }

  void onTouch(int x, int y) {
    if (helpMode) { draw(); return; }
    if (keypadMode) { onNumPadTouch(x, y); return; }
    if (letterMode) { onLetterTouch(x, y); return; }

    if (ui.hit(x, y, 12, 72, 126, 36)) { keypadMode = true; drawNumPad(); return; }
    if (ui.hit(x, y, 146, 72, 60, 36)) { letterMode = true; drawLetterPad(); return; }
    if (ui.hit(x, y, 214, 72, 94, 36)) { helpMode = true; drawHelp(); return; }
  }

private:
  Ui &ui;
  char code[8] = "102";
  char vLetter[2] = "A";

  bool keypadMode = false;
  bool letterMode = false;
  bool helpMode = false;

  void drawCapPicture(int x0, int y0) {
    ui.screen().fillRoundRect(x0, y0, 120, 56, 8, ILI9341_WHITE);
    ui.screen().fillRoundRect(x0 + 2, y0 + 2, 116, 52, 8, ILI9341_BLACK);
    ui.screen().drawRoundRect(x0, y0, 120, 56, 8, ILI9341_WHITE);
    ui.u8Text(x0 + 16, y0 + 28, "102", Ui::FONT_BIG, ILI9341_CYAN);
    ui.u8Text(x0 + 80, y0 + 28, "A", Ui::FONT_BIG, ILI9341_GREENYELLOW);
  }

  void renderResult() {
    float pf = decode3digitPf(code);
    char line1[40];
    formatCap(pf, line1, sizeof(line1));

    ui.u8Text(146, 126, "Результат:", Ui::FONT_MED, ILI9341_WHITE);
    ui.u8Text(146, 148, line1, Ui::FONT_MED, ILI9341_GREENYELLOW);
    ui.u8Text(146, 176, voltageText(vLetter[0]), Ui::FONT_SMALL, ILI9341_CYAN);
  }

  float decode3digitPf(const char* s) {
    if (strlen(s) != 3) return 0;
    if (s[0] < '0' || s[0] > '9' || s[1] < '0' || s[1] > '9' || s[2] < '0' || s[2] > '9') return 0;
    int xy = (s[0] - '0') * 10 + (s[1] - '0');
    int z = s[2] - '0';
    float pf = (float)xy;
    while (z-- > 0) pf *= 10.0f;
    return pf;
  }

  void formatCap(float pf, char* out, size_t n) {
    if (pf <= 0) { strncpy(out, "Неверный код", n); return; }
    if (pf >= 1e6f) snprintf(out, n, "%.3f мкФ", pf / 1e6f);
    else if (pf >= 1000.0f) snprintf(out, n, "%.3f нФ", pf / 1000.0f);
    else snprintf(out, n, "%.0f пФ", pf);
  }

  const char* voltageText(char c) {
    switch (c) {
      case 'A': return "A = 10 В";
      case 'B': return "B = 16 В";
      case 'C': return "C = 25 В";
      case 'D': return "D = 35 В";
      case 'E': return "E = 50 В";
      case 'F': return "F = 63 В";
      default: return "Зависит от серии";
    }
  }

  void drawNumPad() {
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("Ввод кода", ILI9341_NAVY);
    ui.button(12, 38, 296, 34, code, ILI9341_DARKGREY, ILI9341_WHITE, 2);

    const char* keys[12] = {"1","2","3","4","5","6","7","8","9","DEL","0","OK"};
    int idx = 0;
    int sx = 18, sy = 78, bw = 92, bh = 34, gap = 8;
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 3; c++) {
        ui.button(sx + c * (bw + gap), sy + r * (bh + gap), bw, bh, keys[idx], ILI9341_DARKCYAN);
        idx++;
      }
    }
  }

  void onNumPadTouch(int x, int y) {
    const char* keys[12] = {"1","2","3","4","5","6","7","8","9","DEL","0","OK"};
    int idx = 0;
    int sx = 18, sy = 78, bw = 92, bh = 34, gap = 8;
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 3; c++) {
        int bx = sx + c * (bw + gap), by = sy + r * (bh + gap);
        if (ui.hit(x, y, bx, by, bw, bh)) {
          const char* k = keys[idx];
          if (strcmp(k, "OK") == 0) { draw(); return; }
          if (strcmp(k, "DEL") == 0) {
            int len = strlen(code);
            if (len > 0) code[len - 1] = 0;
            drawNumPad();
            return;
          }
          size_t len = strlen(code);
          if (len < 3) { code[len] = k[0]; code[len + 1] = 0; }
          drawNumPad();
          return;
        }
        idx++;
      }
    }
  }

  void drawLetterPad() {
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("Выбор буквы", ILI9341_NAVY);
    ui.button(12, 38, 296, 34, vLetter, ILI9341_DARKGREY, ILI9341_WHITE, 2);

    const char* letters[7] = {"A","B","C","D","E","F","OK"};
    int i = 0;
    for (int r = 0; r < 3; r++) {
      for (int c = 0; c < 3; c++) {
        if (i >= 7) return;
        ui.button(32 + c * 90, 86 + r * 46, 80, 38, letters[i], ILI9341_DARKCYAN);
        i++;
      }
    }
  }

  void onLetterTouch(int x, int y) {
    const char* letters[7] = {"A","B","C","D","E","F","OK"};
    int i = 0;
    for (int r = 0; r < 3; r++) {
      for (int c = 0; c < 3; c++) {
        if (i >= 7) return;
        int bx = 32 + c * 90, by = 86 + r * 46;
        if (ui.hit(x, y, bx, by, 80, 38)) {
          if (strcmp(letters[i], "OK") == 0) { draw(); return; }
          vLetter[0] = letters[i][0];
          vLetter[1] = 0;
          drawLetterPad();
          return;
        }
        i++;
      }
    }
  }

  void drawHelp() {
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("Справка", ILI9341_NAVY);
    ui.u8Text(12, 66, "102 = 1 нФ", Ui::FONT_MED, ILI9341_WHITE);
    ui.u8Text(12, 90, "104 = 100 нФ = 0.1 мкФ", Ui::FONT_MED, ILI9341_WHITE);
    ui.u8Text(12, 122, "A=10В B=16В C=25В D=35В E=50В F=63В", Ui::FONT_SMALL, ILI9341_GREENYELLOW);
    ui.u8Text(12, 154, "Точные коды зависят от серии.", Ui::FONT_SMALL, ILI9341_LIGHTGREY);
  }
};
