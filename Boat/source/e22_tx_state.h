#pragma once
#include <stdint.h>
// Local UART/module lifecycle only. Never indicates RF delivery or peer ACK.
// Provisional bench timeout, not a measured RF timing guarantee.
namespace flytec_rtos {
class E22TxState {
public:
 enum class Outcome:uint8_t { Idle, Pending, LocalReady, Uncertain };
 static constexpr uint32_t minUartMs=34; // 32 bytes, 9600 baud, 8N1
 static constexpr uint32_t timeoutMs=2000;
 void reset(){outcome_=Outcome::Idle;}
 void accepted(uint32_t now){started_=now;outcome_=Outcome::Pending;}
 void uncertain(){outcome_=Outcome::Uncertain;}
 void poll(uint32_t now,bool aux){
  if(!pending())return;
  const uint32_t elapsed=now-started_;
  if(elapsed>=timeoutMs)uncertain();
  else if(aux&&elapsed>=minUartMs)outcome_=Outcome::LocalReady;
 }
 bool pending()const{return outcome_==Outcome::Pending;}
 bool fault()const{return outcome_==Outcome::Uncertain;}
 Outcome outcome()const{return outcome_;}
private:
 Outcome outcome_=Outcome::Idle;
 uint32_t started_=0;
};
}
