# Sonar power control and ping alert

## Button behavior

The screen button is named **Sonar Off / Sonar On**. Its current handler only
pauses Remote-side profile processing and clears the local plot; it does not
sleep the Sonar ESP32. Keep the status text explicit until an end-to-end state
acknowledgement exists.

## Sonar sleep/wake path

The current data UART is one-way: Sonar ESP32 GPIO22 TX → Boat GPIO21 RX. A
Remote button cannot send a sleep request over that link. The intended software
path is an authenticated, idempotent `SONAR_POWER` command from Remote to Boat,
then a framed request over a second UART direction, with the Sonar board
returning its actual state over the existing data TX line.

ESP32 UART wake is supported for **light sleep**, not deep sleep. The sender
must transmit wake edges first because the wake-triggering UART character is
lost. The Sonar runtime should stop the capture cycle, DMA/ADC, and TUSS4470
activity before sleeping, then reinitialize them after an acknowledged wake.
Those peripherals are not active in the current disarmed firmware. Light sleep
does not disconnect shield/transducer supply; cutting all sonar-rail power
requires a load switch or a verified sensor standby mode and power measurement.

Required before the button can operate the Sonar board:

1. Assign and physically verify a Boat UART TX and Sonar UART RX, common ground,
   connector pins, and 3.3 V logic levels. Existing candidate maps are unsigned.
2. Add the authenticated command to both Remote and Boat protocol paths, with
   duplicate suppression, timeout handling, and ACK only after the Sonar board
   confirms its new state.
3. Implement UART wake/light-sleep in Sonar and connect shutdown/restart hooks
   for each commissioned acquisition peripheral.
4. Verify current draw in active, sleep, and wake states with the actual shield
   connected. Do not show “Off” as confirmed until the Sonar ACK arrives.

## Generated ping output

The selected Waveshare ESP32-S3-Touch-LCD-4.3 Standard has no documented
onboard speaker/audio codec. The planned carrier has an optional `BUZZER_CTRL`
connector, but no MCU pin is assigned to it. GPIO6 is already the SD library's
dummy-CS pin in this firmware and is not a free buzzer output.

A **passive piezo** can produce a short synthesized sonar chirp from a PWM
output; an active buzzer generally gives only its built-in pitch, switched on
and off. Use a transistor driver if the selected piezo's current or voltage is
outside the MCU pin rating. This output would synthesize a tone; it would not
play the linked Pixabay MP3. Trigger only for a debounced strong echo
candidate—current profiles do not classify fish or provide echo bearing.

The Pixabay sound is a reference for the desired sound, not firmware audio:
[Sonar ping sound effect](https://pixabay.com/sound-effects/film-special-effects-sonar-89448/).
