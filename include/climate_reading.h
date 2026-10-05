#pragma once
#include <math.h>
#include <stdint.h>

// Measurement availability, not sensor accuracy or radiation-detector health.
class ClimateReading {
 public:
  void update(bool readOk, float temperature, float humidity, uint32_t nowMs) {
    sampleMs_ = nowMs;
    ok_ = readOk && isfinite(temperature) && isfinite(humidity) &&
          humidity >= 0.0F && humidity <= 100.0F;
    temperature_ = ok_ ? temperature : NAN;
    humidity_ = ok_ ? humidity : NAN;
  }
  bool valid(uint32_t nowMs) const {
    return ok_ && uint32_t(nowMs - sampleMs_) <= kMaxAgeMs;
  }
  float temperature(uint32_t nowMs) const { return valid(nowMs) ? temperature_ : NAN; }
  float humidity(uint32_t nowMs) const { return valid(nowMs) ? humidity_ : NAN; }
  static constexpr uint32_t kMaxAgeMs = 15000;

 private:
  bool ok_ = false;
  uint32_t sampleMs_ = 0;
  float temperature_ = NAN, humidity_ = NAN;
};
