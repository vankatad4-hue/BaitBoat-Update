#include "dual_radio_manager.h"

static constexpr uint32_t PRIMARY_STALE_MS = 250;
static constexpr uint32_t BACKUP_STALE_MS = 3000;
static constexpr uint32_t RECOVERY_HOLDOFF_MS = 1500;
static constexpr uint8_t RECOVERY_GOOD_REQUIRED = 8;

DualRadioManager::DualRadioManager() : active_(LINK_NONE), state_(LINK_DISCONNECTED),
    recoveryStartMs_(0), recoveryStreak_(0), lastCmdId_(0), haveCmdId_(false) {
    memset(&primary_, 0, sizeof(primary_));
    memset(&backup_, 0, sizeof(backup_));
}

void DualRadioManager::begin(uint32_t now) {
    (void)now;
    active_ = LINK_NONE;
    state_ = LINK_DISCONNECTED;
    recoveryStartMs_ = 0;
    recoveryStreak_ = 0;
    haveCmdId_ = false;
}

void DualRadioManager::reportValidRx(ManagedLink link, bool authenticated, uint32_t now) {
    LinkHealth &h = (link == LINK_NRF_PRIMARY) ? primary_ : backup_;
    h.lastValidMs = now;
    h.authenticated = authenticated;
    if (authenticated) {
        if (h.goodStreak < 255) ++h.goodStreak;
        h.failureStreak = 0;
    } else {
        h.goodStreak = 0;
        if (h.failureStreak < 255) ++h.failureStreak;
    }
}

void DualRadioManager::reportFailure(ManagedLink link) {
    LinkHealth &h = (link == LINK_NRF_PRIMARY) ? primary_ : backup_;
    h.authenticated = false;
    h.goodStreak = 0;
    if (h.failureStreak < 255) ++h.failureStreak;
}

void DualRadioManager::setTransportReady(ManagedLink link, bool ready) {
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
    if (p) {
        if (primary_.goodStreak < 255) ++primary_.goodStreak;
    } else primary_.goodStreak = 0;

    switch (state_) {
    case LINK_DISCONNECTED:
    case LINK_NO_LINK:
        if (p) { active_ = LINK_NRF_PRIMARY; state_ = LINK_RECOVERY; recoveryStartMs_=now; recoveryStreak_=1; }
        else if (b) { active_ = LINK_LORA_BACKUP; state_ = LINK_BACKUP; }
        else { active_ = LINK_NONE; state_ = LINK_NO_LINK; }
        break;
    case LINK_PRIMARY:
        if (!p) { state_ = LINK_DEGRADED; active_=LINK_NONE; }
        break;
    case LINK_DEGRADED:
        if (p) { active_=LINK_NRF_PRIMARY; state_=LINK_PRIMARY; }
        else if (b) { active_=LINK_LORA_BACKUP; state_=LINK_BACKUP; }
        else if (!connected(now)) { active_=LINK_NONE; state_=LINK_NO_LINK; }
        break;
    case LINK_BACKUP:
        if (!b && !p) { active_=LINK_NONE; state_=LINK_NO_LINK; }
        else if (p) { state_=LINK_RECOVERY; active_=LINK_NONE; recoveryStartMs_=now; recoveryStreak_=1; }
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
    case CMD_EMERGENCY_STOP:
    case CMD_SET_MODE: return cmd.mode == MODE_RETURN_HOME;
    case CMD_MANUAL_MOTORS: return true;
    default: return false; // no bait/hook release, mission start or clear-stop on backup
    }
}
void DualRadioManager::limitBackupMotorCommand(BoatCommand &cmd) {
    constexpr int16_t LIMIT = 250; // 25% normalized max, provisional
    if (cmd.leftMotor > LIMIT) cmd.leftMotor=LIMIT;
    if (cmd.leftMotor < -LIMIT) cmd.leftMotor=-LIMIT;
    if (cmd.rightMotor > LIMIT) cmd.rightMotor=LIMIT;
    if (cmd.rightMotor < -LIMIT) cmd.rightMotor=-LIMIT;
}
bool DualRadioManager::send(const BoatCommand &cmd, RadioSendFn sendFn) {
    if (!sendFn || active_ == LINK_NONE) return false;
    if (active_ == LINK_LORA_BACKUP) {
        if (!backupCommandAllowed(cmd)) return false;
        BoatCommand safe = cmd;
        if (safe.type == CMD_MANUAL_MOTORS) limitBackupMotorCommand(safe);
        return sendFn(LINK_LORA_BACKUP, safe);
    }
    if (active_ == LINK_NRF_PRIMARY && state_ == LINK_PRIMARY)
        return sendFn(LINK_NRF_PRIMARY, cmd);
    // During degradation/recovery, do not issue normal commands.
    return false;
}
