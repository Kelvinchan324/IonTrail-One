# SDK quick start

This guide brings up the count-rate SDK on an ESP32-C3 SuperMini without requiring the display, environmental sensor, or final enclosure.

## What you need

- ESP32-C3 SuperMini or compatible ESP32-C3 development board
- USB data cable
- A GM pulse source that presents a clean **3.3 V logic signal**
- Visual Studio Code with PlatformIO

> Do not attach a GM tube or high-voltage output directly to the ESP32. Use an isolated or properly conditioned, current-limited pulse interface.

## Build the reference firmware

```bash
git clone https://github.com/Kelvinchan324/IonTrail-One.git
cd IonTrail-One
pio run
pio run --target upload
pio device monitor
```

The serial monitor should print one CPM result per sampling window.

## Confirm the board map

Open `include/iontrail_board.h` and compare every pin with the labels and schematic for your specific SuperMini revision. The file is a provisional EVT map, not a promise that all boards sold under the same name are identical.

The GM pulse input must:

- remain between 0 V and 3.3 V;
- idle at the level expected by `interruptMode`;
- have a clean edge and adequate pulse width;
- share a valid logic reference with the ESP32; and
- be protected against transients from the high-voltage section.

## Use the library in your own sketch

```cpp
#include <IonTrail.h>

IonTrailDevice detector;

void setup() {
  Serial.begin(115200);

  IonTrailConfig config;
  config.gmPulsePin = 3;
  config.sampleWindowMs = 10000;
  detector.begin(config);
}

void loop() {
  detector.update();
  if (detector.hasFreshReading()) {
    Serial.printf("%.1f CPM\n", detector.cpm());
  }
}
```

Start with `deadTimeMicros = 0`. Add dead-time rejection only after characterizing the chosen tube and pulse-conditioning circuit.

## Next steps

- Adapt the [basic counter example](../examples/basic_counter/basic_counter.ino).
- Review [customization](customization.md) before adding screens or BLE services.
- Read [safety and calibration](safety.md) before interpreting measurements.

