#pragma once
#include <Arduino.h>
#include <Wire.h>

constexpr int SHT4X_MED_PRECISION = 1, SHT4X_NO_HEATER = 0;
struct sensors_event_t { float temperature = 0, relative_humidity = 0; };
struct Adafruit_SHT4x {
  bool beginOk = true, readOk = true;
  unsigned begins = 0, reads = 0;
  uint32_t readDelayMs = 0;
  float temperature = 25, humidity = 50;
  int precision = -1, heater = -1;
  bool begin(TwoWire*) { ++begins; return beginOk; }
  void setPrecision(int value) { precision = value; }
  void setHeater(int value) { heater = value; }
  bool getEvent(sensors_event_t* h, sensors_event_t* t) {
    ++reads; fakeMillis += readDelayMs;
    h->relative_humidity = humidity; t->temperature = temperature;
    return readOk;
  }
};
