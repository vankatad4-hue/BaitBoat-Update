#include "r30h118_completion_gate.h"
#include <cassert>
#include "r30h118_radio_handoff.h"
#include "r30h118_worker_guard.h"
#include "r30h118_transport.h"
#include "r30h118_worker.h"
#include "r30h118_phy_events.h"
#include "r30h118_phy_pump.h"
#include "r30h118_stop_retry.h"
#include "r30h118_outcome_gate.h"
using namespace flytec_r30h118;
int main(){
 CompletionGate g; Ticket a,b,c;
 assert(!g.issue(a));
 assert(g.invalidate(1));
 assert(g.issue(a)); assert(!g.issue(b));
 assert(g.invalidate(1)); // newer STOP, same session
 assert(g.issue(b)); assert(!g.consume(a));
 assert(g.consume(b)); assert(!g.consume(b));
 assert(g.issue(c)); assert(!g.consume(b));
 assert(g.invalidate(2)); assert(!g.consume(c));
 assert(g.issue(a)); assert(!g.invalidate(0));
 assert(!g.consume(a)); assert(!g.issue(b));
 assert(g.invalidate(1)); assert(g.issue(b));
 assert(!g.consume(a)); assert(g.consume(b));
 RadioHandoff<uint32_t> lane;
 RadioHandoff<uint32_t>::Request old{},fresh{};
 bool admitted=false;
 assert(lane.invalidate(1));
 assert(lane.prepare(42,old));
 assert(lane.invalidate(1));
 assert(lane.prepare(99,fresh));
 assert(!lane.accept({old.ticket,true},admitted));
 assert(!admitted);
 assert(lane.accept({fresh.ticket,true},admitted));
 assert(admitted);
 assert(lane.prepare(7,fresh));
 assert(lane.cancelUnqueued(fresh));
 assert(!lane.accept({fresh.ticket,true},admitted));
 WorkerGuard guard;
 CompletionGate owner; Ticket t;
 assert(owner.invalidate(1));assert(owner.issue(t));
 assert(!guard.allows(t,0,1,100));
 guard.publish(owner.activeGeneration());
 assert(guard.allows(t,0xfffffffeU,1,100));
 assert(!guard.allows(t,0,100,100));
 assert(owner.invalidate(1));guard.publish(owner.activeGeneration());
 assert(!guard.allows(t,0,1,100));
 assert(owner.issue(t));assert(guard.allows(t,0,1,100));
 guard.disable();assert(!guard.allows(t,0,1,100));
 Transport<uint32_t> transport;
 Transport<uint32_t>::Request req{};
 assert(!transport.submit(1,0,false));
 assert(transport.invalidate(1));
 assert(transport.submit(1,0,false));
 assert(transport.invalidate(1));
 assert(transport.submit(2,1,true));
 assert(transport.take(req)&&req.stop&&req.frame==2);
 assert(transport.allows(req,2,100));
 assert(transport.complete(req,true));
 assert(transport.poll(admitted)&&admitted);
 assert(transport.take(req)&&!req.stop);
 assert(!transport.allows(req,2,100));
 assert(transport.complete(req,false));
 assert(!transport.poll(admitted));
 assert(transport.invalidate(0)==false);
 assert(!transport.submit(3,3,true));
 Transport<uint32_t> bus;
 Worker<uint32_t> worker(bus);
 unsigned calls=0;
 auto admit=[&](uint32_t){++calls;return true;};
 assert(bus.invalidate(1));
 assert(bus.submit(1,0,false));
 worker.step(1,100,admit);
 assert(bus.poll(admitted)&&admitted&&calls==1);
 assert(bus.submit(2,0,false));
 worker.step(100,100,admit);
 assert(bus.poll(admitted)&&!admitted&&calls==1);
 assert(bus.submit(3,100,false));
 worker.requestStop();
 worker.step(101,100,admit);
 assert(worker.exited()&&calls==1);
 worker.step(102,100,admit);
 assert(calls==1);
 PhyEvents<uint32_t> primaryEvents,backupEvents;
 PhyEvents<uint32_t>::Rx rx{};
 for(uint32_t i=0;i<4;++i)assert(primaryEvents.publishRx({i,100+i}));
 assert(!primaryEvents.publishRx({4,104}));
 assert(backupEvents.publishRx({99,200}));
 for(uint32_t i=0;i<4;++i){assert(primaryEvents.takeRx(rx));assert(rx.frame==i&&rx.capturedAt==100+i);}
 assert(!primaryEvents.takeRx(rx));
 assert(backupEvents.takeRx(rx)&&rx.frame==99);
 PhyEvents<uint32_t>::Health health{};
 assert(primaryEvents.publishHealth({10,true,false,true,false}));
 assert(primaryEvents.takeHealth(health)&&health.fault&&!health.ready);
 PhyEvents<uint32_t>::Outcome outcome{};
 Ticket eventTicket{7,8,1};
 assert(primaryEvents.publishOutcome({eventTicket,20,PhyOutcome::Uncertain}));
 assert(primaryEvents.takeOutcome(outcome)&&sameTicket(outcome.ticket,eventTicket)&&outcome.result==PhyOutcome::Uncertain);
 PhyEvents<uint32_t> pumpEvents;
 PhyPump<uint32_t> pump(pumpEvents);
 uint32_t nextRx=0;
 auto receive=[&](uint32_t &v){if(nextRx==5)return false;v=nextRx++;return true;};
 pump.pollRx(10,receive);assert(nextRx==4);
 pump.pollRx(11,receive);assert(nextRx==5);
 assert(pumpEvents.takeRx(rx)&&rx.frame==0);
 pump.pollRx(12,receive);
 for(uint32_t i=1;i<5;++i){assert(pumpEvents.takeRx(rx)&&rx.frame==i);if(i==4)assert(rx.capturedAt==11);}
 for(unsigned i=0;i<4;++i)assert(pumpEvents.publishOutcome({eventTicket,1,PhyOutcome::Failed}));
 assert(pump.track(eventTicket));assert(!pump.canAdmit());
 assert(pump.terminal(PhyOutcome::Uncertain,20));
 pump.flushOutcome();assert(!pump.canAdmit());
 assert(pumpEvents.takeOutcome(outcome));pump.flushOutcome();assert(pump.canAdmit());
 for(unsigned i=0;i<4;++i)assert(pumpEvents.takeOutcome(outcome));
 assert(outcome.result==PhyOutcome::Uncertain&&sameTicket(outcome.ticket,eventTicket));
 PhyEvents<uint32_t>::Health fresh{0xfffffffeU,true,true,false,false};
 assert(healthyFresh(fresh,1,10));assert(!healthyFresh(fresh,8,10));
 fresh.fault=true;assert(!healthyFresh(fresh,1,10));
 Transport<uint32_t> retryBus;
 StopRetry<uint32_t> retry;
 assert(retry.arm(retryBus,1,10,0));assert(retry.service(retryBus,0,100));
 assert(retry.arm(retryBus,1,20,1));assert(!retry.service(retryBus,2,100));assert(retry.pending());
 assert(retryBus.take(req)&&req.frame==10&&!retryBus.allows(req,2,100));
 assert(retry.service(retryBus,3,100));assert(retryBus.take(req)&&req.frame==20&&req.queuedAt==1);
 assert(retryBus.allows(req,3,100));
 assert(retry.arm(retryBus,1,30,4));assert(!retry.service(retryBus,104,100));assert(!retry.pending());
 assert(!retryBus.submit(99,105,false));
 assert(retryBus.invalidate(2));assert(!retryBus.submit(99,105,false));
 assert(!retryBus.rearm(0));assert(!retryBus.submit(99,105,false));
 assert(retryBus.rearm(2));assert(retryBus.submit(99,105,false));
 OutcomeGate og;
 assert(og.expect(eventTicket));assert(!og.expect(eventTicket));
 assert(!og.accept({7,9,1},7));assert(!og.accept({7,8,2},7));
 assert(!og.accept(eventTicket,8));assert(og.accept(eventTicket,7));
 assert(!og.accept(eventTicket,7));assert(og.expect(eventTicket));
 og.reset();assert(!og.accept(eventTicket,7));
 PhyEvents<uint32_t> congested,independent;
 for(uint32_t i=0;i<4;++i)assert(congested.publishHealth({i,true,true,false,false}));
 assert(!congested.publishHealth({4,true,false,true,false}));
 assert(congested.stickyFault());assert(!independent.stickyFault());
 assert(congested.latestHealth(health));assert(health.observedAt==3&&health.fault&&!health.ready);
 assert(!healthyFresh(health,4,100));
 assert(congested.publishHealth({5,true,true,false,false}));
 assert(congested.latestHealth(health)&&health.fault&&!health.ready);
 assert(!congested.latestHealth(health)&&health.fault);
}
