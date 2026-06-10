#include <SPI.h>
#include <Preferences.h>
#include <stdio.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

#include <U8g2_for_Adafruit_GFX.h>

#include "TouchCalib.h"
#include "Ui.h"
#include "Tools_Dipole.h"
#include "Tools_AdcPlot.h"
#include "Tools_Capacitor.h"
#include "Tools_Pwm.h"
#include "Tools_Inductor.h"
#include "Tools_Resistor.h"
#include "srcTools_Yagi.h" 
#include "Tools_Calculator.h"

//экран
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  4

#define T_CS     21   

#define PIN_SCK  18
#define PIN_MISO 19
#define PIN_MOSI 23


static const int ROT = 1;              // экран горизонтально
static const int BOOT_GPIO = 0;        // кнопка BOOT (GPIO0)
static const int ADC_PIN = 34;         // график АЦП (GPIO34 input-only)
static const int PWM_PIN = 25;         // ШИМ 
static const int PWM_CH  = 0;          // канал ledc
static const uint16_t COLOR_DARKBLUE = 0x0010;

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(T_CS);
U8G2_FOR_ADAFRUIT_GFX u8g2;
Preferences prefs;

TouchCalib calib;
Ui ui(tft, ts, u8g2, calib);


enum ScreenId {
  SCR_MENU = 0,
  SCR_DIPOLE,
  SCR_ADC,
  SCR_CAP,
  SCR_PWM,
  SCR_IND,
  SCR_RES,
  SCR_YAGI,
  SCR_CALC
};

ScreenId screen = SCR_MENU;


ToolDipole toolDipole(ui);
ToolAdcPlot toolAdc(ui, ADC_PIN);
ToolCapacitor toolCap(ui);
ToolPwm toolPwm(ui, PWM_PIN, PWM_CH);
ToolInductor toolInd(ui);
ToolResistor toolRes(ui);
ToolYagi toolYagi(ui);
ToolCalculator toolCalc(ui);


static void agentLog(const char* hypothesisId, const char* location, const char* message, const char* dataJson) {
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


static void splash() {
  tft.fillScreen(ILI9341_BLACK);
  ui.topBar("Electro Helper", ILI9341_NAVY, false);
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(3);
  tft.setCursor(20, 92);
  tft.print("Electro Helper");
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(66, 145);
  tft.print("By @TiramisuFMB");
  delay(1400);
}

static void drawMenu() {
  tft.fillScreen(ILI9341_BLACK);
  ui.topBar("Меню", ILI9341_NAVY, false);

  agentLog("H4", "ElectroHelper.ino:drawMenu", "menu_drawn", "{\"hasResButton\":true}");

  ui.button(12, 44, 146, 44, "Диполь", ILI9341_DARKGREEN);
  ui.button(162, 44, 146, 44, "График АЦП", ILI9341_DARKCYAN);

  ui.button(12, 92, 146, 44, "Конденсатор", ILI9341_MAROON);
  ui.button(162, 92, 146, 44, "ШИМ", ILI9341_DARKGREY);

  ui.button(12, 140, 146, 44, "Катушка (L)", ILI9341_OLIVE);
  ui.button(162, 140, 146, 44, "Калькулятор", ILI9341_DARKGREY);
  ui.button(12, 188, 146, 44, "Резистор", ILI9341_PURPLE);
  ui.button(162, 188, 146, 44, "Yagi (3 эл.)", COLOR_DARKBLUE);
}

static void go(ScreenId s) {
  screen = s;
  switch (screen) {
    case SCR_MENU:  drawMenu(); break;
    case SCR_DIPOLE: toolDipole.draw(); break;
    case SCR_ADC:    toolAdc.draw(); break;
    case SCR_CAP:    toolCap.draw(); break;
    case SCR_PWM:    toolPwm.draw(); break;
    case SCR_IND:    toolInd.draw(); break;
    case SCR_RES:    toolRes.draw(); break;
    case SCR_YAGI:   toolYagi.draw(); break;
    case SCR_CALC:   toolCalc.draw(); break;
  }
}

void setup() {
  Serial.begin(115200);

  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI);

  tft.begin();
  tft.setRotation(ROT);

  ts.begin(); 

  u8g2.begin(tft);
  ui.initFonts();

  calib.begin(prefs);
  calib.load();

  pinMode(BOOT_GPIO, INPUT_PULLUP);
  delay(30);

  bool held = true;
  uint32_t t0 = millis();
  while (millis() - t0 < 2000) {
    if (digitalRead(BOOT_GPIO) != LOW) { held = false; break; }
    delay(10);
  }

  if (!calib.isValid() || held) {
    ui.runCalibrationWizard();
  }

  splash();
  drawMenu();

  toolAdc.setSampleRateHz(120);
  toolPwm.begin();
}

void loop() {
  int x, y;
  if (ui.getTouch(x, y)) {
    char touchData[80];
    snprintf(touchData, sizeof(touchData), "{\"x\":%d,\"y\":%d,\"screen\":%d}", x, y, (int)screen);
    agentLog("H2", "ElectroHelper.ino:loop", "touch_point", touchData);
    if (screen == SCR_MENU) {
      if (ui.hit(x, y, 12, 44, 146, 44)) { agentLog("H4", "ElectroHelper.ino:loop", "menu_to_dipole", "{}"); go(SCR_DIPOLE); }
      else if (ui.hit(x, y, 162, 44, 146, 44)) { go(SCR_ADC); }
      else if (ui.hit(x, y, 12, 92, 146, 44)) { go(SCR_CAP); }
      else if (ui.hit(x, y, 162, 92, 146, 44)) { go(SCR_PWM); }
      else if (ui.hit(x, y, 12, 140, 146, 44)) { go(SCR_IND); }
      else if (ui.hit(x, y, 162, 140, 146, 44)) { go(SCR_CALC); }
      else if (ui.hit(x, y, 12, 188, 146, 44)) { go(SCR_RES); }
      else if (ui.hit(x, y, 162, 188, 146, 44)) { go(SCR_YAGI); }
      ui.waitRelease();
    } else {
      if (ui.hit(x, y, 6, 2, 74, 26)) {
        go(SCR_MENU);
        ui.waitRelease();
      } else {
        switch (screen) {
          case SCR_DIPOLE: toolDipole.onTouch(x, y); break;
          case SCR_ADC:    toolAdc.onTouch(x, y); break;
          case SCR_CAP:    toolCap.onTouch(x, y); break;
          case SCR_PWM:    toolPwm.onTouch(x, y); break;
          case SCR_IND:    toolInd.onTouch(x, y); break;
          case SCR_RES:    toolRes.onTouch(x, y); break;
          case SCR_YAGI:   toolYagi.onTouch(x, y); break;
          case SCR_CALC:   toolCalc.onTouch(x, y); break;
          default: break;
        }
        ui.waitRelease();
      }
    }
  }

  if (screen == SCR_ADC) toolAdc.tick();
  if (screen == SCR_PWM) toolPwm.tick();
}
