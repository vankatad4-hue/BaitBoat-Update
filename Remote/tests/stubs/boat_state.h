#pragma once
#include <stdint.h>
enum RadioUiLink:uint8_t{RADIO_UI_NONE=0,RADIO_UI_NRF_PRIMARY,RADIO_UI_LORA_BACKUP};struct BoatState{bool radioConnected;RadioUiLink radioActiveLink;bool emergencyStopConfirmed;bool emergencyStop;bool actuatorSafetyReady;};extern BoatState boatState;
