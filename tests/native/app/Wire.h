#pragma once
#include <cstdint>

// Deterministic application test doubles, not an I2C/driver/concurrency model.
struct TwoWire {
  bool present[128]{};
  unsigned probes[128]{};
  uint8_t address = 0;
  int sda = -1, scl = -1;
  uint32_t clock = 0, timeout = 0;
  void begin(int data, int clockPin) { sda = data; scl = clockPin; }
  void setClock(uint32_t value) { clock = value; }
  void setTimeOut(uint32_t value) { timeout = value; }
  void beginTransmission(uint8_t value) { address = value; }
  int endTransmission() { ++probes[address]; return present[address] ? 0 : 2; }
};
inline TwoWire Wire;
