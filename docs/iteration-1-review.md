# Engineering review — iteration 1, 5 October 2026

Disposition: **continue; educational bench draft, not a calibrated safety instrument**.

## Delivered

- ESP32-C3 pulse-counting firmware with SHT40, optional SSD1306, status LED,
  hold-to-reset button and elapsed-time-based CPM CSV output.
- Intact SEN0463 module architecture; no custom high-voltage generator design.
- 13-row BOM, 11-net module schematic, six placement envelopes, editable manifest,
  drilled fixture STEP/STL and re-importable layout STEP.
- Pulse-injection fixture source and CSV checker; README/manual and five lessons.

## Evidence actually obtained

- PlatformIO 6.2.0 / Espressif32 6.10.0: esp32-c3-supermini compile PASS.
  RAM 14,632 bytes; flash 281,564 bytes at this iteration.
- CSV checker: four pytest tests PASS (synthetic data only).
- CadQuery 2.8.0: valid positive-volume fixture; seven layout solids re-imported.
- Manifest connection/geometry checks PASS. CI added; remote result pending push.

No actual pulse generator, detector, environmental sensor or enclosure was tested.
No CPM-to-dose factor, isotope identification or radiation protection claim is made.

## Next iteration, in priority order

1. Test the actual interrupt/counting library with a host time/GPIO shim, including
   reset, dead-time rejection and timer rollover; review shared ISR state.
2. Verify exact regulator package/pinout and rail budget; resolve generic board
   and display supplier choices, connector ordering and power-off interactions.
3. Replace layout-only guard with a parameterized, ventilated enclosure draft,
   explicitly gated by measured detector clearances and mounting dimensions.
4. Extend CSV validation to elapsed windows/count increments; pin software deps.
5. Physical owner: injected-pulse accuracy, missing sensor behavior, safe intact
   detector handling, current/temperature measurements and guarded assembly review.
