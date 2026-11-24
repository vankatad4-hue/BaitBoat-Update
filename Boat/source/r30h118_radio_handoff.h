#pragma once
#include "r30h118_completion_gate.h"
namespace flytec_r30h118 {
// Owner-local adapter; queue implementation must copy requests/results.
// One instance per PHY lane. No driver or actuator calls.
template<class Frame> class RadioHandoff {
 CompletionGate gate_;
public:
 struct Request { Ticket ticket; Frame frame; };
 struct Completion { Ticket ticket; bool admitted; };
 bool invalidate(uint8_t epoch){return gate_.invalidate(epoch);}
 bool prepare(const Frame &frame,Request &out){
  Ticket t;
  if(!gate_.issue(t))return false;
  out={t,frame};return true;
 }
 // Use only when enqueue definitely failed; never on ambiguous delivery.
 bool cancelUnqueued(const Request &r){return gate_.consume(r.ticket);}
 bool accept(const Completion &c,bool &admitted){
  if(!gate_.consume(c.ticket))return false;
  admitted=c.admitted;return true;
 }
};
}
