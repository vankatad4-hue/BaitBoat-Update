#pragma once
#include <stdint.h>
// Application-owner scope. Local transport STOP stays latched. Only a
// separately validated preparation handoff and peer CLEAR evidence may
// open the mission-specific path. Reset only with generation revocation.
namespace flytec_rtos {
class MissionTxScope {
public:
 enum class State:uint8_t{Idle,Preparing,HandedOff,ClearQueued,Authorized,Canceling,BackupOnly};
 State state()const{return state_;}
 bool locked()const{return state_!=State::Idle;}
 uint16_t transaction()const{return transaction_;}
 bool permitsPrepare(uint16_t id,bool begin,bool cancel)const{
  if(!id)return false;
  if(state_==State::Idle)return begin&&!cancel;
  if(id!=transaction_)return false;
  if(cancel)return state_==State::Preparing||state_==State::HandedOff||state_==State::Canceling;
  return state_==State::Preparing;
 }
 void preparationQueued(uint16_t id,bool commit,bool cancel){
  transaction_=id;
  if(cancel){state_=State::Canceling;commitQueued_=false;return;}
  state_=State::Preparing;commitQueued_=commitQueued_||commit;
 }
 bool handoff(uint16_t id,bool independentlyConfirmed,bool primaryIdle){
  if(state_!=State::Preparing||!commitQueued_||id!=transaction_||!independentlyConfirmed||!primaryIdle)return false;
  state_=State::HandedOff;return true;
 }
 bool permitsClear(uint16_t id,bool approved)const{
  return state_==State::HandedOff&&id==transaction_&&approved;
 }
 void clearQueued(uint16_t command){clearId_=command;state_=State::ClearQueued;}
 bool confirmClear(uint16_t id,uint16_t command,bool peerEvidence,bool primaryIdle){
  if(state_!=State::ClearQueued||id!=transaction_||!command||command!=clearId_||!peerEvidence||!primaryIdle)return false;
  state_=State::Authorized;return true;
 }
 bool permitsMission(uint16_t id,bool approved)const{return state_==State::Authorized&&id==transaction_&&approved;}
 // Transfer only from a previously independently authorized primary mission.
 // Called by the transport AFTER successful old-generation revocation.
 bool openBackupAfterRevocation(uint16_t id){
  if(state_!=State::Idle||!id)return false;
  transaction_=id;state_=State::BackupOnly;return true;
 }
 bool permitsBackup(uint16_t id,bool approved)const{return state_==State::BackupOnly&&id==transaction_&&approved;}
 void resetAfterRevocation(){*this=MissionTxScope{};}
private:
 State state_=State::Idle;uint16_t transaction_=0,clearId_=0;bool commitQueued_=false;
};
}
