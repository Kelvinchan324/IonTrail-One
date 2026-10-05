#pragma once
#include <Arduino.h>
#include <Wire.h>

constexpr int SSD1306_SWITCHCAPVCC = 2, SSD1306_WHITE = 1;
struct Adafruit_SSD1306 : HostText {
  bool beginOk = true;
  bool dropAckAfterBegin = false;
  uint32_t beginDelayMs = 0;
  unsigned begins = 0, frames = 0;
  std::string lastFrame;
  Adafruit_SSD1306(int, int, TwoWire*, int) {}
  bool begin(int, int address, bool reset, bool periphBegin) {
    assert(!reset && !periphBegin); // Never reset the shared bus through this driver.
    ++begins; fakeMillis += beginDelayMs;
    if (dropAckAfterBegin) Wire.present[address] = false;
    return beginOk;
  }
  void clearDisplay() { text.clear(); }
  void setTextColor(int) {}
  void setTextSize(int) {}
  void setCursor(int, int) {}
  void display() { ++frames; lastFrame = text; }
};
