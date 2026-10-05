// Separate 3.3 V ESP32 board; detector DISCONNECTED. GPIO4 -> 1k -> IonTrail GPIO3.
// Join GND. Do not attach a high-voltage node to either board.
#include <Arduino.h>
constexpr int kPulsePin = 4;
uint32_t nextPulse = 0;
void setup() { pinMode(kPulsePin, OUTPUT); digitalWrite(kPulsePin, HIGH); nextPulse = micros(); }
void loop() {
  uint32_t now = micros();
  if (static_cast<int32_t>(now - nextPulse) >= 0) {
    digitalWrite(kPulsePin, LOW); delayMicroseconds(300); digitalWrite(kPulsePin, HIGH);
    nextPulse += 100000; // 10 Hz => 600 CPM. Count fidelity, not dose calibration.
  }
}
