# Iteration 4 — climate availability and log consistency

5 October 2026. No board flashed, sensors connected, or physical tests performed.

Implemented a shared climate reading helper: failed/nonfinite/out-of-range-RH
measurements invalidate both values; cached display readings expire after 15 s;
only a successful new reading restores availability. Boot-time sensor absence
is retried once per completed counter window, while initialized drivers use
ordinary read retries. Removed the counter-init early return so the available
OLED can show INIT FAIL rather than remain on an initial screen.

CSV header unchanged. Offline validation now checks climate availability fields
when present, rejects incomplete climate headers and duplicate columns, and
keeps legacy counter-only capture support. README and the new
[climate lesson](climate-validity.md) distinguish absence from zero, sample age
from accuracy, and climate status from radiation detector health.

Verification:

- 51 Python tests pass (38 CSV cases and 13 power-budget cases); scoped Ruff passes.
- New native climate helper tests pass with `-Wall -Wextra -Werror`.
- Pinned ESP32-C3 firmware builds: RAM 14,648 bytes, flash 282,034 bytes.
- CI includes the climate helper test; new remote result remains to be checked.
- These tests do not exercise the full main loop, physical I2C retries, OLED
  text layout on hardware or fault recovery under a stuck bus.

Unfinished: hardware identification/retention, thermal and signal measurements,
real sensor calibration comparison, display reconnection, independent fault
supervision, and guarded-detector validation. The product remains an untested
engineering draft, not a safety instrument or calibrated dosimeter.
