#ifndef BOAT_RADIO_H
#define BOAT_RADIO_H

#include <Arduino.h>
#include "boat_commands.h"

// Dual-radio architecture. Hardware transports are still non-operational stubs.
enum BoatRadioLink : uint8_t
{
    RADIO_LINK_NONE = 0,
    RADIO_LINK_NRF24_PRIMARY,
    RADIO_LINK_LORA_BACKUP
};

enum BoatRadioState : uint8_t
{
    RADIO_STATE_DISCONNECTED = 0,
    RADIO_STATE_PRIMARY,
    RADIO_STATE_DEGRADED,
    RADIO_STATE_BACKUP,
    RADIO_STATE_NO_LINK,
    RADIO_STATE_RECOVERY
};

// Initialise radio drivers and link supervisor. Returns false until drivers
// and an authenticated boat response are implemented.
bool boatRadio_begin();

// Transmit one application command through LinkManager. Command IDs must be
// preserved across transport retries/failover; action commands are idempotent.
bool boatRadio_sendCommand(const BoatCommand &command);

// Poll both transports without blocking the UI/control loop.
void boatRadio_update();

// Aggregate connection flag: true only after valid response from either link.
bool boatRadio_isConnected();

// Diagnostics/UI API. Stub implementation returns disconnected/NONE.
BoatRadioLink boatRadio_getActiveLink();
BoatRadioState boatRadio_getState();
bool boatRadio_primaryConnected();
bool boatRadio_backupConnected();

#endif
