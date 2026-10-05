#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT4x.h>
#include <Adafruit_SSD1306.h>
#include <IonTrail.h>
#include "iontrail_board.h"

IonTrailDevice ionTrail;
Adafruit_SHT4x climate;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
bool counterReady = false, climateReady = false, displayReady = false;
bool resetHeld = false, resetDone = false;
uint32_t resetSince = 0, lastUi = 0;
float temperature = NAN, humidity = NAN;

bool i2cPresent(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

void setup() {
  Serial.begin(115200);
  pinMode(IonTrailBoard::kButtonPrimaryPin, INPUT_PULLUP);
  pinMode(IonTrailBoard::kStatusLedPin, OUTPUT);
  Wire.begin(IonTrailBoard::kI2cSdaPin, IonTrailBoard::kI2cSclPin);
  Wire.setClock(100000); Wire.setTimeOut(50);
  climateReady = i2cPresent(0x44) && climate.begin(&Wire);
  if (climateReady) {
    climate.setPrecision(SHT4X_MED_PRECISION); climate.setHeater(SHT4X_NO_HEATER);
  }
  displayReady = i2cPresent(IonTrailBoard::kDisplayAddress) &&
      display.begin(SSD1306_SWITCHCAPVCC, IonTrailBoard::kDisplayAddress, false, false);
  IonTrailConfig config;
  config.gmPulsePin = IonTrailBoard::kGmPulsePin;
  config.sampleWindowMs = 10000; config.deadTimeMicros = 0;
  counterReady = ionTrail.begin(config);
  Serial.printf("# IonTrail EVT-A counter=%d sht40=%d oled=%d\n", counterReady, climateReady, displayReady);
  Serial.println("# CPM only; not calibrated dose. Zero counts cannot prove detector health.");
  Serial.println("uptime_ms,window_ms,cpm,total_counts,temperature_c,humidity_percent,sensor_ok");
}

void loop() {
  if (!counterReady) { delay(100); return; }
  ionTrail.update();
  uint32_t now = millis();
  if (digitalRead(IonTrailBoard::kButtonPrimaryPin) == LOW) {
    if (!resetHeld) { resetHeld = true; resetSince = now; }
    if (!resetDone && now - resetSince >= 2000) {
      ionTrail.resetTotals(); resetDone = true;
      Serial.println("# totals reset; wait one full 10-second window");
    }
  } else { resetHeld = false; resetDone = false; }
  if (ionTrail.hasFreshReading()) {
    bool sensorOk = false;
    if (climateReady) {
      sensors_event_t h{}, t{};
      sensorOk = climate.getEvent(&h, &t);
      temperature = sensorOk ? t.temperature : NAN;
      humidity = sensorOk ? h.relative_humidity : NAN;
    }
    Serial.printf("%lu,%lu,%.2f,%lu,%.2f,%.2f,%d\n",
                  static_cast<unsigned long>(now),
                  static_cast<unsigned long>(ionTrail.lastWindowMs()), ionTrail.cpm(),
                  static_cast<unsigned long>(ionTrail.totalCounts()), temperature, humidity, sensorOk);
  }
  if (now - lastUi >= 500) {
    lastUi = now;
    digitalWrite(IonTrailBoard::kStatusLedPin, !digitalRead(IonTrailBoard::kStatusLedPin));
    if (displayReady) {
      display.clearDisplay(); display.setTextColor(SSD1306_WHITE);
      display.setTextSize(1); display.setCursor(0, 0); display.println("IONTRAIL EVT-A");
      display.setTextSize(2);
      if (ionTrail.lastWindowMs()) display.print(ionTrail.cpm(), 1);
      else display.print("WAIT");
      display.setTextSize(1); display.println(" CPM");
      display.printf("Total %lu\n", static_cast<unsigned long>(ionTrail.totalCounts()));
      if (isfinite(temperature) && isfinite(humidity))
        display.printf("%.1f C / %.1f %%RH\n", temperature, humidity);
      else display.println("Climate unavailable");
      display.println("Not a safety meter"); display.display();
    }
  }
  delay(1);
}
