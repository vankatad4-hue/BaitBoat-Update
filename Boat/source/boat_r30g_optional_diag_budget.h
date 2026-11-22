#pragma once
#include <stdint.h>

// R30G: strictly best-effort optional diagnostic airtime on primary nRF.
// The normal heartbeat/mission/ACK/control radio stack owns TX priority.
// No allocation, waiting, retransmission, actuator command or second queue.
// Exactly one replaceable pending payload; never replay stale diagnostics.
namespace boat_r30g {
static constexpr uint32_t MAX_PENDING_AGE_MS=500U;
static constexpr uint32_t MIN_DIAGNOSTIC_PERIOD_MS=2000U;
static constexpr uint32_t MIN_IDLE_AFTER_COMMAND_MS=250U;
static constexpr uint32_t HEARTBEAT_INTERVAL_MS=100U;
static constexpr uint32_t HEARTBEAT_GAP_BEGIN_MS=15U;
static constexpr uint32_t HEARTBEAT_GAP_END_MS=40U;

struct Opportunity {
 bool authenticatedSession=false;
 bool primaryFresh=false;
 bool txPending=false;
 bool commandQueued=false;
 bool activeMission=false;
 bool failsafeOrLinkLoss=false;
 bool recentCommand=false;
 uint32_t lastHeartbeatAtMs=0;
 bool haveHeartbeat=false;
};

class OptionalDiagnosticBudget {
public:
 bool offer(const uint8_t bytes[8],uint32_t nowMs) {
  if(!bytes || bytes[0]!=8U || bytes[1]!=1U)return false;
  if(pending_)++replaced_;
  for(unsigned i=0;i<8;i++)payload_[i]=bytes[i];
  offeredAtMs_=nowMs;pending_=true;
  return true;
 }
 // Returns only after a heartbeat has already been serviced this cycle.
 // On no slot: NEVER touch nRF TX. No deferred FIFO or forced retry.
 bool take(const Opportunity &state,uint32_t nowMs,uint8_t out[8]) {
  if(!pending_||!out)return false;
  if((uint32_t)(nowMs-offeredAtMs_)>MAX_PENDING_AGE_MS) {
   discard();return false;
  }
  if(!state.authenticatedSession||!state.primaryFresh||
     state.failsafeOrLinkLoss) {
   discard();return false;
  }
  if(state.txPending||state.commandQueued||state.activeMission||
     state.recentCommand||!state.haveHeartbeat)return false;
  const uint32_t heartbeatAge=(uint32_t)(nowMs-state.lastHeartbeatAtMs);
  if(heartbeatAge<HEARTBEAT_GAP_BEGIN_MS ||
     heartbeatAge>HEARTBEAT_GAP_END_MS)return false;
  if(haveSent_ &&
     (uint32_t)(nowMs-lastSentAtMs_)<MIN_DIAGNOSTIC_PERIOD_MS)return false;
  for(unsigned i=0;i<8;i++)out[i]=payload_[i];
  pending_=false;
  return true;
 }
 void sent(uint32_t nowMs){haveSent_=true;lastSentAtMs_=nowMs;++transmitted_;}
 void rejectedTx(){++discarded_;}
 void discard(){if(pending_){pending_=false;++discarded_;}}
 void reset(){pending_=false;haveSent_=false;offeredAtMs_=lastSentAtMs_=0;}
 bool pending()const{return pending_;}
 uint32_t transmitted()const{return transmitted_;}
 uint32_t discarded()const{return discarded_;}
 uint32_t replaced()const{return replaced_;}
private:
 uint8_t payload_[8]={};
 uint32_t offeredAtMs_=0,lastSentAtMs_=0;
 uint32_t transmitted_=0,discarded_=0,replaced_=0;
 bool pending_=false,haveSent_=false;
};
} // namespace boat_r30g
