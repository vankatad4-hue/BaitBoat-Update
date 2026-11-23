#pragma once
#include "mission_control_wire.h"
// One in-flight logical operation. Transport retries use fresh frame sequence
// numbers but retain the request ID. Missing ACK means unknown, never success.
class MissionControlClient{
public:
 void reset(){*this=MissionControlClient{};}
 void bind(uint16_t tx){reset();transaction_=tx;}
 bool request(MissionControlOp op,uint32_t now){
  if(!transaction_||pending_||nextId_==65535||op<MCTRL_START||op>MCTRL_SWITCH_MANUAL)return false;
  if(unknown_&&op!=MCTRL_CANCEL)return false;
  request_={op,transaction_,++nextId_};pending_=true;tries_=0;started_=now;lastTx_=now;return true;
 }
 // A transport may refuse admission while its RF slot is busy. Merely
 // inspecting a due request must not consume a retry or move its timestamp.
 bool due(uint32_t now,MissionControlRequest &out,bool backup=false){
  if(!pending_)return false;
  const uint32_t retryMs=backup?3000U:400U;
  const uint32_t timeoutMs=backup?12000U:1800U;
  if((uint32_t)(now-started_)>=timeoutMs||
     (tries_>=3&&(uint32_t)(now-lastTx_)>=retryMs)){
   pending_=false;unknown_=true;return false;
  }
  if(tries_&&(uint32_t)(now-lastTx_)<retryMs)return false;
  out=request_;return true;
 }
 void admitted(uint32_t now){if(pending_){++tries_;lastTx_=now;}}
 // Legacy host simulation interface; runtime uses due/admitted separately.
 bool next(uint32_t now,MissionControlRequest &out,bool backup=false){
  if(!due(now,out,backup))return false;
  admitted(now);return true;
 }
 bool observe(const MissionControlFeedback &f,uint32_t now,bool authenticated){
  if(!authenticated||!missionFeedbackValid(f)||!transaction_||f.transaction!=transaction_)return false;
  if(f.result!=MC_STATUS){
   if(!pending_||f.request!=request_.request||f.op!=request_.op)return false;
   pending_=false;unknown_=false;lastResult_=f.result;
  }
  phase_=f.phase;seen_=true;at_=now;return true;
 }
 bool fresh(uint32_t now)const{return seen_&&now-at_<1000U;}
 bool pending()const{return pending_;}
 bool unknown()const{return unknown_;}
 MissionControlResult result()const{return lastResult_;}
 MissionControlPhase phase()const{return phase_;}
 uint16_t transaction()const{return transaction_;}
 bool locked()const{return transaction_&&(pending_||unknown_||!seen_||(phase_!=MC_IDLE&&phase_!=MC_COMPLETE&&phase_!=MC_ABORTED&&phase_!=MC_MANUAL_REQUESTED));}
private:
 uint16_t transaction_=0,nextId_=0;MissionControlRequest request_{};
 MissionControlPhase phase_=MC_IDLE;MissionControlResult lastResult_=MC_STATUS;
 uint32_t started_=0,lastTx_=0,at_=0;uint8_t tries_=0;bool pending_=false,unknown_=false,seen_=false;
};
