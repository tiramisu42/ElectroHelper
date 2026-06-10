#pragma once
#include "Ui.h"
#include <stdlib.h>
#include <stdio.h>

class ToolDipole {
public:
  explicit ToolDipole(Ui &u): ui(u) {}

  void draw() {
    agentLog("H1", "Tools_Dipole.h:draw", "dipole_draw", "{\"mode\":\"main\"}");
    
    ui.topBar("Диполь", ILI9341_DARKGREEN);
    ui.fillRect(0, 31, ui.w(), ui.h()-31, ILI9341_BLACK);

    ui.u8Text(12, 54, "Частота (МГц):", Ui::FONT_MED, ILI9341_WHITE);
    char buf[20];
    dtostrf(freqMHz, 0, 3, buf);
    ui.button(12, 58, 296, 34, buf, ILI9341_NAVY, ILI9341_CYAN, 2);

    ui.button(12, 96, 140, 30, unitsCm ? "Сантиметры" : "Метры", ILI9341_DARKCYAN, ILI9341_WHITE, 1);

    drawAntennaDiagram(170, 96);
    renderResults();
  }

  void onTouch(int x, int y) {
    if (keypadMode) { onKeypadTouch(x,y); return; }

    if (ui.hit(x,y,12,58,296,34)) {
      dtostrf(freqMHz, 0, 3, input);
      keypadMode = true;
      drawKeypad();
      return;
    }
    if (ui.hit(x,y,12,96,140,30)) {
      unitsCm = !unitsCm;
      draw();
      return;
    }
  }

private:
  Ui &ui;
  float freqMHz = 145.0f;
  bool unitsCm = false;

  bool keypadMode = false;
  char input[20] = "145.0";

  
  void agentLog(const char* hypothesisId, const char* location, const char* message, const char* dataJson) {
    FILE* f = fopen("debug-33c8a0.log", "a");
    if (!f) return;
    fprintf(
      f,
      "{\"sessionId\":\"33c8a0\",\"runId\":\"pre-fix-1\",\"hypothesisId\":\"%s\",\"location\":\"%s\","
      "\"message\":\"%s\",\"data\":%s,\"timestamp\":%lu}\n",
      hypothesisId, location, message, dataJson, (unsigned long)millis()
    );
    fclose(f);
  }
  

  void renderResults() {
    float f = freqMHz;
    if (f < 0.01f) f = 0.01f;

    float half_m   = 150.0f / f;
    float quart_m  = 75.0f  / f;
    float eighth_m = 37.5f  / f;

    float k = unitsCm ? 100.0f : 1.0f;
    const char* unit = unitsCm ? "см" : "м";

    ui.u8Text(12, 148, "Размеры:", Ui::FONT_MED, ILI9341_WHITE);

    printLine(12, 166, "1/2 волны", half_m*k, unit);
    printLine(12, 186, "1/4 волны", quart_m*k, unit);
    printLine(170, 166, "1/8 волны", eighth_m*k, unit);
  }

  void printLine(int x, int y, const char* label, float v, const char* unit) {
    char buf[32];
    dtostrf(v, 0, unitsCm ? 1 : 3, buf);

    ui.u8Text(x, y, label, Ui::FONT_SMALL, ILI9341_LIGHTGREY);
    ui.u8Text(x, y+14, buf, Ui::FONT_MED, ILI9341_GREENYELLOW);
    ui.u8Text(x+80, y+14, unit, Ui::FONT_MED, ILI9341_GREENYELLOW);
  }

  void drawAntennaDiagram(int x0, int y0) {
    int y = y0 + 16;
    int xL = x0;
    int xR = x0 + 140;

    ui.u8Text(x0, y0+12, "Диполь", Ui::FONT_SMALL, ILI9341_WHITE);

    ui.line(xL, y, xR, y, ILI9341_WHITE);
    ui.fillRect((xL+xR)/2 - 2, y-8, 4, 16, ILI9341_WHITE);
    ui.line(xL, y-6, xL, y+6, ILI9341_WHITE);
    ui.line(xR, y-6, xR, y+6, ILI9341_WHITE);

    ui.line(xL, y+18, xR, y+18, ILI9341_CYAN);
    ui.line(xL, y+18, xL+6, y+14, ILI9341_CYAN);
    ui.line(xL, y+18, xL+6, y+22, ILI9341_CYAN);
    ui.line(xR, y+18, xR-6, y+14, ILI9341_CYAN);
    ui.line(xR, y+18, xR-6, y+22, ILI9341_CYAN);

    ui.u8Text(x0+30, y+38, "L = 1/2 волны", Ui::FONT_SMALL, ILI9341_CYAN);
  }

  void drawKeypad() {
    
    agentLog("H1", "Tools_Dipole.h:drawKeypad", "dipole_keypad_draw", "{\"x\":18,\"y\":104,\"btnW\":70,\"btnH\":34}");
    
    ui.screen().fillScreen(ILI9341_BLACK);
    ui.topBar("Ввод частоты", ILI9341_NAVY);

    ui.button(12, 36, 296, 30, input, ILI9341_DARKGREY, ILI9341_WHITE, 2);

    const char* keys[12] = {"1","2","3","4","5","6","7","8","9",".","0","C"};
    int sx = 18, sy = 72, bw = 70, bh = 30, gap = 6;
    int idx = 0;
    for (int r=0;r<4;r++){
      for (int c=0;c<3;c++){
        ui.button(sx + c*(bw+gap), sy + r*(bh+gap), bw, bh, keys[idx], ILI9341_DARKCYAN);
        idx++;
      }
    }
    ui.button(244, 72, 64, 30, "DEL", ILI9341_MAROON, ILI9341_WHITE, 1);
    ui.button(244, 108, 64, 30, "OK", ILI9341_DARKGREEN);
    ui.button(244, 144, 64, 30, "BACK", ILI9341_DARKGREY, ILI9341_WHITE, 1);
  }

  void appendChar(char c) {
    int len = strlen(input);
    if (len >= 18) return;
    if (c=='.' && strchr(input,'.')) return;

    if (strcmp(input,"0")==0 && c!='.') {
      input[0]=c; input[1]=0; return;
    }
    input[len]=c; input[len+1]=0;
  }

  void onKeypadTouch(int x,int y) {
    
    char d[64];
    snprintf(d, sizeof(d), "{\"x\":%d,\"y\":%d}", x, y);
    agentLog("H2", "Tools_Dipole.h:onKeypadTouch", "dipole_keypad_touch", d);
    
    if (ui.hit(x,y,244,144,64,30)) { keypadMode=false; draw(); return; }
    if (ui.hit(x,y,244,72,64,30)) {
      int len=strlen(input);
      if (len>1) input[len-1]=0; else strcpy(input,"0");
      drawKeypad(); return;
    }
    if (ui.hit(x,y,244,108,64,30)) {
      float v = atof(input);
      if (v < 0.01f) v = 0.01f;
      freqMHz = v;
      keypadMode=false;
      draw();
      return;
    }

    const char* keys[12] = {"1","2","3","4","5","6","7","8","9",".","0","C"};
    int sx = 18, sy = 72, bw = 70, bh = 30, gap = 6;
    int idx=0;
    for(int r=0;r<4;r++){
      for(int c=0;c<3;c++){
        int bx=sx + c*(bw+gap);
        int by=sy + r*(bh+gap);
        if (ui.hit(x,y,bx,by,bw,bh)) {
          char k = keys[idx][0];
          if (k=='C') strcpy(input,"0");
          else appendChar(k);
          drawKeypad();
          return;
        }
        idx++;
      }
    }
  }
};