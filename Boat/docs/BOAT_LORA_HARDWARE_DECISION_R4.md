# Active Boat LoRa decision R4: onboard SX1262 (2026-11-21)
Supersedes the R3 external-Boat-E22 configuration.

- Heltec Wireless Tracker integrated SX1262 over SPI, not E22/E227 UART.
- Boat does not need SC16IS750. Keep external nRF24 primary, Heltec only MCU,
  Cytron MDD20A direct PWM/DIR, separate WDI/permit and four coil drivers.
- Remote still has E22-900T22S UART via SC16IS750. Raw SX1262 RF packets
  are NOT assumed compatible with its transparent UART protocol. Backup
  remains disabled until peer PHY and framing pass paired-hardware tests.
- Pin count: 10 external native signals, 11 optimistic but UNSIGNED candidates,
  5 conservative candidates. No final V1.2 GPIO allocation.
- No release, no physical acceptance, sonar remains on hold.
Read Boat/docs/BOAT_INTEGRATED_SX1262_R8.md.
