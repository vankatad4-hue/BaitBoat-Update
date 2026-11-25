#pragma once
// Consolidated Boat radio component; original types and owner contracts retained.
#include "boat_radio_transport.h"
#include <atomic>
#include "boat_radio_phy.h"
namespace flytec_r30h120 {
// Owner-side priority selection: no lower class may bypass pending safety.
// A busy PHY call already underway cannot be preempted.
enum class TxPriority:uint8_t {Stop,Safety,Handshake,Heartbeat,Mission,Telemetry,Diagnostic,None};
inline TxPriority choosePriority(uint8_t pending){
 for(uint8_t i=0;i<7;++i)if(pending&(1U<<i))return static_cast<TxPriority>(i);
 return TxPriority::None;
}
// Owner-side arbitration only. Pending STOP blocks every lower class even
// when STOP cannot currently be admitted. Each PHY has its own arbiter.
template<class Service> bool servicePriority(uint8_t pending,bool phyAvailable,Service service){
 const TxPriority selected=choosePriority(pending);
 if(!phyAvailable||selected==TxPriority::None)return false;
 return service(selected); // never fall through on failed admission
}
// Worker-side admission selection for one PHY. The owner publishes the
// pending classes; callbacks must revalidate tickets immediately before TX.
// This does not interrupt an already admitted physical transmission.
template<class App,class Handshake>
bool serviceSharedPhy(uint8_t pending,bool busy,bool enabled,App app,Handshake handshake){
 if(!enabled||busy)return false;
 return servicePriority(pending,true,[&](TxPriority selected){
  return selected==TxPriority::Handshake?handshake():app(selected);
 });
}
// One instance per PHY. Single application publisher, worker snapshot reader.
// A snapshot is advisory: worker must still validate the actual queued ticket.
class SharedPriority {
 std::atomic<uint8_t> pending_{0};
public:
 void publish(uint8_t mask){pending_.store(mask&0x7fU,std::memory_order_release);}
 uint8_t snapshot()const{return pending_.load(std::memory_order_acquire);}
 template<class App,class Handshake>
 bool service(bool busy,bool enabled,App app,Handshake handshake)const{
  return serviceSharedPhy(snapshot(),busy,enabled,app,handshake);
 }
};
// Wire message IDs from radio_protocol.h: CHALLENGE=11, CONFIRM=13.
// Authentication callback is owner-local; the worker checks immutable shape only.
template<class Frame> bool boatReplyShape(const Frame &f,uint8_t boat,uint8_t remote,bool backup){
 return (f.type==11U||f.type==13U)&&f.sourceId==boat&&f.destinationId==remote&&f.flags==(backup?1U:0U)&&f.sessionEpoch==0&&f.sequence==0&&f.commandId==0;
}
// Separate lane per PHY. Internal ticket epoch 1 is NOT a wire session epoch.
// Owner must verify root-key authentication and protocol direction before submit.
// Worker must arbitrate this lane with STOP priority and exclusive driver ownership.
template<class Frame> class HandshakeLane {
 using Bus=flytec_r30h118::Transport<Frame>;
 Bus bus_{flytec_r30h118::TicketDomain::Handshake};
public:
 using Request=typename Bus::Request;
 bool begin(){return bus_.invalidate(1);}
 void close(){bus_.invalidate(0);}
 template<class Policy> bool submit(const Frame &f,uint32_t now,Policy permitted,flytec_r30h118::Ticket *ticket=nullptr){
  if(f.sessionEpoch!=0||f.sequence!=0||f.commandId!=0||!permitted(f))return false;
  return bus_.submit(f,now,false,ticket);
 }
 // Application session rollover deliberately does not close this lane.
 uint32_t activeGeneration()const{return bus_.activeGeneration();}
 bool queued()const{return bus_.ordinaryQueued();}
 template<class Policy,class Verify> bool submitAuthenticated(const Frame &f,uint32_t now,Policy permitted,Verify verify,flytec_r30h118::Ticket *ticket=nullptr){
  return submit(f,now,[&](const Frame &candidate){return permitted(candidate)&&verify(candidate);},ticket);
 }
 bool take(Request &r){return bus_.take(r);}
 template<class Policy> bool allows(const Request &r,uint32_t now,uint32_t age,Policy permitted)const{
  return r.frame.sessionEpoch==0&&r.frame.sequence==0&&r.frame.commandId==0&&permitted(r.frame)&&bus_.allows(r,now,age);
 }
 bool complete(const Request &r,bool admitted){return bus_.complete(r,admitted);}
 bool poll(bool &admitted){return bus_.poll(admitted);}
};

// Worker-local handshake admission; retain completion on queue saturation.
// A selected but invalid request is rejected, never sent or replaced by lower priority.
template<class Frame> class HandshakeService {
 using Lane=HandshakeLane<Frame>;
 Lane &lane_;
 typename Lane::Request request_{};
 bool held_=false,admitted_=false;
public:
 explicit HandshakeService(Lane &lane):lane_(lane){}
 bool flush(){
  if(held_&&lane_.complete(request_,admitted_))held_=false;
  return !held_;
 }
 template<class Policy,class Admit> bool step(uint32_t now,uint32_t age,bool enabled,Policy permitted,Admit admit){
  if(!flush()||!enabled||!lane_.take(request_))return false;
  admitted_=lane_.allows(request_,now,age,permitted)&&admit(request_);
  held_=!lane_.complete(request_,admitted_);
  return admitted_;
 }
 bool resultPending()const{return held_;}
};

// Owner-local consumer, one outstanding handshake per PHY. Local completion
// is not peer authentication or proof of session establishment.
enum class HandshakeTx:uint8_t {Idle,Pending,LocalComplete,Failed,Uncertain};
template<class Frame> class HandshakeEndpoint {
 using Events=flytec_r30h118::PhyEvents<Frame>;
 using Ticket=flytec_r30h118::Ticket;
 HandshakeLane<Frame>& lane_; Events& events_;
 Ticket ticket_{}; uint32_t started_=0;
 bool admission_=false,outcome_=false;
 flytec_r30h118::PhyOutcome result_=flytec_r30h118::PhyOutcome::Uncertain;
 HandshakeTx state_=HandshakeTx::Idle;
public:
 HandshakeEndpoint(HandshakeLane<Frame>& lane,Events& events):lane_(lane),events_(events){}
 bool begin(){if(state_==HandshakeTx::Pending)return false;state_=HandshakeTx::Idle;ticket_={};return lane_.begin();}
 void close(){lane_.close();if(state_==HandshakeTx::Pending)state_=HandshakeTx::Uncertain;}
 // Explicit transcript cancellation: retire the ticket, discard any local
 // result and let the worker drain stale immutable entries by generation.
 void cancel(){if(lane_.activeGeneration())lane_.close();ticket_={};state_=HandshakeTx::Idle;admission_=outcome_=false;}
 bool needsService()const{return state_==HandshakeTx::Pending||lane_.queued();}
 template<class Policy,class Verify> bool submit(const Frame& f,uint32_t now,Policy policy,Verify verify){
  if(state_!=HandshakeTx::Idle)return false;
  if(!lane_.submitAuthenticated(f,now,policy,verify,&ticket_))return false;
  started_=now;admission_=outcome_=false;state_=HandshakeTx::Pending;return true;
 }
 void poll(uint32_t now,uint32_t timeout){
  bool admitted=false;
  for(unsigned i=0;i<3;++i)if(lane_.poll(admitted)&&state_==HandshakeTx::Pending){
   if(!admitted)state_=HandshakeTx::Uncertain;else admission_=true;
  }
  typename Events::Outcome event{};
  for(unsigned i=0;i<4&&events_.takeHandshakeOutcome(event);++i)
   if(state_==HandshakeTx::Pending&&flytec_r30h118::sameTicket(ticket_,event.ticket)){outcome_=true;result_=event.result;}
  if(state_!=HandshakeTx::Pending)return;
  if(ticket_.generation!=lane_.activeGeneration()||!timeout||uint32_t(now-started_)>=timeout){close();return;}
  if(admission_&&outcome_)state_=result_==flytec_r30h118::PhyOutcome::LocalComplete?HandshakeTx::LocalComplete:result_==flytec_r30h118::PhyOutcome::Failed?HandshakeTx::Failed:HandshakeTx::Uncertain;
 }
 HandshakeTx state()const{return state_;}
 bool takeTerminal(Ticket& ticket,HandshakeTx& state){
  if(state_==HandshakeTx::Idle||state_==HandshakeTx::Pending)return false;
  ticket=ticket_;state=state_;state_=HandshakeTx::Idle;return true;
 }
};
}

