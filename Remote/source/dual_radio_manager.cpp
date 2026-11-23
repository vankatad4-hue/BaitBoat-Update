#include "dual_radio_manager.h"
#include <cstring>

static constexpr uint32_t PRIMARY_STALE_MS = 250;
static constexpr uint32_t BACKUP_STALE_MS = 6000;
static constexpr uint32_t RECOVERY_HOLDOFF_MS = 1500;
static constexpr uint8_t RECOVERY_GOOD_REQUIRED = 8;

DualRadioManager::DualRadioManager() : active_(LINK_NONE), state_(LINK_DISCONNECTED),
    recoveryStartMs_(0), recoveryStreak_(0) {
    memset(&primary_, 0, sizeof(primary_));
    memset(&backup_, 0, sizeof(backup_));
}

void DualRadioManager::begin(uint32_t now) {
    active_ = LINK_NONE;
    state_ = LINK_DISCONNECTED;
    recoveryStartMs_ = now;
    recoveryStreak_ = 0;
    memset(&primary_, 0, sizeof(primary_));
    memset(&backup_, 0, sizeof(backup_));
}

void DualRadioManager::reportValidRx(ManagedLink link, bool authenticated, uint32_t now) {
    if (link != LINK_NRF_PRIMARY && link != LINK_LORA_BACKUP) return;
    LinkHealth &h = (link == LINK_NRF_PRIMARY) ? primary_ : backup_;
    h.lastValidMs = now;
    h.authenticated = authenticated;
    if (authenticated) { h.failureStreak = 0; if (h.goodStreak < 255) ++h.goodStreak; }
    else { h.goodStreak = 0; if (h.failureStreak < 255) ++h.failureStreak; }
}

void DualRadioManager::reportFailure(ManagedLink link) {
    if (link != LINK_NRF_PRIMARY && link != LINK_LORA_BACKUP) return;
    LinkHealth &h = (link == LINK_NRF_PRIMARY) ? primary_ : backup_;
    h.authenticated = false;
    h.goodStreak = 0;
    if (h.failureStreak < 255) ++h.failureStreak;
}
void DualRadioManager::setTransportReady(ManagedLink link, bool ready) {
    if (link != LINK_NRF_PRIMARY && link != LINK_LORA_BACKUP) return;
    (link == LINK_NRF_PRIMARY ? primary_ : backup_).transportReady = ready;
}
bool DualRadioManager::primaryHealthy(uint32_t now) const {
    return primary_.transportReady && primary_.authenticated &&
        (uint32_t)(now - primary_.lastValidMs) <= PRIMARY_STALE_MS;
}
bool DualRadioManager::backupHealthy(uint32_t now) const {
    return backup_.transportReady && backup_.authenticated &&
        (uint32_t)(now - backup_.lastValidMs) <= BACKUP_STALE_MS;
}
bool DualRadioManager::connected(uint32_t now) const { return primaryHealthy(now) || backupHealthy(now); }

void DualRadioManager::tick(uint32_t now) {
    const bool p = primaryHealthy(now), b = backupHealthy(now);
    if (!p) primary_.goodStreak = 0;

    switch (state_) {
    case LINK_DISCONNECTED:
    case LINK_NO_LINK:
        if (p) { active_=LINK_NONE; state_=LINK_RECOVERY; recoveryStartMs_=now; recoveryStreak_=0; }
        else if (b) { active_=LINK_LORA_BACKUP; state_=LINK_BACKUP; }
        else { active_=LINK_NONE; state_=LINK_NO_LINK; }
        break;
    case LINK_PRIMARY:
        if (!p) { active_=LINK_NONE; state_=LINK_DEGRADED; }
        break;
    case LINK_DEGRADED:
        if (p) { active_=LINK_NONE; state_=LINK_RECOVERY; recoveryStartMs_=now; recoveryStreak_=0; }
        else if (b) { active_=LINK_LORA_BACKUP; state_=LINK_BACKUP; }
        else if (!connected(now)) { active_=LINK_NONE; state_=LINK_NO_LINK; }
        break;
    case LINK_BACKUP:
        if (!b && !p) { active_=LINK_NONE; state_=LINK_NO_LINK; }
        else if (p) { active_=LINK_NONE; state_=LINK_RECOVERY; recoveryStartMs_=now; recoveryStreak_=0; }
        break;
    case LINK_RECOVERY:
        if (!p) {
            if (b) { active_=LINK_LORA_BACKUP; state_=LINK_BACKUP; }
            else { active_=LINK_NONE; state_=LINK_NO_LINK; }
        } else {
            if (primary_.goodStreak >= RECOVERY_GOOD_REQUIRED &&
                (uint32_t)(now-recoveryStartMs_) >= RECOVERY_HOLDOFF_MS) {
                active_=LINK_NRF_PRIMARY; state_=LINK_PRIMARY;
            }
        }
        break;
    }
}

bool DualRadioManager::backupCommandAllowed(const BoatCommand &cmd) {
    switch (cmd.type) {
    case CMD_EMERGENCY_STOP: return true;
    case CMD_SET_MODE: return cmd.mode == MODE_RETURN_HOME || cmd.mode == MODE_AUTOPILOT;
    case CMD_MANUAL_MOTORS: return false; // 3s LoRa pacing cannot meet 200ms motor lease
    case CMD_CLEAR_STOP: return false; // cannot safely clear STOP without fresh control lease
    case CMD_RELEASE_HOPPER_LEFT: case CMD_RELEASE_HOPPER_RIGHT:
    case CMD_RELEASE_HOOK_LEFT: case CMD_RELEASE_HOOK_RIGHT:
        return true; // Boat must still enforce all release interlocks
    default: return false; // legacy combined release and unknown commands
    }
}
bool DualRadioManager::send(const BoatCommand &cmd, RadioSendFn sendFn, uint32_t now) {
    if (!sendFn || active_ == LINK_NONE) return false;
    // Polling may lag a failed physical link. Never use a cached active_
    // selection as permission to send after authenticated RX has expired.
    if (active_ == LINK_NRF_PRIMARY && !primaryHealthy(now)) return false;
    if (active_ == LINK_LORA_BACKUP && !backupHealthy(now)) return false;
    if (active_ == LINK_LORA_BACKUP && state_ == LINK_BACKUP) {
        if (!backupCommandAllowed(cmd)) return false;
        return sendFn(LINK_LORA_BACKUP, cmd);
    }
    if (active_ == LINK_NRF_PRIMARY && state_ == LINK_PRIMARY)
        return sendFn(LINK_NRF_PRIMARY, cmd);
    return false; // no normal control during degraded/recovery transitions
}
