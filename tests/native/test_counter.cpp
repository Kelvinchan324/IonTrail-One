#include <cmath>
#include <cstdio>
#include "IonTrail.h"
#include "count_math.h"

void expectRate(const IonTrailDevice& d, float expected) {
  assert(std::fabs(d.cpm() - expected) < 0.01F);
}

int main() {
  IonTrailConfig config;
  config.gmPulsePin = 3;
  IonTrailDevice device, second;
  assert(device.begin(config));
  assert(!second.begin(config)); // One physical interrupt owner.
  for (uint32_t i = 1; i <= 100; ++i) pulseAt(i * 100000);
  fakeMillis = 9999; device.update();
  assert(!device.hasFreshReading());
  fakeMillis = 10000; device.update();
  assert(device.hasFreshReading() && !device.hasFreshReading());
  assert(device.totalCounts() == 100 && device.lastWindowCounts() == 100);
  assert(device.lastWindowMs() == 10000 && !device.overflowed());
  expectRate(device, 600);
  fakeMillis = 20000; device.update();
  expectRate(device, 0); // Zero is a count, not proof the detector works.
  for (uint32_t i = 1; i <= 250; ++i) pulseAt(i * 100000 + 20000000);
  fakeMillis = 45000; device.update();
  assert(device.lastWindowMs() == 25000 && device.totalCounts() == 350);
  expectRate(device, 600); // Actual elapsed time, not nominal window duration.

  pulseAt(45000100); device.resetTotals();
  assert(device.totalCounts() == 0 && device.lastWindowCounts() == 0);
  assert(device.lastWindowMs() == 0 && !device.hasFreshReading());
  fakeMillis = 55000; device.update(); expectRate(device, 0);
  assert(device.totalCounts() == 0); // Pending pulse was discarded by reset.

  config.deadTimeMicros = 100;
  fakeMillis = 0; fakeMicros = 0;
  assert(device.begin(config));
  pulseAt(0); pulseAt(50); pulseAt(100);
  fakeMillis = 10000; device.update();
  assert(device.lastWindowCounts() == 2); // First pulse at t=0 must count.
  device.resetTotals();
  pulseAt(UINT32_MAX - 50); pulseAt(20); pulseAt(60);
  fakeMillis = 20000; device.update();
  assert(device.lastWindowCounts() == 2); // micros rollover; 71 us rejected.

  fakeMillis = UINT32_MAX - 4999;
  device.resetTotals();
  pulseAt(1000);
  fakeMillis = 5000; device.update();
  assert(device.lastWindowMs() == 10000 && device.lastWindowCounts() == 1);
  expectRate(device, 6); // millis rollover.
  IonTrailConfig invalid = config;
  invalid.sampleWindowMs = 0; assert(!device.begin(invalid));
  invalid.sampleWindowMs = 0x80000000UL; assert(!device.begin(invalid));
  invalid = config; invalid.deadTimeMicros = 0x80000000UL;
  assert(!device.begin(invalid));
  invalid = config; invalid.gmPulsePin = -1; assert(!device.begin(invalid));
  assert(attachedPin == 3); // Invalid reconfiguration preserves current owner.
  device.end(); device.end(); assert(fakeInterrupt == nullptr);
  assert(second.begin(config)); second.end();
  { IonTrailDevice scoped; assert(scoped.begin(config)); }
  assert(fakeInterrupt == nullptr); // Destructor releases interrupt ownership.

  uint32_t result = 0;
  assert(addCounts(UINT32_MAX - 10, 10, result) && result == UINT32_MAX);
  assert(!addCounts(UINT32_MAX - 10, 11, result) && result == UINT32_MAX);
  assert(!addCounts(UINT32_MAX, 1, result) && result == UINT32_MAX);
  assert(addCounts(0, 0, result) && result == 0);
  std::puts("PASS: actual counter library rate, windows, reset, dead time, timer rollover, lifetime, saturating arithmetic");
}
