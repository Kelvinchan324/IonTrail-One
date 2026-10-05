# Counter configuration and startup lesson

Status: source-reviewed and host-tested draft, 5 October 2026. No hardware test.

## Contract

The supported build uses ESP32-C3 GPIO3 with `FALLING`, one selected edge per
pulse, a 10,000 ms window and no software dead-time rejection. `begin` rejects:

- Negative GPIO values, numbers outside the target SoC domain, or an invalid
  GPIO according to the target mask. Validate the original integer before any
  narrowing conversion or mask shift. On C3 the numeric domain is 0 through 21.
- Any mode except `FALLING` or `RISING`. `CHANGE` counts both edges of a normal
  pulse; a level-triggered mode can repeatedly interrupt while the level persists.
- A zero window or window/dead-time interval at least 2^31; another library
  instance already owning the counter interrupt.

Invalid reconfiguration returns false without detaching, resetting totals,
discarding pending pulses or changing the window start. Valid reconfiguration
does reset acquisition. Call these methods from one foreground task only.

Numeric pin validity is **not board suitability**: flash, USB, boot straps,
reserved functions and board wiring still require review. GPIO3 is the only
reference detector input; this change does not authorize rewiring.

`begin == true` means accepted configuration and an attempted Arduino attachment,
not an acknowledged driver initialization. The pinned Arduino API returns void;
its internal service installation can fail and its GPIO calls are not checked by
this library. External code could replace the same interrupt. Likewise,
`counts_valid=1` means count arithmetic has not overflowed, not that the detector
or signal path is healthy. Zero counts cannot establish a safe environment.

## Reproduce the lesson without hardware

1. Run the counter and basic-example commands in [tests/README.md](../tests/README.md).
2. Inspect the rejected inputs: -1, 22, 255, 256, 259 and integer extremes.
   Explain why 259 becomes 3 when narrowed to eight bits.
3. Explain why counting both edges can double a pulse count. The implementation
   rejects that mode; do not try level-triggered counting on the detector.
4. Follow the active-measurement test: one completed window, one pending pulse,
   rejected reconfiguration, then another completed window. Neither the pending
   count nor original epoch may disappear.
5. Read the starter sketch's explicit failed-initialization branch. Test doubles
   inject competing library ownership; no real allocation failure is simulated.

## Pending physical acceptance

Use the existing [bench record](bench-record.md), with detector disconnected,
guard closed, and the approved 3.3 V low-voltage pulse fixture only. Check the
actual board GPIO mapping, input waveform levels and pulse widths, then record
three full windows at known pulse frequencies and validate the serial capture.
Repeat while display/climate I/O runs. Record loss or extra counts, not just CPM.
Do not call a successful build or synthetic test a measured throughput result.
Never probe an HV node or remove the vendor guard for this lesson.

## Primary source basis

- [Arduino-ESP32 2.0.17 GPIO implementation](https://github.com/espressif/arduino-esp32/blob/2.0.17/cores/esp32/esp32-hal-gpio.c)
  and [GPIO declarations](https://github.com/espressif/arduino-esp32/blob/2.0.17/cores/esp32/esp32-hal-gpio.h):
  eight-bit pin argument, mode constants and void attachment behavior.
- [Arduino pin mapping](https://github.com/espressif/arduino-esp32/blob/2.0.17/cores/esp32/Arduino.h)
  and [ESP-IDF 4.4.7 C3 capabilities](https://github.com/espressif/esp-idf/blob/v4.4.7/components/soc/esp32c3/include/soc/soc_caps.h):
  original integer must be range-checked before conversion/mask use.

Reviewed local PlatformIO framework 3.20017.241212+sha.dcc1105b. GPIO C-source
SHA-256: `9e9c47f299ce5a57ac6bd41b57eb0481df75eb3de17815f0aad4f5faa040f102`.
The native shim models the C3 numeric domain and attachment signature, not actual
ISR service allocation, electrical edge detection, concurrency or throughput.
