# Engineering review — iteration 2, 5 October 2026

Disposition: **software draft improved; physical build gates remain open**.

## Findings and corrections

- Optional dead-time filtering previously used a zero timestamp as though a pulse
  had already occurred. An explicit first-pulse flag now accepts the first event,
  including at microsecond zero after startup/reset.
- ISR timestamp, pending count and reset state now share the same critical section.
  Tests check logic and balanced lock calls, not real simultaneous execution.
- Pending and total counts no longer silently wrap. Overflow latches a fault;
  CSV validity and display FAULT expose it. Reset starts a new measurement.
- The device now owns its interrupt explicitly, releases it with end/destruction,
  rejects a second active instance and is not copyable.
- CSV includes window_counts and counts_valid. Offline validation checks elapsed
  time, raw-count CPM arithmetic, contiguous totals and uptime, including rollover.
- Tested sensor-library versions are pinned in PlatformIO.
- README troubleshooting, test guide and blank bench worksheet added.
- AP2112 SOT25 pin mapping checked against manufacturer DS39724 Rev. 2-2;
  assembled breakout selection/order and thermal measurements remain unresolved.

## Evidence obtained

| Check | Result | Boundary |
| --- | --- | --- |
| Python CSV validator | 21 passed | Synthetic captures only |
| Native production counter library | PASS | Mock GPIO/time, no concurrent interrupt model |
| ESP32-C3 firmware | Compile PASS | RAM 14,640 bytes; flash 281,822 bytes |
| Previous engineering GitHub workflow | PASS, commit 948a4ed | New commit's remote result still to be checked |

Native cases: 10 Hz count rate; real elapsed interval after delayed update;
zero-count interval; pending-data reset; first pulse; exact dead-time boundary;
micros/millis rollover; invalid interval configuration; ownership and destructor;
saturating arithmetic at uint32 limits.

No physical pulse-rate sweep, detector response, thermal or electrical test
has been performed. Arithmetic overflow detection does not detect missed pulses,
tube dead time, disconnected sensors or unsafe environmental conditions.

## Next priorities

1. Parameterized enclosure/guard-retention draft around the intact vendor module,
   with fabrication gates for actual mounting/guard dimensions.
2. Resolve exact MCU/display/regulator breakout procurement and connector schedule.
3. Verify power-off signal interactions and thermal/current budget on hardware.
4. Run the blank bench worksheet with instruments; preserve raw logs and failures.
5. Expand firmware-facing failure tests and measured interrupt-throughput evidence.

The pre-existing product-page deployment workflow failed on both iteration 1 and
its predecessor. No deployment settings were changed or rerun.
