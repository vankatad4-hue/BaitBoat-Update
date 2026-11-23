# R30H-13 — retired Remote direct-SPI RF24 adapter

These are unchanged copies of the old `Remote/source/nrf24_link.cpp` and `nrf24_link.h` as of 2026-11-23. They are excluded from both PlatformIO and the Arduino reference build.

The Remote Waveshare Standard has no electrically approved direct CE/CSN pair: GPIO6 is reserved for SD dummy CS / Sensor AD; GPIO15 belongs to RS485. Host SPI11–13 belongs to the TF card. Current Remote application uses separate SC18IS606 bridge SPI and MCP23017 @0x21 GPA0 CE; all RF flags remain 0.

**Boat is different** and retains its own direct-SPI `Boat/source/nrf24_link.*`, still uncommissioned. Never reintroduce the archived Remote direct-SPI sources into active compilation without board-level hardware audit.
