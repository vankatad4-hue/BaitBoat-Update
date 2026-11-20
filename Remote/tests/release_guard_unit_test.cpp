#include <cassert>
#include "release_button_guard.h"
#include "boat_state.h"

// boatState is defined by boat_state.cpp, compiled with the same full struct
// definition as release_button_guard.cpp. Avoid a reduced-layout test stub.
static int sent = -1;

bool boatCommands_releaseHopperLeft() { sent = 0; return true; }
bool boatCommands_releaseHopperRight() { sent = 1; return true; }
bool boatCommands_releaseHookLeft() { sent = 2; return true; }
bool boatCommands_releaseHookRight() { sent = 3; return true; }

static void ready()
{
    boatState.radioConnected = true;
    boatState.radioActiveLink = RADIO_UI_NRF_PRIMARY;
    boatState.emergencyStopConfirmed = true;
    boatState.emergencyStop = false;
    boatState.actuatorSafetyReady = true;
}

int main()
{
    initialiseBoatState();
    ready();
    releaseGuard_beginKey(RELEASE_HOPPER_LEFT, 100);
    releaseGuard_endKey(900);
    assert(releaseGuard_state() == RELEASE_GUARD_IDLE && sent == -1);

    releaseGuard_beginKey(RELEASE_HOPPER_RIGHT, 1000);
    releaseGuard_endKey(2300);
    releaseGuard_beginConfirm(2500);
    releaseGuard_endConfirm(3300);
    assert(sent == 1);

    sent = -1;
    releaseGuard_beginKey(RELEASE_HOOK_LEFT, 4000);
    releaseGuard_endKey(5200);
    releaseGuard_tick(9200);
    assert(sent == -1);

    initialiseBoatState();
    ready();
    releaseGuard_beginKey(RELEASE_HOOK_RIGHT, 10000);
    releaseGuard_endKey(11200);
    releaseGuard_beginConfirm(11300);
    boatState.radioActiveLink = RADIO_UI_LORA_BACKUP;
    releaseGuard_endConfirm(12200);
    assert(sent == -1);

    initialiseBoatState();
    ready();
    releaseGuard_beginKey(RELEASE_HOPPER_LEFT, 13000);
    releaseGuard_endKey(14200);
    releaseGuard_beginConfirm(14300);
    releaseGuard_endConfirm(15100);
    assert(sent == 0);
}
