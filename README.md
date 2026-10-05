## EVT-A build and teaching manual (5 October 2026)

The runnable draft is a **USB-powered ESP32-C3 counter with a guarded SEN0463,
SHT40 and SSD1306 display**. Its 170 x 100 mm bench fixture is a development step;
the pocket battery product below remains a future design.

[Engineering manual](docs/engineering.md) · [BOM](hardware/BOM.md) ·
[Wiring schematic](hardware/schematic.svg) · [Placement drawing](mechanical/placement.svg) ·
[STEP draft](mechanical/layout-draft.step)

1. Confirm the exact C3 board, OLED pin order and 3.3 V signal levels against the
   engineering manual. Keep the detector guard closed. Do the first test with
   the detector disconnected and injected low-voltage pulses only.
2. Install PlatformIO, then run `pio run`, `pio run --target upload`, and
   `pio device monitor` at 115200 baud.
3. The screen starts at WAIT and reports CPM after ten seconds. A missing screen
   or environmental sensor should not prevent serial counting.
4. Supply 10 Hz pulses from a separate 3.3 V fixture through 1k into GPIO3.
   Collect at least three full windows and run
   `python tools/check_log.py capture.csv --hz 10`. Expect about 600 CPM.
5. Power off, remove the fixture, then connect the detector's low-voltage output.
   Record natural background counts. Hold the count-reset button for two seconds
   to clear totals; the next full measurement again takes ten seconds.
6. Complete the five lessons in the engineering manual before changing hardware.

The software emits CSV count/environment readings. It does not measure calibrated
dose, certify a safe environment or prove sensor health when counts are zero.
The CAD is a guarded-module packaging draft. Battery, custom HV board and final
pocket enclosure work are not complete.

New review drafts: [secondary bench cover STEP/STL and fit lesson](mechanical/README.md)
and [low-voltage power-budget worksheet](docs/power-review.md). Reproduce the
planning arithmetic with `python tools/check_power_budget.py`. Current allowances,
guard dimensions, retention and temperature remain unmeasured; passing the script
is not hardware approval.

### Verification and troubleshooting

- Run `python -m pytest tests`; production counting-library host tests are
  documented in [tests/README.md](tests/README.md).
- Log validation checks raw window counts, elapsed time, total-count continuity
  and frequency agreement, not just the displayed CPM.
- WAIT: no full post-start/reset window yet. FAULT or `counts_valid=0`: counter
  overflow; stop the test, investigate, reset and record a new capture.
- `sensor_ok=0`: environmental data unavailable; it says nothing about GM health.
- Zero counts cannot distinguish quiet conditions from a broken detector path.
- Record actual results in the [bench worksheet](docs/bench-record.md).

<p align="center">
  <img src="docs/assets/iontrail-concepts.png" alt="IonTrail One enclosure study" width="920">
</p>

<p align="center">
  <strong>IonTrail One</strong><br>
  A pocketable environmental radiation counter designed to be explored, extended, and made your own.
</p>

<p align="center">
  <a href="docs/index.html"><strong>Product-page source</strong></a> ·
  <a href="docs/quickstart.md">SDK quick start</a> ·
  <a href="docs/hardware.md">Hardware architecture</a> ·
  <a href="docs/customization.md">Customization guide</a>
</p>

<p align="center">
  <img alt="Project status" src="https://img.shields.io/badge/status-EVT%20planning-ee8a3b">
  <img alt="Target" src="https://img.shields.io/badge/target-ESP32--C3-222222">
  <img alt="SDK license" src="https://img.shields.io/badge/SDK-MIT-blue">
</p>

---

## See the invisible environment

IonTrail One is a planned consumer environmental instrument built around a Geiger–Müller tube, an ESP32-C3, and temperature/humidity sensing. Its river-pebble enclosure uses a smoked transparent back to reveal the tube and custom PCB, while a structural loop lets it travel on a bag or lanyard.

The product is designed for home science, STEM learning, makers, collectors, and people curious about the world around them. CPM/CPS is the primary radiation measurement; environmental readings provide context for experiments and logged observations.

> **Development status:** concept and EVT planning. Dimensions, battery life, radiation response, dose conversion, ingress protection, and accuracy are not final specifications until verified on production-representative hardware.

## Product principles

| Principle | What it means |
| --- | --- |
| Understandable | The main screen explains the current count rate without presenting an unverified “safe/unsafe” judgement. |
| Customizable | The public SDK exposes count events, CPM, environmental readings, alarms, logging, and display hooks. |
| Repairable | A screwed enclosure, replaceable subassemblies, test points, and versioned hardware documentation are preferred. |
| Honest | Dose-rate estimates remain optional and require tube-specific calibration data supplied by the user or manufacturer. |
| Safe by design | The high-voltage section is physically guarded, current-limited, and inaccessible despite the transparent back. |

