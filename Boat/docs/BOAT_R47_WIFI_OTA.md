# R47 — Boat Wi-Fi OTA (maintenance only, not commissioned)

Selected hardware: ESP32-S3-DevKitC-1-N32R16V. Wi-Fi is integrated; no extra radio board is needed for Wi-Fi OTA. ESP32-S3 supports Bluetooth LE, **not Bluetooth Classic**. Bluetooth OTA is deferred; no BLE GATT transport or secure authorization exists.

## Implemented
- `Boat/source/boat_wifi_ota.h` optional, compile-time OFF by default (`BOAT_WIFI_OTA_COMMISSIONED`).
- HTTPS download using `WiFiClientSecure` with a supplied trusted CA; refuses plain HTTP.
- Fail-closed `BoatOtaPreflight`: local physical maintenance switch, independent motor AND solenoid power isolation, STOP latched, stationary, healthy battery, secure boot, flash encryption, mission/calibration idle, operator confirmation.
- Runtime ESP secure boot / flash encryption checks; 12-second Wi-Fi association timeout; no hard-coded SSID/password/CA; no OTA control from primary/backup radio.
- Portable host regression test.

## Critical remaining implementation/commissioning
1. Provision secure boot V2 and flash encryption securely on the physical ESP32-S3; protect signing keys offline, choose secure OTA partition table and anti-rollback policy. Provisioning fuses is irreversible: verify exact board first.
2. Implement OTA health-confirmation and rollback using ESP-IDF OTA APIs; this module deliberately **does not reboot automatically** and cannot claim rollback or successful activation. Validate new image signature and reboot policy before production.
3. Provide trusted HTTPS release hosting, CA certificate, signed image, verified version/anti-rollback metadata and safe Wi-Fi credentials provisioning. Never place credentials in GitHub.
4. Wire a genuine independent local maintenance switch, motor/coil power isolation contacts and feedback; verify them before passing preflight booleans. Software-only flags are not proof of isolation.
5. Connect an authenticated local maintenance workflow to the module. No radio or web trigger while underway. Do not call install() from the ordinary mission loop.
6. Compile the enabled OTA path for the exact ESP32 Arduino core, test interrupted downloads, power loss, invalid signatures, rollback, failed Wi-Fi, and both radios offline.
7. Only after hardware validation consider optional BLE OTA; Bluetooth Classic is unsupported on ESP32-S3.

**Status:** secure Wi-Fi OTA *component scaffold*, not yet a working end-to-end firmware upgrade. Existing diagnostic firmware and all actuators remain fail-disabled. Boat VER014 release unchanged.
