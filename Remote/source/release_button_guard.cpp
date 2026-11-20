#include "release_button_guard.h"
#include "boat_state.h"
#include "boat_commands.h"
static constexpr uint32_t KEY_HOLD=1200,OK_HOLD=800,WINDOW=4000;static ReleaseGuardState st=RELEASE_GUARD_IDLE;static ReleaseChannel ch=RELEASE_HOPPER_LEFT;static uint32_t keyAt=0,okAt=0,deadline=0;
static bool ready(){return boatState.radioConnected&&boatState.radioActiveLink==RADIO_UI_NRF_PRIMARY&&boatState.emergencyStopConfirmed&&!boatState.emergencyStop&&boatState.actuatorSafetyReady;}
static bool send(){if(!ready())return false;switch(ch){case RELEASE_HOPPER_LEFT:return boatCommands_releaseHopperLeft();case RELEASE_HOPPER_RIGHT:return boatCommands_releaseHopperRight();case RELEASE_HOOK_LEFT:return boatCommands_releaseHookLeft();case RELEASE_HOOK_RIGHT:return boatCommands_releaseHookRight();default:return false;}}
void releaseGuard_beginKey(ReleaseChannel c,uint32_t n){if(st!=RELEASE_GUARD_IDLE||!ready()||c>RELEASE_HOOK_RIGHT){releaseGuard_cancel();return;}ch=c;keyAt=n;st=RELEASE_GUARD_HOLDING_KEY;}
void releaseGuard_endKey(uint32_t n){if(st!=RELEASE_GUARD_HOLDING_KEY)return;if(n-keyAt<KEY_HOLD||!ready()){releaseGuard_cancel();return;}deadline=n+WINDOW;st=RELEASE_GUARD_WAIT_CONFIRM;}
void releaseGuard_beginConfirm(uint32_t n){if(st!=RELEASE_GUARD_WAIT_CONFIRM||(int32_t)(deadline-n)<=0||!ready()){releaseGuard_cancel();return;}okAt=n;st=RELEASE_GUARD_HOLDING_CONFIRM;}
void releaseGuard_endConfirm(uint32_t n){if(st!=RELEASE_GUARD_HOLDING_CONFIRM)return;if(n-okAt>=OK_HOLD&&(int32_t)(deadline-n)>0&&ready())(void)send();releaseGuard_cancel();}
void releaseGuard_cancel(){st=RELEASE_GUARD_IDLE;keyAt=okAt=deadline=0;}void releaseGuard_tick(uint32_t n){if(st==RELEASE_GUARD_WAIT_CONFIRM&&((int32_t)(deadline-n)<=0||!ready()))releaseGuard_cancel();if((st==RELEASE_GUARD_HOLDING_KEY||st==RELEASE_GUARD_HOLDING_CONFIRM)&&!ready())releaseGuard_cancel();}ReleaseGuardState releaseGuard_state(){return st;}ReleaseChannel releaseGuard_pendingChannel(){return ch;}
