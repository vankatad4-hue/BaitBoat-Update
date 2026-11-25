#pragma once
#include "mission_control_wire.h"
#include "lora_link_policy.h"
#include <stdint.h>
namespace flytec_rtos {
// Dormant backup-only request ledger. No TX ownership or implicit failover.
// Caller must revoke the old generation, preserve the primary ID high-water,
// authenticate/replay-check RX, and share the physical 3-second cooldown
// with STOP/heartbeat. No moving command or automatic mission recovery.
struct BackupMissionAuthority {
 bool session=false,primaryUnavailable=false,backupFresh=false,knownMission=false;
 bool batteryOk=false,safetyReturn=false,stopPending=false;
};
class BackupMissionControl {
public:
 enum class State:uint8_t{Unbound,Idle,Pending,Accepted,Rejected,Unknown};
 static constexpr uint32_t Deadline=12000U,Gap=LoRaAirtimeProfile::BaselineGap,TxLifetime=250U;
 // Admission remains 250 ms. Allow the E22's 2 s local TX watchdog plus
 // 100 ms polling/dispatch margin after the latest possible admission.
 // This is a provisional software observation budget, not RF proof.
 static constexpr uint32_t CompletionGrace=2100U;
 explicit BackupMissionControl(LoRaOwnerTiming timing={}):timing_(timing){}
 State state()const{return state_;}
 uint16_t transaction()const{return transaction_;}
 uint16_t requestHighWater()const{return high_;}
 bool inFlight()const{return ticket_!=0;}
 bool bind(uint16_t transaction,uint8_t epoch,uint32_t generation,uint16_t highWater,bool primaryRevoked){
  if(state_!=State::Unbound||!transaction||!epoch||!generation||!primaryRevoked)return false;
  transaction_=transaction;epoch_=epoch;generation_=generation;high_=highWater;state_=State::Idle;return true;
 }
 bool request(MissionControlOp op,uint32_t now,const BackupMissionAuthority& a){
  if((state_!=State::Idle&&state_!=State::Accepted&&state_!=State::Rejected&&state_!=State::Unknown)||
     inFlight()||high_==UINT16_MAX||!safe(a)||(op!=MCTRL_PAUSE&&op!=MCTRL_CANCEL)||
     (state_==State::Unknown&&op!=MCTRL_CANCEL))return false;
  request_={op,transaction_,++high_};started_=now;attempts_=0;state_=State::Pending;return true;
 }
 bool maintain(uint32_t now,const BackupMissionAuthority& a){
  if(state_==State::Pending&&(!safe(a)||uint32_t(now-started_)>=Deadline||
      (attempts_>=3&&!inFlight()&&uint32_t(now-offeredAt_)>=Gap)))state_=State::Unknown;
  // Local reservation expiry cannot erase an already authenticated peer result.
  // Keep the ticket until correlated completion/revocation; it still blocks new work.
  if(state_==State::Pending&&inFlight()&&uint32_t(now-offeredAt_)>=ttl_+CompletionGrace)state_=State::Unknown;
  return state_!=State::Unknown;
 }
 uint32_t lifetime(uint32_t now)const{
  const auto age=uint32_t(now-started_);if(state_!=State::Pending||age>=Deadline)return 0;
  const auto remaining=Deadline-age;return remaining<TxLifetime?remaining:TxLifetime;
 }
 bool due(uint32_t now,const BackupMissionAuthority& a,bool sharedSlotAvailable,MissionControlRequest& out){
  if(!maintain(now,a)||state_!=State::Pending||!sharedSlotAvailable||inFlight()||attempts_>=3||
     (everOffered_&&uint32_t(now-offeredAt_)<timing_.urgentGap()))return false;
  out=request_;return true;
 }
 bool queued(uint32_t ticket,uint32_t generation,uint16_t sequence,uint8_t epoch,uint32_t now,uint32_t ttl){
  if(state_!=State::Pending||inFlight()||!ticket||!sequence||generation!=generation_||epoch!=epoch_||
     !ttl||ttl!=lifetime(now)||attempts_>=3||(everOffered_&&uint32_t(now-offeredAt_)<timing_.urgentGap()))return false;
  ticket_=ticket;sequence_=sequence;offeredAt_=now;ttl_=ttl;everOffered_=true;++attempts_;return true;
 }
 bool complete(uint32_t ticket,uint32_t generation,uint16_t sequence,uint8_t epoch,uint16_t transaction,uint32_t created,uint32_t ttl){
  if(!inFlight()||ticket!=ticket_||generation!=generation_||sequence!=sequence_||epoch!=epoch_||
     transaction!=transaction_||created!=offeredAt_||ttl!=ttl_)return false;
  ticket_=0;return true; // never confirms Boat execution
 }
 bool observe(const MissionControlFeedback& f,uint8_t epoch,uint32_t generation,uint32_t captured,uint32_t now,
              bool authenticatedBackup,const BackupMissionAuthority& a){
  if(!maintain(now,a)||!authenticatedBackup||state_!=State::Pending||!attempts_||
     epoch!=epoch_||generation!=generation_||!missionFeedbackValid(f)||f.transaction!=transaction_||
     f.result==MC_STATUS||f.request!=request_.request||f.op!=request_.op||uint32_t(now-captured)>=100U||
     uint32_t(captured-started_)>=Deadline||uint32_t(captured-offeredAt_)>=Deadline)return false;
  const auto expected=request_.op==MCTRL_PAUSE?MC_PAUSED:MC_ABORTED;
  if(f.result==MC_ACCEPTED&&f.phase!=expected){state_=State::Unknown;return false;}
  state_=f.result==MC_ACCEPTED?State::Accepted:State::Rejected;return true;
 }
 void transportRevoked(){ticket_=0;if(state_!=State::Unbound)state_=State::Unknown;}
 void resetAfterSession(){*this=BackupMissionControl{timing_};}
private:
 static bool safe(const BackupMissionAuthority& a){return a.session&&a.primaryUnavailable&&a.backupFresh&&a.knownMission&&a.batteryOk&&!a.safetyReturn&&!a.stopPending;}
 LoRaOwnerTiming timing_;
 State state_=State::Unbound;MissionControlRequest request_{};
 uint16_t transaction_=0,high_=0,sequence_=0;uint8_t epoch_=0,attempts_=0;
 uint32_t generation_=0,ticket_=0,started_=0,offeredAt_=0,ttl_=0;bool everOffered_=false;
};
}
