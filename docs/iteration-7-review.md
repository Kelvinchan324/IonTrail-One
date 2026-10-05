# Iteration 7 — bounded OLED availability recovery

5 October 2026. USB bench firmware draft; no detector, screen or bus connected.

An absent/failed OLED previously remained unavailable until reboot. The running
loop now probes its address once per two-second period and retries initialization
only while unavailable. A live display is not repeatedly initialized. A detected
loss suspends drawing; return requires successful begin and a post-init ACK.
Transition comments preserve the existing CSV schema. Count processing still
precedes optional display service, and UI time refreshes after synchronous I/O.

Pinned Adafruit SSD1306 2.5.17 implementation reviewed locally and against the
upstream version: existing framebuffer is reused, shared-Wire setup can remain
owned by the app, but drawing does not return a transfer success acknowledgement.
See [driver basis and physical gaps](display-recovery.md#driver-basis-and-limits).
We do not equate address presence with correct pixels or detector health.

Production-main-loop native tests now cover boot absence, init failure, retry
cadence/no catch-up burst, no live reinitialization, detected loss, post-init ACK
loss, recovery, delay versus count timestamp, transition comments and rollover.
The first test incorrectly expected immediate drawing on recovery; it was
corrected to the existing scheduled 500-ms refresh behavior, not hidden by a
firmware timing change. Vendor methods and I2C results remain scripted doubles.

Verification: application, counter and climate native tests pass; 54 pytest tests
pass, including a new CSV transition-comment case. Power-budget check remains a
planning calculation. ESP32-C3 build passes: RAM 14,648 bytes, flash 282,300 bytes.
No dependency, pin, count schema, CAD, power or detector-guard changes. New remote
CI result was not yet checked at commit time. No website deployment performed.

Still unverified: actual display/clone identification, pixel/layout quality,
power-loss recovery, wrong-device ACK, silent resets, retry heap/stack use,
shared-bus faults and count throughput during I/O. A two-second poll is not a
guaranteed detection latency or independent watchdog. Synchronous driver calls
can delay the loop; stale pixels cannot be trusted as current data. The README
now links a synthetic exercise and unfilled physical validation plan. Power off
before changing wiring; do not hot-plug the detector or open its HV guard.
