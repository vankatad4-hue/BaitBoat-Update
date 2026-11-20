#include "boat_radio.h"

// Safe placeholder state. No hardware driver, packet decoder, authentication,
// ACK processing, or failover supervisor is implemented in this scaffold.
static BoatRadioLink activeLink = RADIO_LINK_NONE;
static BoatRadioState linkState = RADIO_STATE_DISCONNECTED;
static bool nrfConnected = false;
static bool loraConnected = false;

bool boatRadio_begin()
{
    // TODO: initialise nRF24 primary transport.
    // TODO: initialise EU868 LoRa UART backup transport.
    // TODO: start non-blocking LinkManager and authenticated heartbeat.
    // Keep both link flags false until valid boat-side response is decoded.
    activeLink = RADIO_LINK_NONE;
    linkState = RADIO_STATE_DISCONNECTED;
    nrfConnected = false;
    loraConnected = false;
    return false;
}

bool boatRadio_sendCommand(const BoatCommand &command)
{
    // TODO: route through LinkManager:
    // - primary nRF first;
    // - backup LoRa only for policy-approved commands;
    // - never resend a release action with a new command ID;
    // - TX success is not delivery; wait for validated boat ACK/state.
    (void)command;
    return false;
}

void boatRadio_update()
{
    // TODO: poll both drivers; validate ID/sequence/CRC/authentication;
    // update link metrics; expire stale packets; handle failover/failback.
    // Must remain non-blocking and never wait for a radio ACK in the UI loop.
}

bool boatRadio_isConnected()
{
    return nrfConnected || loraConnected;
}

BoatRadioLink boatRadio_getActiveLink()
{
    return activeLink;
}

BoatRadioState boatRadio_getState()
{
    return linkState;
}

bool boatRadio_primaryConnected()
{
    return nrfConnected;
}

bool boatRadio_backupConnected()
{
    return loraConnected;
}
