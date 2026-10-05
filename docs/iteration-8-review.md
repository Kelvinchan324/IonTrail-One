# Iteration 8 — reject invalid counter configuration before GPIO side effects

5 October 2026. Software draft only; no connected detector or hardware test.

The reusable counter previously rejected negative GPIO numbers but accepted
oversized positive integers and arbitrary interrupt modes. The pinned Arduino
API narrows its pin argument to eight bits: 259 could target GPIO3. A both-edge
mode could count twice per pulse; level modes are unsuitable for this counter.

`begin` now checks the original integer against the target GPIO range/mask and
accepts only FALLING or RISING. Validation precedes every pin/interrupt operation
and preserves an active acquisition when rejecting a new configuration. The
GPIO3/FALLING reference build and CSV schema are unchanged. The starter sketch
and README example now check initialization and suppress readings on failure.

Counter tests cover invalid integer boundaries and modes before/after successful
startup, zero GPIO side effects on rejection, pending-count/last-reading/window
preservation and accepted edge modes. A fourth native executable includes the
actual basic-counter sketch and tests success and competing-owner failure. CI
now runs it. The host shim models the pinned eight-bit attachment signature and
C3 GPIO domain, not a real interrupt subsystem.

Verification: all four native suites pass with C++17 and strict warnings;
54 Python tests and the power-planning arithmetic pass. ESP32-C3 firmware build
passes: RAM 14,648 bytes, flash 282,342 bytes. Remote engineering CI is pending
at commit time. No website deployed, hardware flashed, or CAD/BOM/pins changed.

The [configuration lesson](counter-configuration.md) records primary sources,
reproduction steps and pending low-voltage physical tests. A successful `begin`
still cannot acknowledge Arduino's void attachment call or prove detector
health. Board pin reservations, interrupt allocation failure, real pulse
throughput, electrical integrity, calibration and enclosure retention remain
unverified. The passing software tests do not make this a finished product.
