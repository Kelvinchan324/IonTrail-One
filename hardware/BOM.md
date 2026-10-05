# IonTrail One EVT-A — bill of materials

USB-powered educational bench instrument using intact guarded SEN0463. No internal battery or custom high-voltage circuitry.

Prices and procurement approval are not supplied. Quantities are per prototype.

| Ref | Qty | Selection / value | Selection status | Source |
| --- | ---: | --- | --- | --- |
| U1 | 1 | ESP32-C3 SuperMini module | Provisional board revision; verify pins before assembly | [Design rationale](../docs/engineering.md) |
| J1 | 1 | USB data and 5 V cable | Prototype selection; bench verification required | [Design rationale](../docs/engineering.md) |
| U2 | 1 | AP2112K-3.3 LDO on breakout | Auxiliary sensors only; do not join to MCU 3V3 output | [Diodes AP2112](https://www.diodes.com/part/view/AP2112/) |
| C1 | 1 | 10 uF ceramic / >=10 V | Prototype selection; bench verification required | [Design rationale](../docs/engineering.md) |
| C2 | 1 | 10 uF ceramic / >=10 V | Prototype selection; bench verification required | [Design rationale](../docs/engineering.md) |
| U3 | 1 | DFRobot SEN0463 guarded detector | 107 x 42 mm board; larger guard envelope assumed | [DFRobot SEN0463](https://wiki.dfrobot.com/sen0463/) |
| R1 | 1 | 1k pulse series resistor | Not a high-voltage isolation component | [Design rationale](../docs/engineering.md) |
| U4 | 1 | SHT40 Adafruit breakout | Address 0x44; vented end; envelope assumed | [Adafruit SHT40](https://learn.adafruit.com/adafruit-sht40-temperature-humidity-sensor/pinouts) |
| U5 | 1 | SSD1306 128x64 I2C OLED | Select 3.3 V compatible module at 0x3C; verify pin order | [Adafruit SSD1306 library](https://github.com/adafruit/Adafruit_SSD1306) |
| S1 | 1 | Momentary reset-count button | Firmware: hold two seconds; internal pull-up | [Design rationale](../docs/engineering.md) |
| R2 | 1 | 1k status LED resistor | Prototype selection; bench verification required | [Design rationale](../docs/engineering.md) |
| D1 | 1 | Green indicator LED | Heartbeat only, not detector-health indication | [Design rationale](../docs/engineering.md) |
| H1 | 4 | M3 fixture screw + nut + washer set | Four plate/tray through-bolts; length TBD from 13 mm stack plus nut/washers; no guard retention | [Design rationale](../docs/engineering.md) |
| H2 | 1 | Secondary bench tray draft | Editable STEP/STL; material and protective performance unverified | [Enclosure review](../mechanical/README.md) |
| H3 | 1 | Secondary bench lid draft | Transparent material candidate; 2.5 mm thickness, no protective rating | [Enclosure review](../mechanical/README.md) |
| H4 | 4 | M3 lid through-bolt + nut + washer set | 3.4 mm holes; candidate M3x16 length requires stack and tool-access verification | [Enclosure review](../mechanical/README.md) |

## Net connections

Identical labels in the schematic are electrically connected.
NC means intentionally unconnected. Supply and connector ratings need physical verification.

- **USB_HOST**: U1.USB, J1.USB
- **USB_5V**: U1.5V, U2.1 VIN, U2.3 EN, C1.1
- **GND**: U1.GND, J1.GND, U2.2 GND, C1.2, C2.2, U3.-, U4.GND, U5.GND, S1.2, D1.K
- **COUNT_IN**: U1.GPIO3, R1.2
- **SDA**: U1.GPIO4, U4.SDA, U5.SDA
- **SCL**: U1.GPIO5, U4.SCL, U5.SCL
- **RESET_BTN**: U1.GPIO0, S1.1
- **LED_ANODE**: U1.GPIO7, R2.1
- **AUX_3V3**: U2.5 VOUT, C2.1, U3.+, U4.VIN, U5.VCC
- **PULSE_MODULE**: U3.D, R1.1
- **LED_LIMITED**: R2.2, D1.A
