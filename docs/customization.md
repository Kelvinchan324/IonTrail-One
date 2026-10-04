# Customization guide

IonTrail One is intended to be both a product and a development platform. The pulse-counting core stays small so customers can build new experiences around it without changing time-critical interrupt code.

## Recommended extension points

### Display and interaction

Keep UI code in the application layer under `src/`. A custom screen can read `cpm()`, `totalCounts()`, and `lastWindowMs()` after `hasFreshReading()` returns true. Themes should keep the count rate visible and avoid unsupported “safe” or “danger” labels.

### Feedback

Drive a low-current LED directly only if it is within the ESP32 pin limits. Use a transistor or suitable driver for buzzers and larger loads. Provide independent controls for click sound, LED pulse, and alarm behavior so the device remains discreet on a bag.

### Environmental sensors

The provisional board map reserves an I2C bus for an SHT40 and display. Additional sensors can share the bus if their addresses and power requirements are compatible. Avoid placing a temperature sensor next to the ESP32 regulator, charger, battery, or display where self-heating biases the result.

### Bluetooth Low Energy

A companion app can expose live CPM, total counts, temperature, humidity, battery status, device information, and user settings. Keep firmware updates authenticated and clearly separate experimental characteristics from stable ones.

### Logging and integrations

Store timestamps, CPM/CPS, raw counts, sampling window, battery state, environmental readings, hardware revision, firmware version, and calibration profile. Including metadata makes exported CSV or JSON data scientifically more useful.

### Tube profiles

A tube profile may contain the tube model, operating voltage range, plateau information, dead-time configuration, calibration provenance, date, and uncertainty. Do not ship one universal CPM-to-dose constant.

## API stability

The SDK is pre-1.0 while the product is in EVT. Releases may change pins and APIs. Customer projects should pin a tagged SDK version once releases begin. Breaking changes will be called out in release notes.

## Hardware customization boundary

Changing screens and data handling is low risk. Changing the GM high-voltage supply, tube bias, pulse interface, battery charger, or mechanical guard requires competent engineering review and validation. See [hardware](hardware.md) and [safety](safety.md).

