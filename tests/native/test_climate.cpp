#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>
#include <initializer_list>
#include "climate_reading.h"

int main() {
  ClimateReading sample;
  assert(!sample.valid(0) && std::isnan(sample.temperature(0)));
  sample.update(true, 0.0F, 0.0F, 1000);
  assert(sample.valid(1000) && sample.temperature(1000) == 0 && sample.humidity(1000) == 0);
  assert(sample.valid(16000) && !sample.valid(16001));
  assert(std::isnan(sample.temperature(16001)) && std::isnan(sample.humidity(16001)));
  assert(!sample.valid(999)); // Future sample compared to an older clock is unavailable.
  sample.update(false, 25, 50, 20000);
  assert(!sample.valid(20000) && std::isnan(sample.temperature(20000)));
  sample.update(true, 25, 100, 30000);
  assert(sample.valid(30000) && sample.humidity(30000) == 100);
  for (float bad : {-1.0F, 101.0F, std::numeric_limits<float>::infinity(),
                    std::numeric_limits<float>::quiet_NaN()}) {
    sample.update(true, 25, bad, 30001);
    assert(!sample.valid(30001));
  }
  sample.update(true, std::numeric_limits<float>::quiet_NaN(), 50, 30002);
  assert(!sample.valid(30002));
  sample.update(true, std::numeric_limits<float>::infinity(), 50, 30003);
  assert(!sample.valid(30003));
  sample.update(true, 20, 40, UINT32_MAX - 5);
  assert(sample.valid(4) && sample.temperature(4) == 20); // 10 ms across wrap.
  assert(!sample.valid(15000));
  sample.update(true, 21, 41, 15001);
  assert(sample.valid(15001)); // Explicit successful reading restores availability.
  std::puts("PASS: climate availability, finite values, RH bounds, expiry, failure, recovery, rollover");
}
