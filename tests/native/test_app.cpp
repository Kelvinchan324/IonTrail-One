#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
// Compile the actual setup/loop; only external board/vendor boundaries are doubled.
#include "../../src/main.cpp"

void freshBoot(bool sensorPresent = true, bool oledPresent = true, uint32_t at = 0) {
  ionTrail.end();
  fakeMillis = at; fakeMicros = 0;
  ionTrail.resetTotals();
  std::fill(std::begin(fakePins), std::end(fakePins), HIGH);
  Serial.text.clear();
  Wire = TwoWire{};
  Wire.present[0x44] = sensorPresent;
  Wire.present[IonTrailBoard::kDisplayAddress] = oledPresent;
  climate = Adafruit_SHT4x{};
  display = Adafruit_SSD1306(128, 64, &Wire, -1);
  counterReady = climateReady = displayReady = false;
  resetHeld = resetDone = false;
  resetSince = lastUi = 0;
  environment = ClimateReading{};
}
void loopAt(uint32_t at) { fakeMillis = at; loop(); }
size_t occurrences(const std::string& text, const std::string& word) {
  size_t count = 0, at = 0;
  while ((at = text.find(word, at)) != std::string::npos) { ++count; at += word.size(); }
  return count;
}
std::string lastRow() {
  const size_t end = Serial.text.find_last_not_of('\n');
  assert(end != std::string::npos);
  const size_t before = Serial.text.rfind('\n', end);
  return Serial.text.substr(before == std::string::npos ? 0 : before + 1,
      end - (before == std::string::npos ? 0 : before + 1) + 1);
}
void requireText(const std::string& text, const char* expected) {
  if (text.find(expected) == std::string::npos) {
    std::fprintf(stderr, "Missing [%s] in [%s]\n", expected, text.c_str());
    assert(false);
  }
}
int main() {
  freshBoot(); setup();
  assert(counterReady && climateReady && displayReady);
  assert(Wire.clock == 100000 && Wire.timeout == 50);
  assert(Wire.sda == IonTrailBoard::kI2cSdaPin && Wire.scl == IonTrailBoard::kI2cSclPin);
  assert(climate.precision == SHT4X_MED_PRECISION && climate.heater == SHT4X_NO_HEATER);
  assert(attachedPin == IonTrailBoard::kGmPulsePin);
  loopAt(500); requireText(display.lastFrame, "WAIT CPM");
  requireText(display.lastFrame, "Climate unavailable");
  for (unsigned i = 1; i <= 10; ++i) pulseAt(i * 1000);
  loopAt(10000);
  assert(lastRow() == "10000,10000,10,60.00,10,25.00,50.00,1,1");
  assert(climate.reads == 1 && climate.begins == 1);
  const std::string firstOutput = Serial.text;
  loopAt(10500); assert(Serial.text == firstOutput && climate.reads == 1);
  requireText(display.lastFrame, "25.0 C / 50.0 %RH");

  // Failed reads clear formerly valid numbers; next success recovers without begin.
  climate.readOk = false;
  loopAt(20000); requireText(lastRow(), ",nan,nan,0,1");
  requireText(display.lastFrame, "Climate unavailable");
  climate.readOk = true; climate.temperature = 24; climate.humidity = 101;
  loopAt(30000); requireText(lastRow(), ",nan,nan,0,1");
  climate.humidity = 40;
  loopAt(40000); requireText(lastRow(), ",24.00,40.00,1,1");
  assert(climate.begins == 1 && climate.reads == 4);

  // Boot absence and begin failure retry only once per completed count window.
  freshBoot(false, false); setup();
  assert(!climateReady && !displayReady && climate.begins == 0);
  loopAt(9999); assert(Wire.probes[0x44] == 1 && climate.reads == 0);
  loopAt(10000); assert(Wire.probes[0x44] == 2);
  requireText(lastRow(), ",nan,nan,0,1");
  Wire.present[0x44] = true; climate.beginOk = false;
  loopAt(20000); assert(!climateReady && climate.begins == 1 && climate.reads == 0);
  loopAt(20500); assert(climate.begins == 1);
  climate.beginOk = true;
  loopAt(30000); assert(climateReady && climate.begins == 2 && climate.reads == 1);
  requireText(lastRow(), ",25.00,50.00,1,1");
  assert(display.frames == 0);
  // Explicitly capture current limitation: no OLED retry after boot absence.
  Wire.present[IonTrailBoard::kDisplayAddress] = true;
  loopAt(30500); assert(!displayReady && display.begins == 0);

  freshBoot(); display.beginOk = false; setup();
  assert(!displayReady && display.begins == 1);
  loopAt(10000);
  requireText(lastRow(), ",25.00,50.00,1,1");
  assert(display.frames == 0); // Allocation/init failure also leaves serial available.

  // Delayed synchronous I/O: row timestamp is count endpoint, not climate time.
  freshBoot(); setup(); climate.readDelayMs = 600;
  pulseAt(1000); loopAt(10000);
  assert(lastRow() == "10000,10000,1,6.00,1,25.00,50.00,1,1");
  assert(fakeMillis == 10601 && lastUi == 10600);
  requireText(display.lastFrame, "25.0 C / 50.0 %RH");

  // Hold threshold, no repeated reset while held, release/repress, new full window.
  freshBoot(); setup(); pulseAt(1000); loopAt(10000);
  fakePins[IonTrailBoard::kButtonPrimaryPin] = LOW;
  loopAt(11000); loopAt(12999); assert(ionTrail.totalCounts() == 1);
  pulseAt(12000); loopAt(13000);
  assert(ionTrail.totalCounts() == 0 && ionTrail.lastWindowMs() == 0);
  loopAt(13500); // Display refresh is scheduled at 500 ms, not on every loop.
  requireText(display.lastFrame, "WAIT CPM");
  loopAt(15000); assert(occurrences(Serial.text, "# totals reset") == 1);
  pulseAt(14000); loopAt(22999); assert(ionTrail.lastWindowMs() == 0);
  loopAt(23000); assert(lastRow() == "23000,10000,1,6.00,1,25.00,50.00,1,1");
  fakePins[IonTrailBoard::kButtonPrimaryPin] = HIGH; loopAt(24000);
  fakePins[IonTrailBoard::kButtonPrimaryPin] = LOW;
  loopAt(25000); loopAt(27000); assert(occurrences(Serial.text, "# totals reset") == 2);

  // Fault-inject counter initialization failure using the actual singleton owner.
  freshBoot();
  IonTrailDevice other;
  IonTrailConfig occupied; occupied.gmPulsePin = 2;
  assert(other.begin(occupied)); setup();
  assert(!counterReady && displayReady);
  const std::string headers = Serial.text;
  loopAt(500); requireText(display.lastFrame, "INIT FAIL");
  assert(fakePins[IonTrailBoard::kStatusLedPin] == LOW);
  loopAt(20000); assert(Serial.text == headers && climate.reads == 0);
  other.end();

  // Main-loop reset duration and count endpoint survive uint32 clock rollover.
  freshBoot(true, true, UINT32_MAX - 4999); setup();
  fakePins[IonTrailBoard::kButtonPrimaryPin] = LOW;
  loopAt(UINT32_MAX - 999); loopAt(999); assert(!resetDone);
  loopAt(1000); assert(resetDone);
  pulseAt(1); loopAt(11000);
  assert(lastRow() == "11000,10000,1,6.00,1,25.00,50.00,1,1");
  ionTrail.end();
  std::puts("PASS: production setup/loop, climate retry/failure/delay, CSV, OLED text, reset, init failure and rollover");
}
