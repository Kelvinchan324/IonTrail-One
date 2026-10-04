#pragma once

#include <Arduino.h>

struct IonTrailConfig {
  int gmPulsePin = -1;
  uint32_t sampleWindowMs = 10000;
  // Leave disabled unless the selected tube and pulse circuit have been characterized.
  uint32_t deadTimeMicros = 0;
  int interruptMode = FALLING;
};

class IonTrailDevice {
 public:
  bool begin(const IonTrailConfig& config);
  void update();

  float cpm() const { return cpm_; }
  uint32_t totalCounts() const { return totalCounts_; }
  uint32_t lastWindowMs() const { return lastWindowMs_; }
  bool hasFreshReading();
  void resetTotals();

 private:
  static IonTrailDevice* activeInstance_;
  static void IRAM_ATTR handlePulseInterrupt();
  void IRAM_ATTR recordPulse();

  IonTrailConfig config_{};
  volatile uint32_t pendingPulses_ = 0;
  volatile uint32_t lastPulseMicros_ = 0;
  uint32_t totalCounts_ = 0;
  uint32_t windowStartedMs_ = 0;
  uint32_t lastWindowMs_ = 0;
  float cpm_ = 0.0f;
  bool freshReading_ = false;
  portMUX_TYPE pulseMux_ = portMUX_INITIALIZER_UNLOCKED;
};
