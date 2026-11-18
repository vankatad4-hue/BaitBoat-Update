
#include "boat_radio.h"

// Current radio connection state.
static bool radioConnected = false;

// --------------------------------------------------
// INITIALISE RADIO
// --------------------------------------------------

bool boatRadio_begin()
{
    // nRF24L01+ PA/LNA hardware initialisation will go here.
    // Selected architecture: matching 2.4 GHz modules at remote and boat.
    // Start range testing at 250 kbps; increase only after measured validation.
    //
    // We still need to confirm:
    // - Exact nRF24L01+ PA/LNA board variant and RF front-end.
    // - SPI pins.
    // - NSS / CS pin.
    // - BUSY pin.
    // - RESET pin.
    // - DIO1 interrupt pin.
    //
    // Do not report a connection until the boat responds.

    radioConnected = false;

    return false;
}

// --------------------------------------------------
// TRANSMIT COMMAND
// --------------------------------------------------

bool boatRadio_sendCommand(const BoatCommand &command)
{
    // Radio packet encoding and transmission
    // will be implemented after hardware selection.

    (void)command;

    return false;
}

// --------------------------------------------------
// PROCESS RECEIVED DATA
// --------------------------------------------------

void boatRadio_update()
{
    // Receive packets from the boat.
    // Validate packet integrity.
    // Process acknowledgements.
    // Update boat telemetry.
}

// --------------------------------------------------
// CONNECTION STATUS
// --------------------------------------------------

bool boatRadio_isConnected()
{
    return radioConnected;
}
