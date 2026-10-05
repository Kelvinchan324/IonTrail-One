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

The power-budget tests check planning arithmetic, overload scenarios, invalid
numbers and input range. They do not provide measured current or temperature.
See [the power worksheet](../docs/power-review.md). To check draft solids locally,
install CadQuery 2.8.0 and run `python tools/build_enclosure.py`; CAD fit checks
use assumed component envelopes and are not included in the lightweight CI job.

The tests cover elapsed-window rate calculation, first-pulse/dead-time behavior,
reset of pending data, timer rollover, interrupt ownership, destruction and the
overflow arithmetic helper. They do not simulate simultaneous threads or prove
maximum interrupt throughput. Pending-counter overflow needs a hardware/fault
injection test; the helper test does not claim billions of physical interrupts.
Call foreground methods from one loop/task only. Do not destroy/reconfigure the
device concurrently with another foreground task.
