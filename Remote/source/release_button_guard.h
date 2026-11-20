#pragma once
#include <stdint.h>
enum ReleaseChannel:uint8_t{RELEASE_HOPPER_LEFT=0,RELEASE_HOPPER_RIGHT=1,RELEASE_HOOK_LEFT=2,RELEASE_HOOK_RIGHT=3};
enum ReleaseGuardState:uint8_t{RELEASE_GUARD_IDLE=0,RELEASE_GUARD_HOLDING_KEY,RELEASE_GUARD_WAIT_CONFIRM,RELEASE_GUARD_HOLDING_CONFIRM};
void releaseGuard_beginKey(ReleaseChannel,uint32_t);void releaseGuard_endKey(uint32_t);void releaseGuard_beginConfirm(uint32_t);void releaseGuard_endConfirm(uint32_t);void releaseGuard_cancel();void releaseGuard_tick(uint32_t);ReleaseGuardState releaseGuard_state();ReleaseChannel releaseGuard_pendingChannel();
