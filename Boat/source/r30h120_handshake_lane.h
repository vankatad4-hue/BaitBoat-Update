#pragma once
#include "r30h118_transport.h"
namespace flytec_r30h120 {
// Owner-side priority selection: no lower class may bypass pending safety.
// A busy PHY call already underway cannot be preempted.
enum class TxPriority:uint8_t {Stop,Safety,Handshake,Heartbeat,Mission,Telemetry,Diagnostic,None};
inline TxPriority choosePriority(uint8_t pending){
 for(uint8_t i=0;i<7;++i)if(pending&(1U<<i))return static_cast<TxPriority>(i);
 return TxPriority::None;
}
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
 Bus bus_;
public:
 using Request=typename Bus::Request;
 bool begin(){return bus_.invalidate(1);}
 void close(){bus_.invalidate(0);}
 template<class Policy> bool submit(const Frame &f,uint32_t now,Policy permitted){
  if(f.sessionEpoch!=0||f.sequence!=0||f.commandId!=0||!permitted(f))return false;
  return bus_.submit(f,now,false);
 }
 bool take(Request &r){return bus_.take(r);}
 template<class Policy> bool allows(const Request &r,uint32_t now,uint32_t age,Policy permitted)const{
  return r.frame.sessionEpoch==0&&r.frame.sequence==0&&r.frame.commandId==0&&permitted(r.frame)&&bus_.allows(r,now,age);
 }
 bool complete(const Request &r,bool admitted){return bus_.complete(r,admitted);}
 bool poll(bool &admitted){return bus_.poll(admitted);}
};
}
