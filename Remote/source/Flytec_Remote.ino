#include <Arduino.h>
#include <lvgl.h>

#include "joystick.h"
#include "boat_state.h"
#include "boat_ui.h"
#include "boat_commands.h"
#include "boat_radio.h"
#include "boat_telemetry.h"
#include "offline_map.h"
#include "display_port.h"

// --------------------------------------------------
// EMERGENCY STOP HANDLER
// --------------------------------------------------

static void emergencyStop()
{
    // Send STOP. The boat must acknowledge it through valid telemetry.
    const bool sent = boatCommands_sendStop();
    Serial.println(sent ? "STOP command sent." : "STOP command NOT transmitted.");
}

// --------------------------------------------------
// JOYSTICK HARDWARE READER PLACEHOLDER
// --------------------------------------------------

static bool readJoystickHardware(JoystickRaw &raw)
{
    // GPIO assignments are intentionally unconfigured.
    // Returning false inhibits all joystick motor commands.
    (void)raw;
    return false;
}

// --------------------------------------------------
// ARDUINO SETUP
// --------------------------------------------------

void setup()
{
    Serial.begin(115200);

    // Initialise shared state in a safe, STOP-latched condition.
    initialiseBoatState();
    telemetryInit();

    // Initialise the display port before creating any LVGL objects.
    // The adapter initialises the Waveshare board and registers LVGL 9 display/touch.
    const bool displayReady = displayPort_begin();

    // The display adapter mounts the TF card using the board-owned CH422G expander.
    // A missing card does not prevent the remote UI from starting.
    boatState.sdCardReady = displayReady && displayPort_sdReady();
    boatState.offlineMapReady = offlineMap_begin(boatState.sdCardReady);

    // Dual-radio supervisor is initialised. Hardware transports remain disabled
    // until pin/power design, drivers, and a provisioned authentication key exist.
    boatRadio_begin();

    // Initialise joystick processing once and register one reader.
    joystick_begin();
    joystick_setReadCallback(readJoystickHardware);
    joystick_setLayout(CONTROL_TANK);
    joystick_setAutopilotOverride(OVERRIDE_TEMPORARY);

    // Route application commands through the radio transport.
    boatCommands_setTransport(boatRadio_sendCommand);

    // Connect the main-screen STOP button to the command handler.
    boatUI_setStopCallback(emergencyStop);

    // LVGL objects may only be created after the display runtime is ready.
    if (displayReady)
    {
        createBoatScreen();
        Serial.println("DISPLAY: LVGL UI created.");
    }
    else
    {
        Serial.println("DISPLAY: UI creation skipped safely.");
    }

    Serial.println("Flytec Remote started in safe placeholder mode.");
}

// --------------------------------------------------
// ARDUINO MAIN LOOP
// --------------------------------------------------

void loop()
{
    // Process incoming radio packets and boat acknowledgements.
    boatRadio_update();

    // Apply the latest telemetry; GPS fixes automatically expire after 2 seconds.
    telemetryApplyToBoatState(boatState);

    // Publish radio supervisor status to the UI.
    boatState.radioConnected = boatRadio_isConnected();
    boatState.radioPrimaryConnected = boatRadio_primaryConnected();
    boatState.radioBackupConnected = boatRadio_backupConnected();
    switch (boatRadio_getActiveLink())
    {
        case RADIO_LINK_NRF24_PRIMARY: boatState.radioActiveLink = RADIO_UI_NRF_PRIMARY; break;
        case RADIO_LINK_LORA_BACKUP: boatState.radioActiveLink = RADIO_UI_LORA_BACKUP; break;
        default: boatState.radioActiveLink = RADIO_UI_NONE; break;
    }
    switch (boatRadio_getState())
    {
        case RADIO_STATE_PRIMARY: boatState.radioLinkState = RADIO_UI_PRIMARY; break;
        case RADIO_STATE_DEGRADED: boatState.radioLinkState = RADIO_UI_DEGRADED; break;
        case RADIO_STATE_BACKUP: boatState.radioLinkState = RADIO_UI_BACKUP; break;
        case RADIO_STATE_NO_LINK: boatState.radioLinkState = RADIO_UI_NO_LINK; break;
        case RADIO_STATE_RECOVERY: boatState.radioLinkState = RADIO_UI_RECOVERY; break;
        default: boatState.radioLinkState = RADIO_UI_DISCONNECTED; break;
    }

    // Read sticks and process control only when every safety gate passes.
    joystick_update();

    // Service LVGL from one place only: the display adapter uses loop-based scheduling.
    if (displayPort_isReady())
    {
        // Service LVGL from the display adapter.
        displayPort_loop();

        // Refresh GPS coordinates and status at a modest UI update rate.
        static uint32_t lastScreenUpdateMs = 0;
        if (static_cast<uint32_t>(millis() - lastScreenUpdateMs) >= 250U)
        {
            lastScreenUpdateMs = millis();
            updateBoatScreen(boatState);
        }
    }
}
