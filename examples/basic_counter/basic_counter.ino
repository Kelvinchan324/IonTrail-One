#include <IonTrail.h>

IonTrailDevice detector;
bool counterReady = false;

void setup() {
  Serial.begin(115200);
  IonTrailConfig config;
  config.gmPulsePin = 3;
  counterReady = detector.begin(config);
  if (!counterReady) Serial.println("# counter INIT FAIL; no measurements");
}

void loop() {
  if (!counterReady) return;
  detector.update();
  if (detector.hasFreshReading()) {
    Serial.printf("%.1f CPM\n", detector.cpm());
  }
}

