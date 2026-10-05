# Application behavior lesson — no hardware required

The native application test exercises the production firmware's `setup()` and
`loop()` with a real counter implementation and test-only board/driver doubles.
Use the [test instructions](../tests/README.md#production-application-loop-integration)
to compile and run it. Do not flash the host test doubles to a board.

## What to inspect

| Scenario | Expected application behavior |
| --- | --- |
| No completed count window | OLED text is WAIT; no numeric CSV row yet |
| Ten injected events in a 10-second window | 10 window counts, 60 CPM, total 10; synthetic only |
| Climate sensor absent at boot | Counts continue; climate fields NaN, sensor_ok 0; retry once per completed window |
| Climate initialization still fails | No getEvent call; next completed window retries initialization |
| Initialized climate read fails or humidity exceeds 100 | Previous climate numbers invalidated; next successful read can recover without begin |
| Simulated read takes 600 ms | CSV uptime remains count-window endpoint; display checks climate against the refreshed clock |
| Button low for less than 2000 ms | No totals reset |
| Button low for at least 2000 ms | One reset per uninterrupted hold; pending pulses discarded; new full window required |
| Counter cannot own the interrupt at setup | INIT FAIL display text; no numeric CSV rows |
| OLED absent at boot | Serial/counting still operate; OLED does not currently reconnect automatically |

After a reset, the visible WAIT message appears on the next scheduled display
refresh, nominally within 500 ms **if the loop and I2C calls keep running**.
This is not a measured display response time. If the loop stalls, the screen can
retain old text. The LED similarly indicates only foreground-loop activity.

## Teaching exercise

1. Run the test unchanged and record the PASS result as **host simulation**.
2. Read the `freshBoot(false, false)` section in the test. Identify which checks
   prove counting still works with the optional modules unavailable.
3. Find the delayed-read case. Explain why climate time and count-window endpoint
   are different, and why a fresh clock read is required before rendering.
4. Find the reset case. Explain why held buttons cannot repeatedly erase totals,
   and why a new full window is needed before another rate is reported.
5. Identify the external behaviors these doubles cannot prove: actual driver
   retry behavior, bus electrical faults, latency, pulse throughput and pixel layout.

## Separate physical evidence record — all pending

Use an approved low-voltage fixture and the existing
[bench procedure](bench-record.md) and [climate procedure](climate-validity.md).
Keep the detector's factory high-voltage guard intact. Do not short live bus lines
or improvise fault injection on a powered detector assembly.

| Item | Required evidence | Result |
| --- | --- | --- |
| Exact board/module identification | Revision, photos, verified pins and supply | Pending |
| Missing optional modules at cold boot | Power-off reconfiguration; saved serial capture | Pending |
| Supported climate recovery fixture | Reviewed setup and captured results | Pending |
| Reset button operation | Measured hold, release, new window and display response | Pending |
| OLED layout/readability | Actual 128x64 screen photographs, including INIT FAIL | Pending |
| Loop/bus timing | Measured latency and stop/stall behavior | Pending |

Passing host tests does not make IonTrail a calibrated dose instrument or a
personal safety meter. Zero counts still cannot establish detector health.
