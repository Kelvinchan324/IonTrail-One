# EVT-A bench record — copy before testing

Status: **blank template; no measurement below has been performed**.
Use low-voltage pulse injection first. Keep the detector disconnected during that
test. Do not probe, modify or open its guarded high-voltage section.

## Traceability

| Field | Record |
| --- | --- |
| Date / operator / reviewer | Not recorded |
| Firmware commit and environment | Not recorded |
| MCU vendor / board revision / photograph | Not recorded |
| Detector model / guard revision | Not recorded |
| Display model / address / connector order | Not recorded |
| SHT40 breakout revision | Not recorded |
| Regulator exact part / package / breakout | Not recorded |
| Power source / current limit | Not recorded |
| Instruments / calibration dates | Not recorded |
| Ambient temperature / enclosure configuration | Not recorded |

## Unpowered and electrical checks

| Check | Expected design condition | Measured result / evidence | Pass? |
| --- | --- | --- | --- |
| Harness continuity | Matches revision netlist; no reversed supply | Not tested | — |
| Auxiliary and MCU 3.3 V outputs | Not directly tied together | Not tested | — |
| Detector guard | Factory guard intact, retention undisturbed | Not tested | — |
| USB rail | Within exact board/regulator specifications | Not tested | — |
| Auxiliary rail | Within selected module specifications at load | Not tested | — |
| I2C pull-up equivalent | Verify parallel breakout pull-ups and rise time | Not tested | — |
| Pulse high / low / width | Compatible with MCU limits and selected edge | Not tested | — |
| Auxiliary current and regulator temperature | Record at boot and after 30 min; assess exact datasheet/board limits | Not tested | — |
| Power-off interaction | No unintended powering through signal pins | Not tested | — |

Do not accept a hot regulator merely because its headline current rating exceeds
the load. The assumed auxiliary budget is 150 mA pending thermal measurements.

## Injected-pulse acceptance

Use a characterized 3.3 V pulse generator through 1k into GPIO3, common ground,
detector output disconnected. The supplied separate-board fixture is nominal
10 Hz, not a calibrated frequency standard; measure its output.

| Reference frequency | Expected counts per nominal 10 s | Expected CPM | Recorded count / interval / CPM | Evidence |
| ---: | ---: | ---: | --- | --- |
| 1 Hz | 10 | 60 | Not tested | — |
| 10 Hz | 100 | 600 | Not tested | — |
| 100 Hz | 1000 | 6000 | Not tested | — |

Capture at least three complete CSV windows at each frequency without pressing
reset mid-capture. Run `python tools/check_log.py capture.csv --hz <measured-Hz>`.
The default ±12 CPM comparison is an initial bench tolerance, not a calibrated
accuracy claim. Choose and justify tolerance using source uncertainty, gating and
count quantization. Keep raw logs and checker output with this record.

## Failure behavior and teaching evidence

| Exercise | Expected behavior | Actual evidence |
| --- | --- | --- |
| Start/reset | WAIT until a full new interval; totals cleared | Not tested |
| Button held longer than 2 s | One reset per hold, next reset only after release | Not tested |
| No pulses | Zero-count windows; no healthy/safe claim | Not tested |
| Missing SHT40 at boot | sensor_ok=0 and nan climate fields; counting continues; init retried per completed window | Not tested |
| Transient climate communication failure | Invalid climate fields immediately on failed read; new successful read restores them | Not tested |
| Climate sample ages beyond 15 s without a new valid reading | Display shows Climate unavailable, not cached numbers | Not tested |
| Inconsistent climate CSV validity flag | Offline validator rejects capture | Not tested |
| Counter initialization failure | INIT FAIL when OLED available; no count data rows; LED is not a health indicator | Not tested |
| Missing OLED at boot | CSV continues | Not tested |
| Interrupted CSV / duplicated row | Offline validator rejects capture | Not tested |
| Detector reconnected with power off | Intact guard, correct low-voltage connector | Not tested |

Instructor sign-off requires explaining CPM versus calibrated dose, distinguishing
MCU overflow from physical pulse loss, and identifying which checks need real
instruments. Never acquire radioactive material for these exercises.

Final disposition: **NOT TESTED**. Attach failures and revised drawings; do not
erase failed results when repeating a test.
