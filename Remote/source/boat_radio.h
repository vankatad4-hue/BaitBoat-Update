#ifndef BOAT_RADIO_H
#define BOAT_RADIO_H

#include <Arduino.h>
#include "boat_commands.h"

// Initialise the selected radio hardware. Returns false until hardware and
// a verified boat link are implemented.
bool boatRadio_begin();

// Transmit one application command.
bool boatRadio_sendCommand(const BoatCommand &command);

// Process received packets, acknowledgements and telemetry.
void boatRadio_update();

// True only after a valid boat response confirms the radio link.
bool boatRadio_isConnected();

#endif
