# Hardware architecture

IonTrail One combines a long GM tube with a compact ESP32-C3 platform inside a river-pebble enclosure. The architecture below separates fast prototyping decisions from the requirements of a sellable product.

## EVT prototype

| Subsystem | Prototype choice | Purpose |
| --- | --- | --- |
| Controller | ESP32-C3 SuperMini | Firmware, USB development, BLE and logging |
| Detector | J305βγ or SBM-20 candidate | Compare sensitivity, length, supply needs and sourcing |
| High voltage | Current-limited 400 V GM module with pulse output | Rapid detector evaluation |
| Environment | SHT40 breakout | Temperature and relative humidity |
| Display | 0.96-inch SSD1306 OLED | Simple low-power interface |
| Battery | Protected 1-cell LiPo with NTC | Portable power |
| Charging | USB-C charger module for bench prototypes | Charging during EVT only |

## Product revision

A product PCB should replace the module stack. Use a traceable, certified ESP32-C3 module; USB-C input protection; a charger with power-path behavior; protected cell and temperature monitoring; regulated logic supply; controlled and current-limited GM high voltage; protected pulse conditioning; manufacturing test points; and a hardware revision identifier.

The transparent back should expose only safe surfaces. Cover the complete high-voltage zone with an opaque, mechanically retained insulating guard. The visual PCB can still reveal the tube, low-voltage components, antenna keep-out, and deliberate routing.

## Mechanical arrangement

- Place the tube along the widest internal diagonal.
- Support the tube near both ends with compliant silicone saddles; do not clamp the glass or metal wall rigidly.
- Make the attachment loop part of the internal chassis, not a cosmetic shell feature.
- Isolate impact loads from the tube, solder joints, battery, and transparent window.
- Vent the humidity sensor through a protected labyrinth away from warm components.
- Use screws for serviceability and reserve adhesives for validated seals or strain relief.

## Provisional pin map

The current firmware map lives in `include/iontrail_board.h`. Verify it against the actual development board before assembly. Generic “ESP32-C3 SuperMini” boards can vary by seller and revision.

## Validation gates before sale

1. GM tube operating voltage, plateau and pulse-interface characterization.
2. Instrument response testing against appropriate reference sources and facilities.
3. Battery, charging, thermal, short-circuit and fault testing.
4. USB, ESD, EMC/radio, drop, vibration and attachment-loop testing.
5. Enclosure material, flammability, accessibility and high-voltage protection review.
6. Applicable market compliance, labeling, transport and recycling requirements.

This document is an engineering plan, not a certification claim.