## Planned hardware

```mermaid
flowchart LR
    USB[USB-C 5 V] --> CHG[Protected charger<br/>and power path]
    CELL[Protected 1-cell LiPo<br/>with NTC] --> CHG
    CHG --> REG[3.3 V regulation]
    REG --> MCU[ESP32-C3]
    REG --> DISP[OLED display]
    REG --> ENV[SHT40 temperature/RH]
    REG --> HV[Regulated GM high voltage]
    HV --> TUBE[GM tube]
    TUBE --> PULSE[Pulse conditioning<br/>and input protection]
    PULSE --> MCU
    MCU --> FEEDBACK[Buzzer + pulse LED]
    MCU --> BLE[BLE customization API]
```

The ESP32-C3 SuperMini is used for fast EVT prototypes. A sellable revision should use a certified ESP32-C3 module on a custom PCB with traceable power components, protected battery charging, controlled high voltage, and production test points.

Read the full [hardware architecture](docs/hardware.md) and [prototype BOM](docs/bom.md).

## SDK overview

The SDK is organized as a small Arduino/PlatformIO library. Applications can subscribe to count-rate data, change display behavior, implement their own alarms, add BLE services, or connect additional I2C sensors without rewriting the pulse-counting core.

```cpp
#include <IonTrail.h>

IonTrailDevice ionTrail;

void setup() {
  IonTrailConfig config;
  config.gmPulsePin = 3;
  config.sampleWindowMs = 10000;
  config.deadTimeMicros = 0; // Enable only with characterized tube data.
  ionTrail.begin(config);
}

void loop() {
  ionTrail.update();

  if (ionTrail.hasFreshReading()) {
    Serial.printf("CPM: %.1f  Total: %lu\n",
                  ionTrail.cpm(),
                  ionTrail.totalCounts());
  }
}
```

### What customers can customize

- Screen layouts, icons, units, languages, and themes
- Count-click sound, LED behavior, and alarm thresholds
- BLE characteristics and companion applications
- CSV/JSON logging and cloud integrations
- Environmental sensors on the shared I2C expansion bus
- Experiment modes and classroom activities
- Tube-specific calibration profiles
- Power-saving and sampling strategies

The initial library deliberately does **not** make a universal CPM-to-µSv/h conversion. See [calibration and claims](docs/safety.md).

## Start developing

1. Install [Visual Studio Code](https://code.visualstudio.com/) and [PlatformIO](https://platformio.org/).
2. Clone this repository.
3. Connect an ESP32-C3 SuperMini by USB.
4. Review and confirm the pins in `include/iontrail_board.h` against your exact board revision.
5. Build and upload the default environment:

```bash
pio run
pio run --target upload
pio device monitor
```

See the [quick-start guide](docs/quickstart.md) before connecting any GM high-voltage hardware.

## Repository map

```text
IonTrail-One/
├── include/                 Board-level pin configuration
├── lib/IonTrail/            Reusable SDK library
├── src/                     Reference firmware application
├── examples/                Customer customization examples
├── docs/                    Product, hardware and SDK documentation
├── platformio.ini           Reproducible ESP32-C3 build
├── CONTRIBUTING.md          Contribution workflow
└── LICENSE                  MIT SDK licence
```

The static product site is ready in `docs/`. To publish it at `kelvinchan324.github.io/IonTrail-One`, enable **Settings → Pages → Build and deployment → GitHub Actions** once; the included workflow handles later deployments.

## Safety boundary

IonTrail One is an educational and environmental-monitoring project. It is not a certified personal dosimeter, contamination survey meter, medical device, or proof that an environment is safe. A GM circuit can contain several hundred volts even when powered by a small battery. Use a current-limited design, discharge path, insulating guard, and appropriate test equipment. Never connect a tube or high-voltage node directly to an ESP32 pin.

Read [Safety, calibration and product claims](docs/safety.md) before modifying detector hardware or publishing measurements.

## Contributing

Issues, documentation improvements, display themes, sensor integrations, translations, and tested hardware profiles are welcome. Keep safety-critical changes explicit and include test evidence where practical. See [CONTRIBUTING.md](CONTRIBUTING.md).

## Licence

The SDK and repository content are available under the [MIT License](LICENSE). Product names and logos are not granted for use as trademarks. Third-party libraries and hardware modules retain their own licences and terms.
