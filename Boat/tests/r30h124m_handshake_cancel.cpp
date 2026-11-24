#include "r30h123_owner_link.h"
#include <cassert>
using namespace flytec_r30h118;
using namespace flytec_r30h120;
using namespace flytec_r30h121;
using namespace flytec_r30h123;
struct Frame{uint8_t type=11,sourceId=2,destinationId=1,flags=1,sessionEpoch=0;uint16_t sequence=0,commandId=0;};
int main(){
 Transport<Frame> appBus;PhyEvents<Frame> events;AppRadio<Frame> app(appBus,events);
 HandshakeLane<Frame> lane;SharedPriority priority;OwnerLink<Frame> owner(app,lane,events,priority);
 HandshakeService<Frame> worker(lane);Frame f{};
 auto shape=[](const Frame& v){return boatReplyShape(v,2,1,true);};
 auto verify=[](const Frame&){return true;};
 unsigned admissions=0;
 auto send=[&](const HandshakeLane<Frame>::Request&){++admissions;return true;};
 assert(owner.setSession(1));assert(owner.beginHandshake()&&owner.submitHandshake(f,0,shape,verify));
 HandshakeLane<Frame>::Request dequeued{};assert(lane.take(dequeued));
 owner.cancelHandshake();assert(!lane.allows(dequeued,1,6000,shape));
 assert(lane.complete(dequeued,false));
 assert(events.publishOutcome({dequeued.ticket,1,PhyOutcome::LocalComplete,false}));
 f.type=13;assert(owner.beginHandshake()&&owner.submitHandshake(f,2,shape,verify));
 assert(worker.step(3,6000,true,shape,send)&&admissions==1);
 owner.poll(4,6000,6000);Ticket t{};HandshakeTx state=HandshakeTx::Idle;
 assert(!owner.takeHandshake(t,state)); // stale local completion cannot confirm
 owner.cancelHandshake();assert(!owner.takeHandshake(t,state));
 // Fill queue with canceled work before worker runs. It must drain without RF
 // even after the logical handshake endpoint returns to Idle.
 for(unsigned i=0;i<3;++i){
  assert(owner.beginHandshake()&&owner.submitHandshake(f,10+i,shape,verify));owner.cancelHandshake();
 }
 assert(owner.handshakePending()&&choosePriority(priority.snapshot())==TxPriority::Handshake);
 assert(owner.beginHandshake()&&!owner.submitHandshake(f,14,shape,verify));
 for(unsigned i=0;i<3;++i){
  assert(!worker.step(15+i,6000,true,shape,send));owner.poll(15+i,6000,6000);
 }
 assert(!owner.handshakePending()&&choosePriority(priority.snapshot())==TxPriority::None&&admissions==1);
 assert(owner.beginHandshake()&&owner.submitHandshake(f,20,shape,verify));
 HandshakeLane<Frame>::Request valid{};assert(lane.take(valid)&&lane.allows(valid,21,6000,shape));
 assert(lane.complete(valid,true));assert(events.publishOutcome({valid.ticket,21,PhyOutcome::LocalComplete,false}));
 owner.poll(21,6000,6000);assert(owner.takeHandshake(t,state)&&sameTicket(t,valid.ticket)&&state==HandshakeTx::LocalComplete);
}
