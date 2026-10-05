# Iteration 3 — bench cover and low-voltage budget

2026-10-05. Still an untested engineering draft; not a pocket product or safety meter.

## Delivered

- Parametric secondary tray/lid around the existing fixture and intact guarded
  module allowance. Editable STEP/STL and a nine-solid assembly STEP.
- Four fixture supports, four external lid ears, provisional USB opening and
  vents over the climate-sensor bay. No holes in the original detector enclosure.
- Machine-readable planning loads and a reproducible budget checker; nominal
  current allowance 456.33 mA, auxiliary allocation with margin 106.25 mA.
- Material/retention/clearance and power/thermal worksheets, README teaching links.
  BOM regenerated with tray, lid and lid-fastener candidates (16 rows total).

## Evidence

- 34 Python tests pass, including 13 power-budget cases; scoped Ruff checks pass.
- Nominal budget inequalities pass. Estimated LDO loss 0.181025 W; reference
  junction estimate 73.309 C at assumed 40 C ambient. These are not measurements.
- CAD checks: each part is one valid positive-volume solid; no intrusion into
  component envelopes or the fixture; individual and assembly STEP reimport pass.
  Assembly has nine solids. Lid footprint 202 x 107 mm, nominal cover height
  53.5 mm before unresolved feet/fastener projections.
- No embedded firmware changes in this iteration. No new firmware/hardware
  execution claim; existing counter tests and build remain covered by CI.

## Next gates

1. Measure the intact factory guard and its permitted retention interfaces. The
   published board dimensions do not certify the assumed guard envelope.
2. Resolve positive module retention, cable strain relief, USB mating and feet.
   The count-reset button is inaccessible under this lid until an actuator is designed.
3. Select verified material, lid transparency and fastener lengths; assess
   touch access, deflection, electrical insulation, flammability and drop behavior.
4. Measure startup/full-white display/normal current and enclosed heat rise;
   current allocations are guesses for planning, not worst-case vendor ratings.
5. Complete physical pulse-injection and background-count records. Enclosure
   material may alter detector response; no calibrated-dose claim follows.

Verdict: substantive packaging and power draft, **not fabrication/power-up approval**.
