#include "IonTrail.h"
#include "count_math.h"
#include <math.h>

IonTrailDevice* IonTrailDevice::activeInstance_ = nullptr;

bool IonTrailDevice::begin(const IonTrailConfig& config) {
  if (config.gmPulsePin < 0 || config.sampleWindowMs == 0 ||
      config.sampleWindowMs >= 0x80000000UL || config.deadTimeMicros >= 0x80000000UL)
    return false;
  if (activeInstance_ && activeInstance_ != this) return false;
  if (activeInstance_ == this) detachInterrupt(digitalPinToInterrupt(config_.gmPulsePin));

  config_ = config;
  resetTotals();
  pinMode(config_.gmPulsePin, INPUT_PULLUP);
  windowStartedMs_ = millis();
  activeInstance_ = this;
  attachInterrupt(digitalPinToInterrupt(config_.gmPulsePin),
                  IonTrailDevice::handlePulseInterrupt,
                  config_.interruptMode);
  return true;
}

void IonTrailDevice::end() {
  if (activeInstance_ != this) return;
  detachInterrupt(digitalPinToInterrupt(config_.gmPulsePin));
  activeInstance_ = nullptr;
}

void IonTrailDevice::update() {
  if (activeInstance_ != this) return;
  if (millis() - windowStartedMs_ < config_.sampleWindowMs) return;

  uint32_t pulses = 0;
  portENTER_CRITICAL(&pulseMux_);
  const uint32_t now = millis();
  const uint32_t elapsed = now - windowStartedMs_;
  pulses = pendingPulses_;
  pendingPulses_ = 0;
  windowStartedMs_ = now;
  if (!addCounts(totalCounts_, pulses, totalCounts_)) overflowed_ = true;
  const bool valid = !overflowed_;
  portEXIT_CRITICAL(&pulseMux_);

  lastWindowMs_ = elapsed;
  lastWindowCounts_ = pulses;
  cpm_ = valid && elapsed > 0
             ? (static_cast<float>(pulses) * 60000.0f) /
                   static_cast<float>(elapsed)
             : NAN;
  freshReading_ = true;
}

bool IonTrailDevice::hasFreshReading() {
  const bool result = freshReading_;
  freshReading_ = false;
  return result;
}

void IonTrailDevice::resetTotals() {
  portENTER_CRITICAL(&pulseMux_);
  pendingPulses_ = 0;
  lastPulseMicros_ = 0;
  haveLastPulse_ = false;
  overflowed_ = false;
  windowStartedMs_ = millis();
  portEXIT_CRITICAL(&pulseMux_);
  totalCounts_ = 0;
  cpm_ = 0.0f;
  freshReading_ = false;
  lastWindowMs_ = 0;
  lastWindowCounts_ = 0;
}

void IRAM_ATTR IonTrailDevice::handlePulseInterrupt() {
  if (activeInstance_ != nullptr) activeInstance_->recordPulse();
}

void IRAM_ATTR IonTrailDevice::recordPulse() {
  portENTER_CRITICAL_ISR(&pulseMux_);
  const uint32_t now = micros();
  if (!haveLastPulse_ || (now - lastPulseMicros_) >= config_.deadTimeMicros) {
    lastPulseMicros_ = now;
    haveLastPulse_ = true;
    if (pendingPulses_ == UINT32_MAX) overflowed_ = true;
    else ++pendingPulses_;
  }
  portEXIT_CRITICAL_ISR(&pulseMux_);
}

