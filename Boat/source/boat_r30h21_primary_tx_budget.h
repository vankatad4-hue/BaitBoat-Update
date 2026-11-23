#pragma once
// R30H-21: primary nRF ordinary traffic admission before allocating a
// 16-bit authenticated packet sequence number. Pure C++ (no radio/GPIO).
// STOP ACK and fresh STATUS have a separate high-priority scheduler and
// are NEVER admitted through this ordinary-frame policy.
namespace boat_r30h21 {
struct PrimaryTxConditions {
    bool sessionReady=false;
    bool radioReady=false;
    bool txBusy=false;
    bool stopAckPending=false;
    bool statusPending=false;
};
inline bool ordinarySlotFree(const PrimaryTxConditions &s) {
    return s.sessionReady && s.radioReady && !s.txBusy &&
           !s.stopAckPending && !s.statusPending;
}
} // namespace boat_r30h21
