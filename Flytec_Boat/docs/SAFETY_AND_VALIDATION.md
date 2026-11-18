# Boat Firmware Safety and Validation Checklist

This checklist is mandatory before connecting actuators or operating the boat.

- [ ] Confirm battery pack voltage range, wiring, polarity, and BMS limits.
- [ ] Measure each motor's running current and establish a safe stall-current test method.
- [ ] Measure each solenoid coil's resistance/current and maximum permitted activation duration.
- [ ] Confirm MOSFET/driver voltage, current, thermal, gate-drive, flyback, and PCB copper requirements.
- [ ] Confirm hardware emergency-stop behavior independent of application software.
- [ ] Define a boat-side watchdog and stale-command timeout.
- [ ] Implement and test the selected nRF24L01+ PA/LNA link on both ends.
- [ ] Verify dedicated 3.3 V PA/LNA supply under transmit peaks and check RF module temperature/current.
- [ ] Verify 2.4 GHz channel, antenna, radiated EIRP and applicable Bulgarian/EU compliance.
- [ ] Define and test packet sequence, application acknowledgements, authentication/replay protection, heartbeat and stale-command timeout.
- [ ] Define radio-loss behavior for manual and autonomous modes.
- [ ] Require fresh GNSS, valid fix quality, valid compass heading, and valid home position before autonomous movement.
- [ ] Define behavior for GNSS/compass failure during a mission.
- [ ] Bench-test logic with motors and solenoids disconnected.
- [ ] Test outputs with current-limited supplies and dummy loads.
- [ ] Verify motor direction and emergency stop with the boat restrained and a safe test setup.
- [ ] Verify solenoid pulse timing and thermal limits with dummy loads first.
- [ ] Complete controlled, low-risk water trials before enabling autonomous missions.

Passing a software compile is not evidence that the electrical design or safety behavior is validated.
