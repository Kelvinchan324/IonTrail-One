#include "IonTrail.h"

IonTrailDevice* IonTrailDevice::activeInstance_ = nullptr;

bool IonTrailDevice::begin(const IonTrailConfig& config) {
  if (config.gmPulsePin < 0 || config.sampleWindowMs == 0) return false;
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

void IonTrailDevice::update() {
  const uint32_t now = millis();
  const uint32_t elapsed = now - windowStartedMs_;
  if (elapsed < config_.sampleWindowMs) return;

  uint32_t pulses = 0;
  portENTER_CRITICAL(&pulseMux_);
  pulses = pendingPulses_;
  pendingPulses_ = 0;
  portEXIT_CRITICAL(&pulseMux_);

  lastWindowMs_ = elapsed;
  totalCounts_ += pulses;
  cpm_ = elapsed > 0
             ? (static_cast<float>(pulses) * 60000.0f) /
                   static_cast<float>(elapsed)
             : 0.0f;
  freshReading_ = true;
  windowStartedMs_ = now;
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
  portEXIT_CRITICAL(&pulseMux_);
  totalCounts_ = 0;
  cpm_ = 0.0f;
  freshReading_ = false;
  lastWindowMs_ = 0;
  windowStartedMs_ = millis();
}

void IRAM_ATTR IonTrailDevice::handlePulseInterrupt() {
  if (activeInstance_ != nullptr) activeInstance_->recordPulse();
}

void IRAM_ATTR IonTrailDevice::recordPulse() {
  const uint32_t now = micros();
  if ((now - lastPulseMicros_) < config_.deadTimeMicros) return;

  lastPulseMicros_ = now;
  portENTER_CRITICAL_ISR(&pulseMux_);
  ++pendingPulses_;
  portEXIT_CRITICAL_ISR(&pulseMux_);
}

