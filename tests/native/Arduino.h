#pragma once
// Host test shim, never included in an embedded build. Not a concurrency model.
#include <cassert>
#include <cstdint>
#define IRAM_ATTR
constexpr int FALLING = 2, INPUT_PULLUP = 3;
using portMUX_TYPE = int;
constexpr int portMUX_INITIALIZER_UNLOCKED = 0;
inline uint32_t fakeMillis = 0, fakeMicros = 0;
inline void (*fakeInterrupt)() = nullptr;
inline int attachedPin = -1, criticalDepth = 0;
inline uint32_t millis() { return fakeMillis; }
inline uint32_t micros() { return fakeMicros; }
inline void pinMode(int, int) {}
inline int digitalPinToInterrupt(int pin) { return pin; }
inline void attachInterrupt(int pin, void (*fn)(), int) {
  assert(fakeInterrupt == nullptr);
  attachedPin = pin; fakeInterrupt = fn;
}
inline void detachInterrupt(int pin) {
  assert(pin == attachedPin);
  attachedPin = -1; fakeInterrupt = nullptr;
}
inline void portENTER_CRITICAL(portMUX_TYPE*) { ++criticalDepth; }
inline void portEXIT_CRITICAL(portMUX_TYPE*) { assert(criticalDepth == 1); --criticalDepth; }
inline void portENTER_CRITICAL_ISR(portMUX_TYPE* mux) { portENTER_CRITICAL(mux); }
inline void portEXIT_CRITICAL_ISR(portMUX_TYPE* mux) { portEXIT_CRITICAL(mux); }
inline void pulseAt(uint32_t us) {
  fakeMicros = us;
  assert(fakeInterrupt);
  fakeInterrupt();
  assert(criticalDepth == 0);
}
