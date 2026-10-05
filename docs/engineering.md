# IonTrail One EVT-A engineering draft

The current build is a USB-powered bench instrument using an intact DFRobot
SEN0463 detector inside its guard. This is larger than the original pocket
concept. It deliberately establishes counts, display, environmental context
and logging before a custom battery or high-voltage board is designed.

## Build package

- [Pin/net schematic](../hardware/schematic.svg) and [BOM](../hardware/BOM.md).
- [Editable revision](../hardware/rev-a.json).
- [Placement drawing](../mechanical/placement.svg), [STEP assembly](../mechanical/layout-draft.step).
- [Fixture plate STEP](../mechanical/fixture-plate.step) / [STL](../mechanical/fixture-plate.stl).
- Regenerate with `python tools/build_engineering.py --cad` (CadQuery 2.8.0).
  Without `--cad`, only Python standard library is needed.

These are module-level connection schematics and envelope CAD. No custom
high-voltage PCB, battery charger, enclosure ingress rating or calibrated dose
conversion is released. Unused pins in the original concept remain unconnected.

## Electrical design

USB powers the selected ESP32-C3 SuperMini and AP2112K-3.3 auxiliary regulator.
Confirm the exact SuperMini board schematic; the name alone does not identify a
single vendor revision. The AP2112 pin list is for SOT25, not other packages.
The manufacturer's [DS39724 revision 2-2](https://www.diodes.com/datasheet/download/AP2112.pdf)
pin diagram confirms VIN=1, GND=2, EN=3, NC=4 and VOUT=5 for SOT25.
The diagram does not establish the terminal order of an assembled breakout.
Use a properly assembled breakout and local input/output ceramic capacitors.
The auxiliary 3.3 V rail supplies the detector, screen and SHT40.
It must NOT be tied to the MCU's regulator output.

All signals are referenced to common GND. SEN0463 D connects through 1k to
GPIO3; its supply is 3.3 V in this build. Use only the low-voltage signal connector.
GPIO4/5 are SDA/SCL at 100 kHz, with 3.3 V pull-ups supplied by the breakouts.
Measure equivalent pull-up resistance before adding more. Sensor address is
0x44; display is a 3.3 V compatible SSD1306 128x64 at 0x3C. Verify every purchased
module's connector order; silkscreen order is not standardized.

GPIO0 has a normally-open button to ground; hold two seconds to reset totals.
GPIO7 drives a green LED through 1k. It indicates firmware activity, not detector
health. GPIO2 is unused because the previous battery ADC assignment conflicts
with a boot-strapping pin. There is no battery in EVT-A.

Sources accessed 5 October 2026:
[DFRobot SEN0463](https://wiki.dfrobot.com/sen0463/),
[Diodes AP2112](https://www.diodes.com/part/view/AP2112/),
[Adafruit SHT40](https://learn.adafruit.com/adafruit-sht40-temperature-humidity-sensor/pinouts).
The detector board is listed as 107 x 42 mm; the CAD allows a larger guard volume.
Check the supplied guard dimensions. Keep the module intact: its internal
operating voltage is approximately 400 V despite its low-voltage input.

Before powering, measure rails without sensors and check polarity. Begin with
a current-limited USB source. Budget auxiliary current initially at <=150 mA
pending measurements; AP2112 dissipation is (5-3.3)*I, e.g. 0.255 W at 150 mA.
The module regulator rating alone does not prove acceptable breakout temperature.
USB connector, trace capacity and total board current still need verification.

## Packaging

The 170 x 100 x 4 mm fixture plate is a bench layout, not the pocket enclosure.
Detector occupies the long lower bay. MCU/USB is at the opposite edge;
SHT40 is at the outer upper corner, separated from regulator heat.
OLED faces outward and the count-reset button remains reachable.
No vent or screw may defeat the original detector guard.
All positions in [placements.json](../mechanical/placements.json) are editable
assumptions; module mounting holes are not invented. Retain factory guard mounts
and develop the final retention features from measurements.

## Firmware and user behaviour

Screen reports WAIT until a full 10-second interval completes. The reported CPM
is pulses * 60000 / measured interval in milliseconds. No universal dose factor,
background subtraction, alarm threshold or safe/unsafe label is applied.
SHT40 failure displays unavailable, while counting and serial output continue.
A missing display is tolerated; the serial output remains the primary diagnostic.
Reconnect missing I2C modules with power off and restart to re-detect them.

USB serial runs at 115200 and emits a CSV header, one row per complete window,
and '#' diagnostic lines. Capture it to a log using a serial terminal.
The current columns are `uptime_ms,window_ms,window_counts,cpm,total_counts,
temperature_c,humidity_percent,sensor_ok,counts_valid` (one header line).
Counts saturate instead of wrapping; overflow latches `counts_valid=0`, emits
nonfinite CPM and displays FAULT. Stop the capture, investigate and reset before
starting another test. This detects integer overflow, not detector saturation,
unobserved pulses or a disconnected cable.

The optional dead-time filter accepts the first pulse, then rejects intervals
shorter than the configured value; it is disabled in the reference application.
It does not correct a GM tube's physical dead time. Foreground SDK calls belong
to one loop/task. `end()` releases the interrupt so another instance can start.
A ten-second window has coarse resolution: one count contributes 6 CPM.
For independent Poisson events, the approximate standard deviation scales as
sqrt(N); tiny samples are noisy. This describes counting statistics, not detector
energy response or calibrated accuracy. Zero counts can mean either quiet
conditions or broken hardware.

## Teaching and test plan

1. **Pulse counting (45 min):** leave detector disconnected. Use a 3.3 V signal
   generator, or the separate-board [pulse fixture](../tools/pulse_fixture/pulse_fixture.ino).
   Its GPIO4 drives low pulses through 1k into GPIO3; join grounds. Never combine
   the fixture output and detector output. Ten hertz should yield about 600 CPM.
2. **Reproducibility (30 min):** capture >=3 windows, then run
   `python tools/check_log.py capture.csv --hz 10`. The first captured window is
   excluded only from the frequency comparison; its arithmetic must still be valid.
   Later rows must have contiguous uptime, matching count increments, valid flags
   and CPM consistent with their raw counts. Missing rows, resets during a capture,
   and malformed data fail. Repeat at 1, 10 and 100 Hz with a characterized generator and record
   raw counts, exact period, rejected/missing pulses and measured signal level.
3. **Display/sensors (30 min):** disconnect one I2C module with power off.
   Restart; verify count logging survives. Compare temperature after warm-up
   with a reference placed outside the enclosure.
4. **Background measurement (60 min):** connect the guarded detector, collect
   background counts only, compare 10-second and longer aggregated windows.
   Do not acquire or handle radioactive sources for this lesson.
5. **Engineering review (30 min):** check reset, power cycle, CSV import,
   connector strain and regulator temperature. Attach measurements and firmware hash.

Completion gates: exact MCU/display revision, verified guard retention, injected
pulse fidelity, power/thermal measurements and calibrated-reference assessment.
Battery and pocket enclosure design follow those results.

Use the [blank bench record](bench-record.md) for measurements and
[software test guide](../tests/README.md) for reproducible host checks.
