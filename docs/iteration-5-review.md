# Iteration 5 — production application-loop coverage

5 October 2026. Engineering draft; no firmware flashed or hardware connected.

Added a strict-warning native integration binary that compiles actual `setup()`
and `loop()` with the production counter. New test doubles sit exclusively under
`tests/native`: scripted Wire presence, SHT4x initialization/read outcomes and
delay, serial capture and SSD1306 text capture. No production behavior was changed.

Verified boot settings, completed-window CSV cadence/arithmetic, climate boot
absence and retry, failed initialization, failed/invalid readings and recovery,
delayed I/O timestamp handling, reset threshold and one-per-hold behavior,
pending pulse discard, counter initialization failure and clock rollover.
The test initially expected immediate display refresh after reset; corrected it
to respect the production 500-ms UI schedule. No firmware defect was inferred
from that test timing mistake.

Local verification: native application, counter and climate tests pass; Python
CSV/power suite passes (51 cases). Application test uses -Wall -Wextra -Werror
and is added to Engineering checks. Remote CI must be checked after push.
Embedded production sources/dependencies unchanged; no new embedded build claim.

Remaining: real Adafruit/I2C behavior and electrical fault tests, OLED rendering,
physical module retention/dimensions and current/heat measurements, detector
calibration. OLED boot absence still requires restart after a power-off fixture
change; no automatic reconnect is implemented. Simulated text is not pixel QA.
The README now links the application lesson and blank physical-evidence table.
