#pragma once
#include <stdint.h>
// R30H-96: portable supervisor, owned exclusively by application task.
// A fault is sticky until explicit reset while outputs are safe.
// Heartbeat measures completed polling, not RF connectivity.
namespace flytec_rtos {
class RadioWatchdog {
public:
 enum class State { Starting, Healthy, Fault };
 explicit RadioWatchdog(uint32_t timeoutMs=500): timeout_(timeoutMs?timeoutMs:1){}
 void reset(uint32_t now,uint32_t heartbeat){
  last_=now; seen_=heartbeat; state_=State::Starting;
 }
 State update(uint32_t now,uint32_t heartbeat,bool running,bool wrongCore){
  if(state_==State::Fault)return state_;
  if(!running || wrongCore || uint32_t(now-last_)>=timeout_){
   state_=State::Fault;return state_;
  }
  if(heartbeat!=seen_){seen_=heartbeat;last_=now;state_=State::Healthy;}
  return state_;
 }
 bool permit() const{return state_==State::Healthy;}
private:
 uint32_t timeout_,last_=0,seen_=0;
 State state_=State::Fault;
};
}