#include "boat_radio_transport.h"
namespace flytec_r30h120 {
// Worker-local state. Owner never invokes step or physical callbacks.
template<class Frame> class PhyService {
 using Bus=flytec_r30h118::Transport<Frame>;
 Bus &bus_;
 typename Bus::Request request_{};
 bool held_=false,admitted_=false;
 typename Bus::Request stopRequest_{};
 bool stopHeld_=false,stopAdmitted_=false;
public:
 explicit PhyService(Bus &bus):bus_(bus){}
 bool flush(){if(held_&&bus_.complete(request_,admitted_))held_=false;return !held_;}
 bool flushStop(){if(stopHeld_&&bus_.complete(stopRequest_,stopAdmitted_))stopHeld_=false;return !stopHeld_;}
 template<class Admit> void step(uint32_t now,uint32_t maxAge,bool enabled,Admit admit){
  if(!flush())return;
  if(!enabled||!bus_.take(request_))return;
  admitted_=bus_.allows(request_,now,maxAge)&&admit(request_);
  held_=!bus_.complete(request_,admitted_);
 }
 // Shared arbitration must never dequeue STOP through the ordinary lane.
 template<class Admit> void stepOrdinary(uint32_t now,uint32_t age,bool enabled,Admit admit){
  if(!flush()||!enabled||!bus_.takeOrdinary(request_))return;
  admitted_=bus_.allows(request_,now,age)&&admit(request_);
  held_=!bus_.complete(request_,admitted_);
 }
 template<class Admit> void stepStop(uint32_t now,uint32_t age,Admit admit){
  if(!flushStop()||!bus_.takeStop(stopRequest_))return;
  stopAdmitted_=bus_.allows(stopRequest_,now,age)&&admit(stopRequest_);
  stopHeld_=!bus_.complete(stopRequest_,stopAdmitted_);
 }
 bool resultPending()const{return held_||stopHeld_;}
};

// One worker-local arbiter per physical link. Both admission callbacks must
// target the same exclusive adapter. Terminal outcome routing is separate.
template<class Frame> class SharedPhyService {
 PhyService<Frame> app_;
 HandshakeService<Frame> handshake_;
 const SharedPriority &priority_;
public:
 SharedPhyService(flytec_r30h118::Transport<Frame>& app,HandshakeLane<Frame>& hs,const SharedPriority &priority):app_(app),handshake_(hs),priority_(priority){}
 template<class Policy,class AppAdmit,class HsAdmit>
 void step(uint32_t now,uint32_t age,bool busy,bool enabled,Policy policy,AppAdmit appAdmit,HsAdmit hsAdmit){
  const bool stopClear=app_.flushStop();
  const bool appClear=app_.flush();
  const bool hsClear=handshake_.flush();
  // A held handshake admission result must not block the independent STOP lane.
  if(!stopClear)return;
  if(!appClear&&choosePriority(priority_.snapshot())!=TxPriority::Stop)return;
  if(!hsClear&&choosePriority(priority_.snapshot())!=TxPriority::Stop)return;
  priority_.service(busy,enabled,[&](TxPriority selected){
   if(selected==TxPriority::Stop){app_.stepStop(now,age,[&](const typename flytec_r30h118::Transport<Frame>::Request &r){return appAdmit(r);});return true;}
   if(!appClear||!hsClear)return false;
   app_.stepOrdinary(now,age,true,[&](const typename flytec_r30h118::Transport<Frame>::Request &r){
    // A stale advisory STOP mask must never admit an ordinary request.
    return (selected==TxPriority::Stop)==r.stop&&
     choosePriority(priority_.snapshot())==selected&&appAdmit(r);
   });
   return true;
  },[&](){if(!appClear||!hsClear)return false;
  return handshake_.step(now,age,true,policy,[&](const typename HandshakeLane<Frame>::Request &r){
   return choosePriority(priority_.snapshot())==TxPriority::Handshake&&hsAdmit(r);
  });});
 }
};
}

