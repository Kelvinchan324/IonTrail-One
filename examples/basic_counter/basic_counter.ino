#include <IonTrail.h>

IonTrailDevice detector;

void setup() {
  Serial.begin(115200);
  IonTrailConfig config;
  config.gmPulsePin = 3;
  detector.begin(config);
}

void loop() {
  detector.update();
  if (detector.hasFreshReading()) {
    Serial.printf("%.1f CPM\n", detector.cpm());
  }
}

