# Safety, calibration and product claims

## Intended use

IonTrail One is planned as an educational and environmental-monitoring instrument. It can make count-rate changes visible and help users log experiments. It is not a certified personal dosimeter, contamination survey meter, medical device, emergency instrument, or proof that an environment is safe.

## High voltage

GM tubes commonly operate at several hundred volts. Even a low-current supply can damage electronics, cause a startling shock, or retain charge. Product hardware needs current limiting, appropriate creepage and clearance, a discharge path, insulation, protected pulse conditioning, and a mechanically inaccessible high-voltage region. Only trained people with suitable equipment should work on the energized detector circuit.

## Battery and charging

Use a protected cell from a traceable supplier. Product hardware should monitor temperature, limit charging current to the cell specification, behave safely while operating from USB, and prevent reverse polarity, overcharge, over-discharge, and short-circuit faults. Do not enclose an unvalidated prototype battery in a consumer product.

## Count rate versus dose rate

CPM and CPS are direct instrument outputs. A dose-rate estimate depends on the tube, radiation type and energy, geometry, enclosure attenuation, electronics, calibration reference, and uncertainty. A constant copied from an unrelated counter is not valid evidence.

If a dose estimate is enabled, the UI and exported data should identify the tube profile, calibration source and uncertainty, and should continue to expose raw count-rate data.

## Alarm behavior

An alarm threshold is a user notification, not a safety verdict. Document its units, averaging interval, persistence and reset behavior. Test failure modes including an unplugged tube, saturated input, low battery, crashed firmware, muted sound and stuck button.

## Regulatory review

Before sale, obtain qualified advice for each destination market. Likely areas include product electrical safety, radio/EMC, battery transport, chemical/material restrictions, labeling, waste/recycling, consumer protection, privacy for connected features, and the claims made in advertising and instructions.

