# Verification without connected hardware

Run the CSV validation tests from the repository root:

```sh
python -m pip install pytest==9.1.1
python -m pytest tests
python tools/check_power_budget.py
```

Run the **production counter implementation** against a minimal host GPIO/time shim:

```sh
c++ -std=c++17 -Itests/native -Ilib/IonTrail/src tests/native/test_counter.cpp lib/IonTrail/src/IonTrail.cpp -o /tmp/iontrail-tests
/tmp/iontrail-tests
```

On Windows an optional compiler is available through
`python -m pip install ziglang==0.13.0`; replace `c++` with
`python -m ziglang c++` and choose an output under ignored `build/`.
Compile firmware separately with `pio run`.

The production climate-availability helper has an independent native test:

```sh
c++ -std=c++17 -Iinclude tests/native/test_climate.cpp -o /tmp/climate-tests
/tmp/climate-tests
```

It checks finite values, RH range, failed-read invalidation, the 15-second age
boundary, deliberate successful recovery and clock rollover. It does not execute
real I2C, the full main loop, OLED drawing, or vendor-driver initialization/retry.
Python CSV checks verify consistency of `sensor_ok` and climate fields when
present; legacy counter-only captures remain supported. Neither set of tests
measures environmental accuracy or establishes Geiger-module health.

## Production application-loop integration

```sh
c++ -std=c++17 -Wall -Wextra -Werror -Itests/native/app -Itests/native -Iinclude -Ilib/IonTrail/src tests/native/test_app.cpp lib/IonTrail/src/IonTrail.cpp -o /tmp/app-tests
/tmp/app-tests
```

This compiles the actual `src/main.cpp` setup/loop and production counter library.
Test-only doubles replace the GPIO/time, serial, Wire, SHT4x and SSD1306 boundaries;
they are not included in the embedded build. Cases cover startup pin/bus settings,
one CSV row per count window, climate boot absence/begin failure/read failure and
recovery, invalid humidity, delayed synchronous reads, reset hold/release and
discarded pending counts, counter-init failure, and clock rollover. It checks
display text after its scheduled 500-ms update, not immediately after each loop.

One deliberate regression case records a **limitation**, not a success criterion
for product readiness: an OLED absent at boot is not automatically retried.
Update that case when implementing a reviewed reconnection policy. Native text
capture does not render fonts, clipping, pixels, or actual I2C transfers. Driver
initialization/read behavior is scripted, not the real Adafruit implementation.
These tests are not a bus-hang watchdog, electrical fault injection, simultaneous
interrupt model, hardware reset test, or proof of detector health. No physical
measurement results are filled by the automated tests.

The power-budget tests check planning arithmetic, overload scenarios, invalid
numbers and input range. They do not provide measured current or temperature.
See [the power worksheet](../docs/power-review.md). To check draft solids locally,
install CadQuery 2.8.0 and run `python tools/build_enclosure.py`; CAD fit checks
use assumed component envelopes and are not included in the lightweight CI job.

The Python suite also checks `mechanical/enclosure-visual-review.json` input and
image hashes. Regenerate using `python tools/render_enclosure_review.py` with
CadQuery 2.8.0 and Matplotlib 3.11.2, then visually inspect before committing.
The renderer matches nine input bodies to the saved assembly by volume/bounds;
lightweight CI verifies freshness only, not CAD fit or electrical guarding.

The tests cover elapsed-window rate calculation, first-pulse/dead-time behavior,
reset of pending data, timer rollover, interrupt ownership, destruction and the
overflow arithmetic helper. They do not simulate simultaneous threads or prove
maximum interrupt throughput. Pending-counter overflow needs a hardware/fault
injection test; the helper test does not claim billions of physical interrupts.
Call foreground methods from one loop/task only. Do not destroy/reconfigure the
device concurrently with another foreground task.
