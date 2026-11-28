# Sonar power control and ping alert

## Power command and button

The Remote **Sonar Off / Sonar On** button now sends authenticated command 11
over the primary radio link. Boat forwards a compact, checksummed request to
the Sonar ESP32 and returns the Remote command result only after the Sonar ACK
matches the command ID and requested state. Backup-radio routing is rejected.
Missing UART configuration, an unsigned pin audit, timeout, or a mismatched ACK
leaves the UI unconfirmed and the Sonar power action rejected.

The Sonar firmware has an opt-in UART control compile profile and uses ESP32
UART-triggered **light sleep**. The sender supplies wake edges before the
framed request because the wake-triggering UART bytes are not retained. Wake
and sleep code is compile-tested by the dedicated profile only when a target
build is run; runtime UART enable remains 0.

This is a software sleep request, not a rail disconnect. The TUSS4470 shield
and transducer supply remain powered in light sleep. The acquisition driver is
still disarmed, so there are currently no ADC/DMA or TUSS activity hooks to
stop or restart. Cutting all sonar hardware power requires a verified standby
control or a properly rated load switch and current measurements.

Boat GPIO21 RX and GPIO47 TX, and Sonar GPIO17 RX/GPIO22 TX, are provisional.
All physical UART audit flags remain unsigned. Check the exact board modules,
header routes, common ground, signal voltage and level shifting before enabling
the UART runtime. GPIO47 on the selected S3 module may require a level shifter.
No board was uploaded or connected.

## Generated ping output

Remote now detects the same strong local echo peaks that produce yellow
range-only pings on the top-view circle. When the saved **Fish Alarm** setting
is enabled, a short synthesized PWM tone is requested for an echo candidate;
this is not fish classification and the down-looking single beam has no target
bearing. No MP3 is stored or played; the linked Pixabay effect is a sound
reference.

Tone generation is implemented behind `REMOTE_SONAR_BUZZER_ENABLED`, which is
0, and `REMOTE_SONAR_BUZZER_PIN`, which is -1. The planned carrier's R-J6
`BUZZER_CTRL` is a logical connector only; it has no assigned ESP32 GPIO yet.
Thus there is no audible output on the current firmware. A passive piezo can
play the generated chirp after a PWM GPIO and suitable driver are assigned and
verified. An active buzzer generally only switches its fixed pitch.

## Remaining acceptance

1. Audit and sign the Boat/Sonar UART directions, signal levels and physical
   routes; then enable UART runtime and verify authenticated Remote → Boat →
   Sonar sleep/wake plus matching ACKs.
2. Decide the TUSS4470 and transducer standby or rail-switch circuit, add the
   required stop/restart hooks to the real acquisition driver, and measure
   active/sleep/wake current with the shield connected.
3. Assign R-J6 to a verified PWM-capable Remote GPIO with an appropriate
   driver, enable the audio gate, and confirm the saved fish-alarm setting
   produces one debounced tone per strong echo candidate.
4. Build all affected PlatformIO targets and run the exact-commit host/CI
   checks. These results are not claimed until observed and recorded.
