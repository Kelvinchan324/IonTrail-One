# Low-voltage power planning and test worksheet

No measured current, heat rise, USB compliance or battery runtime is claimed.
Use `python tools/check_power_budget.py` to reproduce the arithmetic from
[`hardware/power-budget.json`](../hardware/power-budget.json). A zero exit code
means the planning inequalities pass, NOT that hardware has passed.

## Inputs and arithmetic

The host allowance is 350 mA. Auxiliary allowances are detector 30 mA, SHT40
breakout 5 mA, OLED 40 mA and reserve 10 mA. **All are design allocations, not
vendor maxima or measurements.** Heater remains disabled in the firmware.
The 25% auxiliary margin gives 106.25 mA; input allowance becomes 456.33 mA,
including 0.08 mA regulator quiescent current. That leaves only 43.67 mA against
the chosen 500 mA planning budget; startup peaks may exceed it.

The [AP2112 datasheet, DS39724 Rev 2-2](https://www.diodes.com/datasheet/download/AP2112.pdf)
lists 80 uA maximum no-load quiescent current under its stated test conditions,
600 mA output rating and 184 C/W SOT25 junction-to-ambient reference. Those values
do not characterize an arbitrary breakout. This calculator accepts 4.3–6 V for
the fixed 3.3 V reference design; it does not model dropout or transient response.

At nominal 5 V, LDO loss is `(5 - 3.3) * 0.10625 + 5 * 0.00008 = 0.181025 W`.
Using the datasheet thermal reference at an assumed 40 C ambient gives about
73.3 C junction. At a 5.25 V scenario, the estimate increases to 78.2 C. The
100 C review threshold is a chosen engineering screen, not a touch-temperature
limit. Enclosed air, copper area and adjacent heat make these estimates uncertain.
Neither the regulator's maximum current nor its thermal shutdown approves the
assembled unit. Do not use shutdown as normal temperature control.

The [DFRobot module specification](https://wiki.dfrobot.com/sen0463/) lists its
low-voltage input range but does not provide a worst-case input current in the
reviewed table. Therefore the detector's 30 mA allowance still needs measurement.

## Supply and power-off boundaries

- The 500 mA number is a planning budget, not evidence of USB enumeration,
  Type-C current advertisement or port entitlement. Use an appropriately rated,
  current-limited source and verify the exact MCU board's USB/5 V path.
- Do not back-power the MCU through GPIO, I2C or count output. Disconnect both
  USB and any separate pulse fixture before changing connections.
- Do not tie auxiliary 3.3 V to the board regulator output. Breakout LEDs and
  regulator variants can change real consumption.
- AP2112 output discharge does not establish that the detector's internal high
  voltage is discharged. Keep its original enclosure closed even after USB is
  removed; no safe internal access waiting time is asserted.
- No battery, charger or custom high-voltage circuit is authorized by this draft.

## Bench lesson (45–60 minutes, only after wiring review)

Start without the detector connected. Check polarity and rails, then use the
low-voltage pulse fixture. Log current at startup, idle, full-white OLED and
normal operation. Power off before adding the intact detector. Measure supply
current and breakout surface temperature without opening or touching its HV area.
Do not deliberately short the supply or block ventilation as a classroom test.

| Record | Result |
| --- | --- |
| Source model, current limit, cable, actual voltage | pending |
| Exact MCU / OLED / regulator breakout revisions | pending |
| Startup peak current and measurement bandwidth | pending |
| Host current; each auxiliary current; full-white OLED | pending |
| Ambient; regulator surface; 10/30/60-minute temperature | pending |
| Auxiliary rail minimum during transitions / oscilloscope evidence | pending |
| Enclosure closed versus open environmental-reading bias | pending |
| Reviewer disposition / revised allowances | pending |

Update measured evidence separately from planning inputs. A budget passing does
not authorize powered use of unreviewed mechanical packaging.
