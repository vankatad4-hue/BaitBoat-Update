#pragma once
#include "r30h118_spsc_queue.h"
#include "r30h118_worker_guard.h"
namespace flytec_r30h118 {
// Prototype only: one owner and one worker per instance. No physical driver.
// STOP classification is the trusted application owner's responsibility.
template<class Frame> class Transport {
public:
 struct Request { Ticket ticket; Frame frame; uint32_t queuedAt; bool stop; };
 struct Result { Ticket ticket; bool admitted; };
private:
 CompletionGate owner_;
 bool stopLatched_=false;
 WorkerGuard guard_;
 SpscQueue<Request,2> stop_;
 SpscQueue<Request,4> ordinary_;
 SpscQueue<Result,4> results_;
public:
 explicit Transport(TicketDomain domain=TicketDomain::Application):owner_(domain),guard_(domain){}
 bool invalidate(uint8_t epoch){
  guard_.disable();
  const bool ok=owner_.invalidate(epoch);
  if(ok)guard_.publish(owner_.activeGeneration());
  return ok;
 }
 void latchStop(){stopLatched_=true;}
 // Owner calls only after authenticated application rearm authorization.
 bool rearm(uint8_t epoch){if(!invalidate(epoch))return false;stopLatched_=false;return true;}
 uint32_t activeGeneration()const{return owner_.activeGeneration();}
 bool submit(const Frame &frame,uint32_t now,bool stop,Ticket *submitted=nullptr){
  if(!stop&&stopLatched_)return false;
  Ticket t;
  if(!owner_.issue(t))return false;
  Request r{t,frame,now,stop};
  if(stop?stop_.push(r):ordinary_.push(r)){if(submitted)*submitted=t;return true;}
  owner_.consume(t);return false;
 }
 // Call invalidate before submitting a new STOP to revoke ordinary tickets.
 bool take(Request &r){return stop_.pop(r)||ordinary_.pop(r);}
 bool allows(const Request &r,uint32_t now,uint32_t maxAge)const{
  return guard_.allows(r.ticket,r.queuedAt,now,maxAge);
 }
 // Worker must retain a result if this returns false, before taking more work.
 bool complete(const Request &r,bool admitted){return results_.push({r.ticket,admitted});}
 bool poll(bool &admitted){
  Result r{};
  if(!results_.pop(r)||!owner_.consume(r.ticket))return false;
  admitted=r.admitted;return true;
 }
};
}
