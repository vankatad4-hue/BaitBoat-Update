#pragma once
#include "boat_commands.h"
#include <stdint.h>
// Core 1 only. This is an intent ledger, not movement authority. The caller
// must derive authorization from fresh authenticated primary telemetry and
// independently controlled rearm. No automatic retry or STOP clearance.
namespace flytec_rtos {
struct OrdinaryAuthorization {
 bool primaryFresh=false,sessionReady=false,stopConfirmed=false,stopped=true;
 bool manualConfirmed=false,actuatorReady=false,joystickValid=false;
 bool missionLocked=true,failsafe=true;
};
inline uint32_t ordinaryLifetime(const BoatCommand& c){
 return c.type==CMD_MANUAL_MOTORS?50U:100U;
}
inline bool ordinaryAllowed(const BoatCommand& c,const OrdinaryAuthorization& a){
 if(!a.primaryFresh||!a.sessionReady||!a.stopConfirmed||a.missionLocked||a.failsafe||
    !c.commandId||c.commandId>UINT16_MAX||c.mode!=MODE_MANUAL)return false;
 const bool zero=c.leftMotor==0&&c.rightMotor==0;
 if(c.type==CMD_SET_MODE)return !c.activate&&zero;
 if(a.stopped||!a.manualConfirmed||!a.actuatorReady)return false;
 if(c.type==CMD_MANUAL_MOTORS)return a.joystickValid&&!c.activate&&
  c.leftMotor>=-1000&&c.leftMotor<=1000&&c.rightMotor>=-1000&&c.rightMotor<=1000;
 return c.type>=CMD_RELEASE_HOPPER_LEFT&&c.type<=CMD_RELEASE_HOOK_RIGHT&&c.activate&&zero;
}
class OrdinaryIntent {
public:
 enum class State:uint8_t{Idle,Staged,InFlight,LocalEvidence,Unknown,Canceled};
 void reset(){state_=State::Idle;command_={};ticket_=generation_=0;sequence_=lastId_=0;epoch_=0;}
 State state()const{return state_;}
 bool stage(const BoatCommand& c,const OrdinaryAuthorization& a,uint32_t now){
  if(!ordinaryAllowed(c,a)||uint32_t(now-c.createdAtMs)>=ordinaryLifetime(c)||state_==State::InFlight)return false;
  if(state_==State::Staged&&(command_.type!=CMD_MANUAL_MOTORS||c.type!=CMD_MANUAL_MOTORS))return false;
  const auto id=uint16_t(c.commandId),delta=uint16_t(id-lastId_);
  if(lastId_&&(!delta||delta>=0x8000U))return false;
  command_=c;lastId_=id;state_=State::Staged;return true;
 }
 // False for stale or unauthorized staged work. For in-flight work, caller
 // must revoke the transport ticket before declaring cancellation complete.
 bool stillAllowed(const OrdinaryAuthorization& a,uint32_t now)const{
  return (state_==State::Staged||state_==State::InFlight)&&ordinaryAllowed(command_,a)&&
   uint32_t(now-command_.createdAtMs)<ordinaryLifetime(command_);
 }
 const BoatCommand& command()const{return command_;}
 bool submitted(uint32_t ticket,uint32_t generation,uint16_t sequence,uint8_t epoch){
  if(state_!=State::Staged||!ticket||!generation||!sequence||!epoch)return false;
  ticket_=ticket;generation_=generation;sequence_=sequence;epoch_=epoch;state_=State::InFlight;return true;
 }
 bool complete(uint32_t ticket,uint32_t generation,uint16_t id,uint16_t sequence,uint8_t epoch,
               uint32_t created,uint32_t lifetime,bool localEvidence,uint32_t now){
  if(state_!=State::InFlight||ticket!=ticket_||generation!=generation_||id!=command_.commandId||
     sequence!=sequence_||epoch!=epoch_||created!=command_.createdAtMs||lifetime!=ordinaryLifetime(command_))return false;
  state_=localEvidence&&uint32_t(now-created)<lifetime?State::LocalEvidence:State::Unknown;
  return true; // never peer ACK, never action executed
 }
 bool discardStaged(){if(state_!=State::Staged)return false;state_=State::Canceled;return true;}
 // Call only AFTER transport generation invalidation has succeeded.
 void transportRevoked(){if(state_==State::Staged||state_==State::InFlight)state_=State::Canceled;}
private:
 BoatCommand command_{};State state_=State::Idle;
 uint32_t ticket_=0,generation_=0;uint16_t sequence_=0,lastId_=0;uint8_t epoch_=0;
};
}
