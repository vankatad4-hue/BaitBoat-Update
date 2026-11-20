# Hardware test sketches — no actuator outputs

These test sketches are separate from normal firmware. They transmit only fixed bench test frames with CRC, **no authenticated commands**, and are intended for a bench with motors/solenoids disconnected.

## nRF24 pair
- Remote CE=GPIO6, CSN=GPIO15, SPI GPIO11/12/13, channel76.
- Disconnect/disable RS485 bus before GPIO15 CSN wiring; leave Sensor AD unused while GPIO6 is CE.
- Boat CE/CSN macros remain -1; fill only after exact Tracker board pin audit.
- Both modules should use matching PA level, channel, data rate, address pipes, and payload settings.
- Test sketches send small fixed frames at low rate; RF24 hardware auto-ack can be observed in driver status. This is not app-level authentication or boat control.

## E22 pair
- Remote SC16IS750 I2C bridge @0x48; Wire GPIO8/9. E22 UART 9600 8N1.
- Boat direct UART test sketch requires exact UART/M0/M1/AUX pins set from Tracker schematic; defaults disabled.
- Both configure channel18/868.125MHz, 62.5kbps air rate, 32-byte subpacket, 10dBm, transparent mode.
- Test sketch transmits fixed CRC-only probe frames at 5s/7.5s intervals. This is deliberately unauthenticated test traffic, but no commands or actuator payloads.
- Keep antennas connected and test at low power/short range first. Duty limiter is RAM-only and not field-compliant.

## Compile / run
1. Install Arduino-ESP32 core and RF24 library versions that match the project; record versions.
2. Select correct ESP32-S3 board/USB settings.
3. Open the sketch in its own folder and confirm relative source includes are resolved. If IDE doesn't compile included source, copy required .h/.cpp from the matching project source folder into the test sketch folder.
4. Wire only with current-limited bench supply. Use test logs; do not connect boat motors/solenoids.
5. Record compile log, module config readback, packet TX/RX, RSSI if available, packet error and current.
6. Test the nRF and LoRa separately, then test concurrent radio power and Wi-Fi/SD activity.

## Safety / limitations
- The nRF test sketch uses GPIO6/15 assignments only as a provisional remote bench configuration.
- Boat pins are not allocated; test remains disabled until the pin audit.
- LoRa module settings are candidate values; verify E22 manual and readback.
- No cryptographic key, app ACK, failsafe, motor or solenoid action is included.
