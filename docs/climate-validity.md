# Environmental data validity and recovery

Date: 5 October 2026. Software-tested draft; **no sensor, display or electrical
failure experiment has been performed**. Environmental availability is separate
from pulse counting. Neither `sensor_ok` nor `counts_valid` establishes detector
health, dose calibration or environmental safety.

## Firmware behavior

| Condition | CSV / display behavior | Recovery |
| --- | --- | --- |
| Climate module absent at boot | `nan,nan,0` climate fields; count windows continue | Try initialization once per completed count window |
| Initialized module read fails | Immediately invalidate cached climate data for the next row/display update | Retry ordinary measurement on the next completed window |
| Read reports success but values are nonfinite or RH is outside 0–100 | Treat the whole climate pair as unavailable | Require a new valid reading |
| No new valid climate sample for more than 15 seconds | Display says Climate unavailable | New valid sample restores display |
| Counter initialization fails | OLED says INIT FAIL if available; no count data rows | Inspect hardware/configuration and deliberately restart after correction |
| Display absent at boot | Serial CSV remains the output | OLED automatic reconnection is not implemented |

The current nominal count window is ten seconds. The 15-second climate display
age bound is a software policy allowing ordinary scheduling delay, not a sensor
specification or a safety limit. Validity expires strictly beyond that bound.
The CSV climate sample is taken when a count window completes; it is a point
sample, not a ten-second environmental average. `uptime_ms` continues to label
the count-window endpoint before climate I/O, so the two acquisitions are not
simultaneous. A reset postpones the next complete count window and can cause
the previous climate display value to expire.

After successful initialization the firmware keeps that driver instance and
retries `getEvent`, without repeated `begin` calls. Before first success, it
probes address 0x44 and attempts initialization at most once per completed count
window. A responding address is not enough: availability still requires a
successful measurement. No automatic bus power cycling is performed.

The pinned [Adafruit SHT4x 1.0.5 source](https://github.com/adafruit/Adafruit_SHT4X/blob/1.0.5/Adafruit_SHT4x.cpp)
checks I2C operations and CRC in its combined measurement method, clamps the
converted humidity to 0–100, and returns a success flag. IonTrail uses that
combined method, then applies its own finite-number and humidity-range checks.
This does not detect calibration drift, a plausible but wrong value, heat bias
from the enclosure, or all electrical faults. The heater remains disabled.

I2C calls are synchronous, with the existing 50 ms Wire timeout. An MCU freeze,
driver hang or electrically stuck bus is not an independently supervised fault.
If the foreground loop hangs, the display can freeze too. The status LED means
only foreground-loop activity; it is never a healthy/safe indicator.

## CSV contract

The existing header is unchanged. When climate columns are present, the checker
requires all three: `temperature_c`, `humidity_percent`, `sensor_ok`.

- `sensor_ok=1`: both values finite, humidity within 0–100.
- `sensor_ok=0`: both numeric fields must be `nan`, not zeros or cached readings.
- Missing climate columns as a complete group remain valid for legacy
  counter-only tests. Partial groups and duplicate CSV column names are rejected.

Temperature has no operating-limit acceptance test here; finite does not mean
accurate, calibrated or inside the exact hardware's rated environment. A passing
log check establishes arithmetic/schema consistency and comparison against the
requested injected frequency only. It cannot assess the sensor's true accuracy.

## Teaching exercise and physical evidence gate

1. Run the native climate test and `python -m pytest tests/test_check_log.py`.
   Contrast zero degrees/zero RH with unavailable `nan` values.
2. In a synthetic CSV copy, set `sensor_ok=1` while temperature is `nan`; confirm
   rejection. Set both climate fields to `nan` with `sensor_ok=0`; valid pulse
   data should still pass. Do not label the synthetic file as a measurement.
3. Predict the 15,000/15,001 ms validity boundary and clock-rollover result.
4. Before physical experiments, obtain a reviewed low-voltage fixture/test
   procedure with the detector disconnected. Configure missing modules only
   while power is off. Do not hot-plug the shared I2C harness or touch/open the
   detector's high-voltage guard to force faults.
5. Record actual startup absence, transient communication fault, recovery and
   delayed-sample display behavior in the [bench record](bench-record.md). Use an
   approved fixture or simulated driver for transient faults; do not short bus
   lines. Record count continuity separately from climate availability.

Required evidence remains blank: exact SHT40 breakout and pull-ups, bus rise time,
timeouts/recovery latency, OLED behavior, power-off signal interactions, reference
temperature/RH comparison and enclosure heat bias. The next physical build is
not approved merely because the host tests or firmware compile pass.
