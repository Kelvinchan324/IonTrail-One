#include <cassert>
#include "../../examples/basic_counter/basic_counter.ino"

int main() {
  setup();
  assert(counterReady && attachedPin == 3 && attachedMode == FALLING);
  pulseAt(1);
  fakeMillis = 10000;
  loop();
  assert(Serial.text == "6.0 CPM\n");
  detector.end();

  IonTrailDevice occupied;
  IonTrailConfig config;
  config.gmPulsePin = 3;
  assert(occupied.begin(config));
  Serial.text.clear();
  setup();
  assert(!counterReady);
  assert(Serial.text == "# counter INIT FAIL; no measurements\n");
  pulseAt(2);
  fakeMillis += 10000;
  loop();
  assert(Serial.text == "# counter INIT FAIL; no measurements\n");
  occupied.end();
  std::puts("PASS: production basic example success and init-failure behavior");
}
