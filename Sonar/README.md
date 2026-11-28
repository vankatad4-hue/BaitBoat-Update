# Sonar coprocessor

Standalone PlatformIO target for the owner's 4 MB ESP32-DevKitC V4, intended
to host an Open Echo TUSS4470 Shield 002. The current target is only a
compile-time scaffold. It starts disarmed, does not configure sensor GPIO,
does not enable the Boat UART, and does not report a depth value.

## Build target

From the repository root, use `pio run -d Sonar -e devkitc_v4_compile` to compile
the scaffold. PlatformIO Upload is intentionally denied by the shared
`Boat/tools/pio_deny_upload.py` guard.

## Candidate pins

See [`src/sonar_pin_map.h`](src/sonar_pin_map.h) and the detailed
[Boat coprocessor recommendation](../Boat/docs/BOAT_ESP32_SONAR_COPROCESSOR_RECOMMENDATION_R2.md).
The pin constants are not yet an instruction to wire the shield.

`src/dbt_sentence.h` contains a side-effect-free integer formatter for valid
depths in Boat's accepted range. It is only a protocol helper; no measurement
source calls it and the current firmware does not send a DBT sentence.

## Work required before sensor firmware

1. Identify the exact ESP32 module and Shield 002 revision, selected capacitor
   jumpers, transducer model, and drive voltage.
2. Measure TUSS logic outputs and VOUT against the ESP32 input limits; add
   level conversion or analog scaling if measurements require it.
3. Port burst generation to an ESP32 timer peripheral and echo sampling to the
   ESP32 ADC continuous/DMA driver. Keep Wi-Fi disabled during capture and
   detect ADC pool overflow.
4. Validate the capture interval, depth threshold, blind zone, and sound-speed
   compensation against known distances before sending DBT.
5. Add the one-way UART DBT output only after Boat's serial ownership, GPIO
   map, freshness rules, and RF scheduling path are reviewed together.

The Boat currently assigns `HardwareSerial(2)` to GNSS and `HardwareSerial(1)`
to E22; Arduino `Serial` is also used for the debug console. A dedicated sonar
UART therefore needs an explicit serial/console allocation before its RX pin
can be connected. The proposed GPIO21 also replaces the optional nRF IRQ
candidate and must be reconciled in the canonical pin crosswalk first.

The upstream Open Echo UNO sketch is a reference, not portable ESP32 firmware:
it uses AVR Timer1 registers and AVR ADC registers. The NMEA DBT sketch declares
an ADC sample buffer but does not populate it, so its reported depth index may
be stale or zero. The raw-data example does capture samples, but its timing and
depth detection still need an ESP32 implementation and bench validation.
