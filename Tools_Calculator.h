#pragma once
#include "Ui.h"
#include <stdlib.h>
#include <string.h>

class ToolCalculator {
public:
  explicit ToolCalculator(Ui &u): ui(u) {}

  void draw() {
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("Калькулятор", ILI9341_DARKGREY);
    ui.button(12, 36, 296, 34, in, ILI9341_DARKGREY, ILI9341_WHITE, 2);

    const char* keys[16] = {
      "7","8","9","/",
      "4","5","6","*",
      "1","2","3","-",
      "C","0","=","+"
    };

    int idx = 0;
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        uint16_t col = (c == 3) ? ILI9341_NAVY : ILI9341_DARKCYAN;
        if (keys[idx][0] == '=') col = ILI9341_DARKGREEN;
        if (keys[idx][0] == 'C') col = ILI9341_MAROON;
        ui.button(12 + c * 74, 78 + r * 40, 68, 34, keys[idx], col);
        idx++;
      }
    }
  }

  void onTouch(int x, int y) {
    const char* keys[16] = {
      "7","8","9","/",
      "4","5","6","*",
      "1","2","3","-",
      "C","0","=","+"
    };

    int idx = 0;
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        int bx = 12 + c * 74, by = 78 + r * 40;
        if (ui.hit(x, y, bx, by, 68, 34)) {
          char k = keys[idx][0];
          if (k >= '0' && k <= '9') append(k);
          else if (k == 'C') reset();
          else if (k == '=') eval('=');
          else eval(k);
          draw();
          return;
        }
        idx++;
      }
    }
  }

private:
  Ui &ui;
  char in[24] = "0";
  double acc = 0.0;
  char op = 0;
  bool newNum = true;

  void reset() {
    strcpy(in, "0");
    acc = 0.0;
    op = 0;
    newNum = true;
  }

  void append(char c) {
    if (newNum) {
      in[0] = c;
      in[1] = 0;
      newNum = false;
      return;
    }
    int len = strlen(in);
    if (len < 22) {
      in[len] = c;
      in[len + 1] = 0;
    }
  }

  void eval(char nextOp) {
    double v = atof(in);

    if (op == 0) acc = v;
    else if (op == '+') acc += v;
    else if (op == '-') acc -= v;
    else if (op == '*') acc *= v;
    else if (op == '/') acc = (v == 0.0) ? 0.0 : (acc / v);

    snprintf(in, sizeof(in), "%.6g", acc);
    op = (nextOp == '=') ? 0 : nextOp;
    newNum = true;
  }
};
