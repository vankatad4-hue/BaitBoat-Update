#pragma once
// Consolidated Boat radio component; original types and owner contracts retained.
#include <atomic>
#include <stddef.h>
#include <type_traits>
namespace flytec_r30h118 {
// Exactly one producer and one consumer. Never reset while either is active.
// Stale entries are drained and rejected by owner-side ticket validation.
template<class T, size_t N> class SpscQueue {
 static_assert(N>=2,"one slot reserved");
 static_assert(std::is_trivially_copyable<T>::value,"copied messages only");
 T slots_[N]{};
 std::atomic<size_t> head_{0},tail_{0};
public:
 bool push(const T &v){
  const size_t h=head_.load(std::memory_order_relaxed), next=(h+1)%N;
  if(next==tail_.load(std::memory_order_acquire))return false;
  slots_[h]=v;head_.store(next,std::memory_order_release);return true;
 }
 // Advisory snapshot; only the consumer may pop.
 bool empty()const{return tail_.load(std::memory_order_acquire)==head_.load(std::memory_order_acquire);}
 bool pop(T &v){
  const size_t t=tail_.load(std::memory_order_relaxed);
  if(t==head_.load(std::memory_order_acquire))return false;
  v=slots_[t];tail_.store((t+1)%N,std::memory_order_release);return true;
 }
};
}

#include <stdint.h>
// Owner-local only. Transfer immutable tickets through synchronized queues.
// This gate never authorizes motion or proves RF delivery.
namespace flytec_r30h118 {
enum class TicketDomain:uint8_t { Application, Handshake };
struct Ticket { uint32_t generation=0; uint32_t serial=0; uint8_t epoch=0; TicketDomain domain=TicketDomain::Application; };
class CompletionGate {
 const TicketDomain domain_;
 uint32_t generation_=0, serial_=0;
 uint8_t epoch_=0;
 bool pending_=false, exhausted_=false;
public:
 explicit CompletionGate(TicketDomain domain=TicketDomain::Application):domain_(domain){}
 bool invalidate(uint8_t epoch){
  pending_=false; epoch_=0;
  if(exhausted_||generation_==UINT32_MAX){exhausted_=true;return false;}
  ++generation_; serial_=0; epoch_=epoch; return epoch!=0;
 }
 uint32_t activeGeneration() const {return epoch_&&!exhausted_?generation_:0;}
 bool issue(Ticket &out){
  if(exhausted_||!epoch_||pending_||serial_==UINT32_MAX)return false;
  ++serial_; pending_=true; out={generation_,serial_,epoch_,domain_};return true;
 }
 bool consume(const Ticket &t){
  if(!pending_||!epoch_||t.domain!=domain_||t.epoch!=epoch_||t.generation!=generation_||t.serial!=serial_)return false;
  pending_=false;return true;
 }
};
}

#include <atomic>
namespace flytec_r30h118 {
// Single application publisher; worker reads only. Zero disables admission.
// A check cannot revoke a driver call already underway.
class WorkerGuard {
 const TicketDomain domain_;
 std::atomic<uint32_t> generation_{0};
public:
 explicit WorkerGuard(TicketDomain domain=TicketDomain::Application):domain_(domain){}
 void disable(){generation_.store(0,std::memory_order_release);}
 void publish(uint32_t generation){generation_.store(generation,std::memory_order_release);}
 bool allows(const Ticket &t,uint32_t queuedAt,uint32_t now,uint32_t maxAge) const {
  const uint32_t active=generation_.load(std::memory_order_acquire);
  return active!=0&&t.domain==domain_&&t.generation==active&&t.epoch!=0&&t.serial!=0&&
         maxAge!=0&&uint32_t(now-queuedAt)<maxAge;
 }
};
}

