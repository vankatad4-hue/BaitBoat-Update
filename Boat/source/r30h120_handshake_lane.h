#pragma once
#include "r30h118_transport.h"
#include <atomic>
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
}
