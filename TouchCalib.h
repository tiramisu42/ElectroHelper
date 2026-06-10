#pragma once
#include <Preferences.h>
#include <XPT2046_Touchscreen.h>

struct TouchCalibData {
  int minX, maxX, minY, maxY;
  bool swapXY, invX, invY;
  bool valid;
};

class TouchCalib {
public:
  void begin(Preferences &p) { prefs = &p; }

  void load() {
    prefs->begin("touch", true);
    data.valid = prefs->getBool("valid", false);
    data.minX = prefs->getInt("minX", 600);
    data.maxX = prefs->getInt("maxX", 3600);
    data.minY = prefs->getInt("minY", 500);
    data.maxY = prefs->getInt("maxY", 3500);
    data.swapXY = prefs->getBool("swap", false);
    data.invX = prefs->getBool("invX", false);
    data.invY = prefs->getBool("invY", false);
    prefs->end();
  }

  void save() {
    prefs->begin("touch", false);
    prefs->putInt("minX", data.minX);
    prefs->putInt("maxX", data.maxX);
    prefs->putInt("minY", data.minY);
    prefs->putInt("maxY", data.maxY);
    prefs->putBool("swap", data.swapXY);
    prefs->putBool("invX", data.invX);
    prefs->putBool("invY", data.invY);
    prefs->putBool("valid", data.valid);
    prefs->end();
  }

  bool isValid() const { return data.valid; }
  const TouchCalibData& get() const { return data; }
  TouchCalibData& getMutable() { return data; }

private:
  Preferences *prefs = nullptr;
  TouchCalibData data {600, 3600, 500, 3500, false, false, false, false};
};