namespace flytec_r30h118 {
// Prototype only: one owner and one worker per instance. No physical driver.
// STOP classification is the trusted application owner's responsibility.
template<class Frame> class Transport {
public:
 struct Request { Ticket ticket; Frame frame; uint32_t queuedAt; bool stop; uint32_t maxAge=0; };
 struct Result { Ticket ticket; bool admitted; };
private:
 CompletionGate owner_;
 bool stopLatched_=false;
 WorkerGuard guard_;
 SpscQueue<Request,2> stop_;
 SpscQueue<Request,4> ordinary_;
 SpscQueue<Result,4> results_;
 SpscQueue<Result,4> stopResults_;
public:
 explicit Transport(TicketDomain domain=TicketDomain::Application):owner_(domain),guard_(domain){}
 bool invalidate(uint8_t epoch){
  guard_.disable();
  const bool ok=owner_.invalidate(epoch);
  if(ok)guard_.publish(owner_.activeGeneration());
  return ok;
 }
 void latchStop(){stopLatched_=true;}
 bool ordinaryEnabled()const{return !stopLatched_&&owner_.activeGeneration()!=0;}
 bool feedbackEnabled()const{return owner_.activeGeneration()!=0;}
 // Owner calls only after authenticated application rearm authorization.
 bool rearm(uint8_t epoch){if(!invalidate(epoch))return false;stopLatched_=false;return true;}
 uint32_t activeGeneration()const{return owner_.activeGeneration();}
 bool submit(const Frame &frame,uint32_t now,bool stop,Ticket *submitted=nullptr,uint32_t requestAge=0,bool outboundFeedback=false){
  // Only the Boat owner may explicitly classify read-only outbound feedback.
  // This exception never clears the latch or authorizes inbound movement.
  if(!stop&&stopLatched_&&!outboundFeedback)return false;
  Ticket t;
  if(!owner_.issue(t))return false;
  Request r{t,frame,now,stop,requestAge};
  if(stop?stop_.push(r):ordinary_.push(r)){if(submitted)*submitted=t;return true;}
  owner_.consume(t);return false;
 }
 // Call invalidate before submitting a new STOP to revoke ordinary tickets.
 bool takeStop(Request &r){return stop_.pop(r);}
 bool ordinaryQueued()const{return !ordinary_.empty();}
 bool takeOrdinary(Request &r){return ordinary_.pop(r);}
 bool take(Request &r){return takeStop(r)||takeOrdinary(r);}
 bool allows(const Request &r,uint32_t now,uint32_t maxAge)const{
  const uint32_t limit=r.maxAge?r.maxAge:maxAge;
  return guard_.allows(r.ticket,r.queuedAt,now,limit);
 }
 // Worker must retain a result if this returns false, before taking more work.
 bool complete(const Request &r,bool admitted){return r.stop?stopResults_.push({r.ticket,admitted}):results_.push({r.ticket,admitted});}
 bool poll(bool &admitted){
  Result r{};
  if((!stopResults_.pop(r)&&!results_.pop(r))||!owner_.consume(r.ticket))return false;
  admitted=r.admitted;return true;
 }
};
}

namespace flytec_r30h118 {
// One-shot lifecycle: construct before task start, destroy only after join.
// Owner requests shutdown; worker alone services driver and pending result.
template<class Frame> class Worker {
 using Bus=Transport<Frame>;
 Bus &bus_;
 typename Bus::Request pending_{};
 bool hasResult_=false, admitted_=false;
 std::atomic<bool> stopping_{false}, exited_{false};
public:
 explicit Worker(Bus &bus):bus_(bus){}
 // Owner must stop submissions before calling this.
 void requestStop(){bus_.invalidate(0);stopping_.store(true,std::memory_order_release);}
 bool exited()const{return exited_.load(std::memory_order_acquire);}
 template<class Admit> void step(uint32_t now,uint32_t maxAge,Admit admit){
  if(exited())return;
  if(hasResult_){
   if(!bus_.complete(pending_,admitted_))return;
   hasResult_=false;
  }
  if(stopping_.load(std::memory_order_acquire)){
   exited_.store(true,std::memory_order_release);return;
  }
  if(!bus_.take(pending_))return;
  admitted_=false;
  if(!stopping_.load(std::memory_order_acquire)&&bus_.allows(pending_,now,maxAge))
   admitted_=admit(pending_.frame);
  hasResult_=true;
  if(bus_.complete(pending_,admitted_))hasResult_=false;
 }
};
}

namespace flytec_r30h118 {
// Owner only. Admission to queue is not physical STOP confirmation.
template<class Frame> class StopRetry {
 Frame frame_{};
 uint32_t started_=0;
 bool pending_=false;
public:
 bool arm(Transport<Frame>& bus,uint8_t epoch,const Frame &frame,uint32_t now){
  pending_=false;
  bus.latchStop();
  if(!bus.invalidate(epoch))return false;
  frame_=frame;started_=now;pending_=true;return true;
 }
 bool service(Transport<Frame>& bus,uint32_t now,uint32_t maxAge,Ticket *submitted=nullptr){
  if(!pending_)return false;
  if(!maxAge||uint32_t(now-started_)>=maxAge){pending_=false;return false;}
  if(!bus.submit(frame_,started_,true,submitted))return false;
  pending_=false;return true;
 }
 void reset(){pending_=false;}
 bool pending()const{return pending_;}
};
}

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
