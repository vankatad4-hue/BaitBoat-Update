#pragma once
#include <Arduino.h>
#include "boat_commands.h"

enum ManagedLink : uint8_t { LINK_NONE=0, LINK_NRF_PRIMARY=1, LINK_LORA_BACKUP=2 };
enum ManagedState : uint8_t { LINK_DISCONNECTED=0, LINK_PRIMARY, LINK_DEGRADED, LINK_BACKUP, LINK_NO_LINK, LINK_RECOVERY };

struct LinkHealth {
    bool authenticated;
    bool transportReady;
    uint32_t lastValidMs;
    uint8_t goodStreak;
    uint8_t failureStreak;
};

typedef bool (*RadioSendFn)(ManagedLink link, const BoatCommand &cmd);

class DualRadioManager {
public:
    DualRadioManager();
    void begin(uint32_t now);
    void reportValidRx(ManagedLink link, bool authenticated, uint32_t now);
    void reportFailure(ManagedLink link);
    void setTransportReady(ManagedLink link, bool ready);
    void tick(uint32_t now);
    bool send(const BoatCommand &cmd, RadioSendFn sendFn);
    ManagedLink active() const { return active_; }
    ManagedState state() const { return state_; }
    bool primaryHealthy(uint32_t now) const;
    bool backupHealthy(uint32_t now) const;
    bool connected(uint32_t now) const;
private:
    LinkHealth primary_, backup_;
    ManagedLink active_;
    ManagedState state_;
    uint32_t recoveryStartMs_;
    uint8_t recoveryStreak_;
    uint16_t lastCmdId_;
    bool haveCmdId_;
    static bool backupCommandAllowed(const BoatCommand &cmd);
    static void limitBackupMotorCommand(BoatCommand &cmd);
};
