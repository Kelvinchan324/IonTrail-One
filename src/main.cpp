#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT4x.h>
#include <Adafruit_SSD1306.h>
#include <IonTrail.h>
#include "iontrail_board.h"
#include "climate_reading.h"

IonTrailDevice ionTrail;
Adafruit_SHT4x climate;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
bool counterReady = false, climateReady = false, displayReady = false;
bool resetHeld = false, resetDone = false;
uint32_t resetSince = 0, lastUi = 0;
ClimateReading environment;

bool i2cPresent(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

bool startClimate() {
  if (!i2cPresent(0x44) || !climate.begin(&Wire)) return false;
  climate.setPrecision(SHT4X_MED_PRECISION);
  climate.setHeater(SHT4X_NO_HEATER);
  return true;
}

void setup() {
  Serial.begin(115200);
  pinMode(IonTrailBoard::kButtonPrimaryPin, INPUT_PULLUP);
  pinMode(IonTrailBoard::kStatusLedPin, OUTPUT);
  Wire.begin(IonTrailBoard::kI2cSdaPin, IonTrailBoard::kI2cSclPin);
  Wire.setClock(100000); Wire.setTimeOut(50);
  climateReady = startClimate();
  displayReady = i2cPresent(IonTrailBoard::kDisplayAddress) &&
      display.begin(SSD1306_SWITCHCAPVCC, IonTrailBoard::kDisplayAddress, false, false);
  IonTrailConfig config;
  config.gmPulsePin = IonTrailBoard::kGmPulsePin;
  config.sampleWindowMs = 10000; config.deadTimeMicros = 0;
  counterReady = ionTrail.begin(config);
  Serial.printf("# IonTrail EVT-A counter=%d sht40=%d oled=%d\n", counterReady, climateReady, displayReady);
  Serial.println("# CPM only; not calibrated dose. Zero counts cannot prove detector health.");
  Serial.println("uptime_ms,window_ms,window_counts,cpm,total_counts,temperature_c,humidity_percent,sensor_ok,counts_valid");
}

void loop() {
  if (counterReady) ionTrail.update();
  uint32_t now = millis();
  if (counterReady && digitalRead(IonTrailBoard::kButtonPrimaryPin) == LOW) {
    if (!resetHeld) { resetHeld = true; resetSince = now; }
    if (!resetDone && now - resetSince >= 2000) {
      ionTrail.resetTotals(); resetDone = true;
      Serial.println("# totals reset; wait one full 10-second window");
    }
  } else { resetHeld = false; resetDone = false; }
  if (counterReady && ionTrail.hasFreshReading()) {
    // Retry boot-time absence once per completed count window. Once initialized,
    // use getEvent for transient recovery; do not repeatedly reinitialize a live driver.
    if (!climateReady) climateReady = startClimate();
    bool readOk = false;
    sensors_event_t h{}, t{};
    if (climateReady) {
      readOk = climate.getEvent(&h, &t);
    }
    const uint32_t climateAt = millis();
    environment.update(readOk, t.temperature, h.relative_humidity, climateAt);
    const bool sensorOk = environment.valid(climateAt);
    Serial.printf("%lu,%lu,%lu,%.2f,%lu,%.2f,%.2f,%d,%d\n",
                  static_cast<unsigned long>(now),
                  static_cast<unsigned long>(ionTrail.lastWindowMs()),
                  static_cast<unsigned long>(ionTrail.lastWindowCounts()), ionTrail.cpm(),
                  static_cast<unsigned long>(ionTrail.totalCounts()),
                  environment.temperature(climateAt), environment.humidity(climateAt),
                  sensorOk, !ionTrail.overflowed());
  }
  now = millis(); // Climate I/O takes time; never compare a sample to an older clock.
  if (now - lastUi >= 500) {
    lastUi = now;
    digitalWrite(IonTrailBoard::kStatusLedPin, !digitalRead(IonTrailBoard::kStatusLedPin));
    if (displayReady) {
      display.clearDisplay(); display.setTextColor(SSD1306_WHITE);
      display.setTextSize(1); display.setCursor(0, 0); display.println("IONTRAIL EVT-A");
      display.setTextSize(2);
      if (!counterReady) display.print("INIT FAIL");
      else if (ionTrail.overflowed()) display.print("FAULT");
      else if (ionTrail.lastWindowMs()) display.print(ionTrail.cpm(), 1);
      else display.print("WAIT");
      display.setTextSize(1); display.println(counterReady ? " CPM" : "");
      display.printf("Total %lu\n", static_cast<unsigned long>(ionTrail.totalCounts()));
      if (environment.valid(now))
        display.printf("%.1f C / %.1f %%RH\n", environment.temperature(now), environment.humidity(now));
      else display.println("Climate unavailable");
      display.println("Not a safety meter"); display.display();
    }
  }
  delay(1);
}
