# Contributing to IonTrail One

Thank you for helping make IonTrail One more useful and understandable.

Good first contributions include documentation fixes, translations, display themes, data-export examples, and tested sensor integrations. Open an issue before starting a large change so the intended hardware or API direction can be discussed.

## Development workflow

1. Fork the repository and create a focused branch.
2. Keep board-specific wiring in `include/iontrail_board.h` and reusable behavior in `lib/IonTrail`.
3. Build the reference firmware with `pio run`.
4. Describe the hardware revision and test method in the pull request.
5. Update the documentation when behavior, pins, or public APIs change.

## Safety-sensitive changes

Changes involving high voltage, battery charging, alarms, calibration, or dose conversion need extra care. Include component ratings, source material, failure modes, and test evidence. Do not describe an uncalibrated result as a safety determination.

## Code style

- Prefer small, readable Arduino-compatible APIs.
- Avoid blocking delays in reusable library code.
- State units in names or documentation.
- Keep defaults conservative; calibration constants should never be silently assumed.
- Preserve compatibility with the ESP32-C3 reference environment where practical.

By contributing, you agree that your contribution is licensed under the repository's MIT license.
