#include <Arduino.h>
#include <lvgl.h>

#include "joystick.h"
#include "joystick_adc.h"
#include "boat_state.h"
#include "boat_ui.h"
#include "boat_commands.h"
#include "boat_radio.h"
#include "boat_telemetry.h"
#include "offline_map.h"
#include "lake_storage.h"
#include "offline_map_coverage.h"
#include "map_manager_ui.h"
#include "display_port.h"
#include "physical_button_scanner.h"
#include "release_button_guard.h"

static bool uiReady = false;

// Physical button events (0..7 left bank, 8..15 right bank).
static void physicalButtonEvent(uint8_t id, bool pressed)
{
    if (!pressed) return;
    switch (id) {
        case 0: boatCommands_sendStop(); boatState.emergencyStop=true; boatState.emergencyStopConfirmed=false; releaseGuard_cancel(); break;
        case 1: if (boatState.mode==MODE_MANUAL) boatCommands_setMode(MODE_AUTOPILOT); else if (boatState.mode==MODE_AUTOPILOT) boatCommands_setMode(MODE_MANUAL); break;
        case 2: boatCommands_setMode(MODE_RETURN_HOME); break;
        case 3: case 4: case 5: case 6: case 7:
        case 12: case 13: case 14: case 15:
            if(id==14)releaseGuard_cancel();
            if(uiReady)boatUI_physicalAction(id);
            break;
        default: break;
    }
}

// --------------------------------------------------
// EMERGENCY STOP HANDLER
// --------------------------------------------------

static void emergencyStop()
{
    // Send STOP. The boat must acknowledge it through valid telemetry.
    const bool sent = boatCommands_sendStop();
    boatState.emergencyStop = true;
    boatState.emergencyStopConfirmed = false;
    releaseGuard_cancel();
    Serial.println(sent ? "STOP command sent." : "STOP command NOT transmitted.");
}

// --------------------------------------------------
// JOYSTICK HARDWARE READER
// --------------------------------------------------

static bool readJoystickHardware(JoystickRaw &raw)
{
    // Read four analog axes through the ADS1115. A failed or stale read
    // returns false so joystick_update() suppresses all motor commands.
    return joystickAdc_read(raw.leftX, raw.leftY, raw.rightX, raw.rightY);
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
    lakeStorage_begin(boatState.sdCardReady);
    boatState.offlineMapReady = offlineMap_begin(boatState.sdCardReady);

    // Dual-radio supervisor is initialised. Hardware transports remain disabled
    // until pin/power design, drivers, and a provisioned authentication key exist.
    boatRadio_begin();

    // Start ADS1115. If missing, the reader fails and joystick output remains inhibited.
    const bool joystickAdcReady = joystickAdc_begin();
    Serial.println(joystickAdcReady ? "JOYSTICK ADC: detected" : "JOYSTICK ADC: unavailable - output inhibited");

    // Initialise joystick processing once and register one reader.
    joystick_begin();
    joystick_setReadCallback(readJoystickHardware);

    // Route application commands through the radio transport.
    boatCommands_setTransport(boatRadio_sendCommand);

    // Connect the main-screen STOP button to the command handler.
    boatUI_setStopCallback(emergencyStop);
    physicalButtons_setCallback(physicalButtonEvent);
    const bool buttonsReady = physicalButtons_begin();
    Serial.println(buttonsReady ? "PHYSICAL BUTTONS: initialized" : "PHYSICAL BUTTONS: unavailable");

    // LVGL objects may only be created after the display runtime is ready.
    if (displayReady)
    {
        createBoatScreen();
        uiReady = true;
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
    telemetryApplyToBoatState(boatState,millis());
    if(boatRadio_stopConfirmationPending()){boatState.emergencyStop=true;boatState.emergencyStopConfirmed=false;boatState.actuatorSafetyReady=false;}

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

    // Poll physical controls with debounce; actuator release remains fail-closed.
    physicalButtons_update(millis());

    // Read sticks and process control only when every safety gate passes.
    joystick_update();

    // Service LVGL from one place only: the display adapter uses loop-based scheduling.
    if (displayPort_isReady())
    {
        // Service LVGL from the display adapter.
        displayPort_loop();
        mapManagerUI_tick();

        // Refresh GPS coordinates and status at a modest UI update rate.
        static uint32_t lastScreenUpdateMs = 0;
        if (static_cast<uint32_t>(millis() - lastScreenUpdateMs) >= 250U)
        {
            lastScreenUpdateMs = millis();
            updateBoatScreen(boatState);
        }
    }
}
