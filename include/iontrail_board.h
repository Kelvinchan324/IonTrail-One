#pragma once

// Provisional EVT pin map for a common ESP32-C3 SuperMini layout.
// Verify the schematic and silk screen of the exact purchased board.
// Do not reuse native USB or boot-strapping pins without checking startup behaviour.

namespace IonTrailBoard {
constexpr int kGmPulsePin = 3;
constexpr int kI2cSdaPin = 4;
constexpr int kI2cSclPin = 5;
constexpr int kBuzzerPin = 6;
constexpr int kStatusLedPin = 7;
constexpr int kButtonPrimaryPin = 0;
constexpr int kButtonBackPin = 1;
constexpr int kButtonNextPin = 10;
constexpr int kBatteryAdcPin = 2;
}  // namespace IonTrailBoard

