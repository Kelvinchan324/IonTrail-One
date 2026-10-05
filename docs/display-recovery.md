# OLED availability and recovery lesson

5 October 2026. Tested with scripted host doubles and an embedded build, not a
real display or I2C fault-injection fixture. Keep the detector guard intact.

## What the firmware does

At boot, the firmware probes the configured display address and initializes the
SSD1306 only if the address acknowledges. It probes again at most once every
2,000 ms of the running loop; delayed loops make one attempt, not a catch-up
burst. A missing acknowledgement clears display availability and suppresses
future drawing until recovery. Failed initialization is retried on that same
schedule. When the display is already available, a successful probe does not
reinitialize it or clear its buffer.

Recovery uses the existing display object, with hardware reset and shared-Wire
initialization disabled. It checks acknowledgement again after initialization.
New contents appear on the next scheduled 500-ms UI refresh, not necessarily in
the same loop. A lost acknowledgement during initialization is not reported as
recovery. Transition comments start with `# oled`; CSV columns are unchanged.

| Observation | Interpretation |
| --- | --- |
| `# oled unavailable` | A scheduled address probe failed; drawing is suspended |
| `# oled available; address ACK, pixels unverified` | Probe/init/post-probe passed, but visible output is unconfirmed |
| Count rows continue | The foreground counter is producing records; not proof of detector or display health |
| Frozen image or no new count rows | Investigate the system; never interpret old pixels as a current measurement |

Count-window timestamps remain the counter snapshot endpoint, before climate or
display I/O. UI time is refreshed after synchronous display recovery. A retry can
delay the loop; the two-second schedule is **not** a maximum fault-detection or
stop-latency guarantee. Probe-only checks can miss short outages, silent display
resets or failed pixels while the controller still acknowledges.

## Driver basis and limits

Reviewed pinned [Adafruit SSD1306 2.5.17 source](https://github.com/adafruit/Adafruit_SSD1306/blob/2.5.17/Adafruit_SSD1306.cpp):
`begin()` allocates a framebuffer only if one is not already present; retries on
the same object reuse it. The false `periphBegin` argument leaves the shared bus
setup to the application. `display()` returns no transfer result, and internal
I2C transmission results are not propagated as a successful-frame acknowledgement.
Thus `displayReady` means software initialization plus sampled address presence,
not verified visible content. Address acknowledgement also does not identify an
SSD1306 uniquely. Confirm the actual module and pin order before any wiring.

Installed source SHA-256:
`ddbd788bfaf97dd3b5ba5ee67eb62ab51652cbe86e34c2e7d24d5e074bb34bdd`.
This review does not prove behavior of every clone, power transient or stuck bus.
The shared Wire timeout is configured to 50 ms, but multiple synchronous driver
transfers, driver faults and MCU failure are not an independent watchdog.

## No-hardware classroom exercise

1. Run the production application-loop test from [tests/README.md](../tests/README.md).
2. Explain why startup without an OLED can still produce serial count windows.
   Distinguish missing address, failed initialization and failed screen pixels.
3. Trace the test's absence/reappearance sequence. A 31,999-ms UI refresh followed
   by recovery at 32,000 ms does not immediately draw; the next eligible UI update
   is at 32,499 ms. Do not treat a scheduling assertion as measured wall-clock time.
4. Inspect the delayed-recovery test: count row endpoint remains 10,000 ms while
   the simulated UI clock advances to 10,600 ms. Explain why changing the count
   timestamp to the later time would misrepresent the count interval.
5. Explain what remains invisible to an ACK probe: frozen pixels, wrong display
   identity, silent reset, data corruption and a failed Geiger detector.

## Physical validation — all pending

Record exact OLED/controller marking, board revision, supply/current, actual
bus levels, source commit and test equipment. Use a separate reviewed low-voltage
test fixture with detector disconnected. De-energize before modifying wiring;
do not hot-plug the detector or expose its high-voltage section.

Measure startup/recovery latency, count-window continuity, bus timeout behavior,
visible text/layout, retry memory/stack use and response to controlled power loss.
Check wrong-address and missing-module cases. A fault test must not depend on an
unreviewed live cable disconnection. Keep failed trials and actual traces; never
copy host-test outputs into measured-result fields. This is not a safety meter.