#include <atomic>
#include <stdint.h>
namespace flytec_r30h120 {
enum class Phase:uint8_t {Idle,Starting,Running,Stopping,Stopped,Failed};
// Owner requests start/stop; worker alone initializes and quiesces drivers.
// Failed/Stopped are terminal. No fallback driver access before worker join.
class Lifecycle {
 std::atomic<Phase> phase_{Phase::Idle};
public:
 Phase phase()const{return phase_.load(std::memory_order_acquire);}
 bool start(){Phase p=Phase::Idle;return phase_.compare_exchange_strong(p,Phase::Starting);}
 bool ready(){Phase p=Phase::Starting;return phase_.compare_exchange_strong(p,Phase::Running);}
 void stop(){
  Phase p=phase();
  while(p==Phase::Starting||p==Phase::Running){
   if(phase_.compare_exchange_weak(p,Phase::Stopping))return;
  }
 }
 bool enabled()const{return phase()==Phase::Running;}
 // Only worker after quiescence, or owner if task creation failed.
 bool finish(bool quiesced){
  Phase p=phase();
  while(p==Phase::Starting||p==Phase::Stopping||p==Phase::Running){
   if(phase_.compare_exchange_weak(p,quiesced&&p==Phase::Stopping?Phase::Stopped:Phase::Failed))return true;
  }
  return false;
 }
};
}
