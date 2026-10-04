#include <Arduino.h>

#include <IonTrail.h>
#include "iontrail_board.h"

IonTrailDevice ionTrail;

void setup() {
  Serial.begin(115200);

  IonTrailConfig config;
  config.gmPulsePin = IonTrailBoard::kGmPulsePin;
  config.sampleWindowMs = 10000;
  config.deadTimeMicros = 0;  // Set only from characterized tube/interface data.

  if (!ionTrail.begin(config)) {
    Serial.println("IonTrail failed to start. Check the configured pulse pin.");
    return;
  }

  Serial.println("IonTrail One SDK reference firmware");
  Serial.println("CPM is a count-rate measurement, not a universal dose value.");
}

void loop() {
  ionTrail.update();
  if (ionTrail.hasFreshReading()) {
    Serial.printf("CPM=%.1f total=%lu interval=%lu ms\n",
                  ionTrail.cpm(),
                  static_cast<unsigned long>(ionTrail.totalCounts()),
                  static_cast<unsigned long>(ionTrail.lastWindowMs()));
  }
}
