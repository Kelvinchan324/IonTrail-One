#pragma once
#include <Arduino.h>
#include <Wire.h>

constexpr int SSD1306_SWITCHCAPVCC = 2, SSD1306_WHITE = 1;
struct Adafruit_SSD1306 : HostText {
  bool beginOk = true;
  unsigned begins = 0, frames = 0;
  std::string lastFrame;
  Adafruit_SSD1306(int, int, TwoWire*, int) {}
  bool begin(int, int, bool, bool) { ++begins; return beginOk; }
  void clearDisplay() { text.clear(); }
  void setTextColor(int) {}
  void setTextSize(int) {}
  void setCursor(int, int) {}
  void display() { ++frames; lastFrame = text; }
};
