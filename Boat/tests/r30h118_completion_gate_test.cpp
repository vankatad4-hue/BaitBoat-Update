#include "../source/r30h121_app_radio.h"
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
#include "r30h120_phy_service.h"
#include "r30h120_lifecycle.h"
#include "r30h120_handshake_lane.h"
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
 PhyEvents<uint32_t>::Health freshHealth{0xfffffffeU,true,true,false,false};
 assert(healthyFresh(freshHealth,1,10));assert(!healthyFresh(freshHealth,8,10));
 freshHealth.fault=true;assert(!healthyFresh(freshHealth,1,10));
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
 Transport<uint32_t> serviceBus;
 flytec_r30h120::PhyService<uint32_t> service(serviceBus);
 unsigned serviceCalls=0;
 auto serviceAdmit=[&](const Transport<uint32_t>::Request &r){++serviceCalls;return r.frame==42;};
 assert(serviceBus.invalidate(1));
 assert(serviceBus.submit(42,0,false));
 service.step(1,100,false,serviceAdmit);assert(serviceCalls==0);
 service.step(1,100,true,serviceAdmit);assert(serviceCalls==1);
 assert(serviceBus.poll(admitted)&&admitted);
 for(unsigned i=0;i<3;++i){
  assert(serviceBus.invalidate(1));assert(serviceBus.submit(42,2,false));
  service.step(3,100,true,serviceAdmit);
 }
 assert(serviceBus.invalidate(1));assert(serviceBus.submit(42,2,false));
 service.step(3,100,true,serviceAdmit);assert(service.resultPending());
 service.step(3,100,false,serviceAdmit);assert(service.resultPending());
 assert(!serviceBus.poll(admitted));
 service.step(3,100,false,serviceAdmit);assert(!service.resultPending());
 assert(serviceCalls==5);
 using flytec_r30h120::Lifecycle;
 using flytec_r30h120::Phase;
 Lifecycle life;
 assert(!life.enabled());assert(life.start());assert(!life.start());
 assert(life.ready()&&life.enabled());life.stop();assert(!life.enabled());
 assert(life.finish(true)&&life.phase()==Phase::Stopped);assert(!life.start());
 Lifecycle early;assert(early.start());early.stop();assert(!early.ready());
 assert(early.finish(true)&&early.phase()==Phase::Stopped);
 Lifecycle failed;assert(failed.start());assert(failed.finish(false));
 assert(failed.phase()==Phase::Failed&&!failed.start());
 Lifecycle unsafe;assert(unsafe.start());assert(unsafe.ready());unsafe.stop();
 assert(unsafe.finish(false)&&unsafe.phase()==Phase::Failed);
 using namespace flytec_r30h120;
 for(unsigned mask=0;mask<256;++mask){
  unsigned expected=0;while(expected<7&&!(mask&(1U<<expected)))++expected;
  assert(static_cast<unsigned>(choosePriority(static_cast<uint8_t>(mask)))==expected);
 }
 struct HandshakeFrame {uint8_t type,sourceId,destinationId,flags,sessionEpoch;uint16_t sequence,commandId;};
 HandshakeLane<HandshakeFrame> handshakeLane;
 HandshakeFrame hf{11,1,2,0,0,0,0};
 auto policy=[](const HandshakeFrame &f){return boatReplyShape(f,1,2,false);};
 assert(!handshakeLane.submit(hf,0,policy));assert(handshakeLane.begin());
 for(unsigned type=0;type<17;++type){hf.type=type;assert(policy(hf)==(type==11||type==13));}
 hf.type=11;hf.flags=1;assert(!handshakeLane.submit(hf,0,policy));hf.flags=0;
 hf.commandId=1;assert(!handshakeLane.submit(hf,0,policy));hf.commandId=0;
 hf.sessionEpoch=1;assert(!handshakeLane.submit(hf,0,policy));hf.sessionEpoch=0;
 assert(handshakeLane.submit(hf,10,policy));
 HandshakeLane<HandshakeFrame>::Request hr{};
 assert(handshakeLane.take(hr));assert(handshakeLane.allows(hr,11,100,policy));
 assert(!handshakeLane.allows(hr,110,100,policy));
 handshakeLane.close();assert(!handshakeLane.allows(hr,11,100,policy));
 assert(handshakeLane.complete(hr,true));bool handshakeAdmitted=false;
 assert(!handshakeLane.poll(handshakeAdmitted));assert(handshakeLane.begin());
 assert(handshakeLane.submit(hf,12,policy));assert(handshakeLane.take(hr));
 assert(handshakeLane.allows(hr,13,100,policy));assert(handshakeLane.complete(hr,true));
 assert(handshakeLane.poll(handshakeAdmitted)&&handshakeAdmitted);

 // Identical numeric tickets must never cross domains.
 Transport<uint32_t> appDomain, hsDomain(TicketDomain::Handshake);
 assert(appDomain.invalidate(1)&&hsDomain.invalidate(1));
 Ticket appTicket{},hsTicket{};
 assert(appDomain.submit(1,10,false,&appTicket));
 assert(hsDomain.submit(2,10,false,&hsTicket));
 assert(appTicket.generation==hsTicket.generation&&appTicket.serial==hsTicket.serial&&appTicket.epoch==hsTicket.epoch);
 assert(!sameTicket(appTicket,hsTicket));
 Transport<uint32_t>::Request ar{},hreq{};
 assert(appDomain.take(ar)&&hsDomain.take(hreq));
 assert(appDomain.allows(ar,11,100)&&hsDomain.allows(hreq,11,100));
 assert(!appDomain.allows(hreq,11,100)&&!hsDomain.allows(ar,11,100));
 assert(appDomain.complete(hreq,true));assert(!appDomain.poll(admitted));
 assert(hsDomain.complete(ar,true));assert(!hsDomain.poll(admitted));
 OutcomeGate domainOutcome;
 assert(domainOutcome.expect(appTicket));
 assert(!domainOutcome.accept(hsTicket,appTicket.generation));
 assert(domainOutcome.accept(appTicket,appTicket.generation));
 assert(appDomain.complete(ar,true)&&appDomain.poll(admitted)&&admitted);
 assert(hsDomain.complete(hreq,true)&&hsDomain.poll(admitted)&&admitted);
 assert(hsDomain.invalidate(0));assert(hsDomain.invalidate(1));
 assert(hsDomain.submit(3,12,false));assert(!hsDomain.allows(hreq,13,100));
 assert(hsDomain.complete(hreq,true));assert(!hsDomain.poll(admitted));
 assert(hsDomain.take(hreq)&&hsDomain.allows(hreq,13,100));
 assert(hsDomain.complete(hreq,true)&&hsDomain.poll(admitted)&&admitted);

 using flytec_r30h121::AppRadio;
 using flytec_r30h121::TxState;
 Transport<uint32_t> asyncBus;
 PhyEvents<uint32_t> asyncEvents;
 AppRadio<uint32_t> endpoint(asyncBus,asyncEvents);
 assert(endpoint.reset(1));
 assert(asyncEvents.publishHealth({10,true,true,false,false}));
 endpoint.poll();assert(endpoint.healthy(11,10));assert(!endpoint.healthy(20,10));
 assert(endpoint.submit(42,10,false));assert(endpoint.state()==TxState::Queued);
 assert(!endpoint.submit(43,11,false));
 Transport<uint32_t>::Request asyncReq{};
 assert(asyncBus.take(asyncReq));
 assert(asyncEvents.publishOutcome({asyncReq.ticket,12,PhyOutcome::LocalComplete}));
 endpoint.poll();assert(endpoint.state()==TxState::Queued);
 assert(asyncBus.complete(asyncReq,true));endpoint.poll();
 assert(endpoint.state()==TxState::LocalComplete);
 assert(endpoint.submit(43,20,false));assert(asyncBus.take(asyncReq));
 assert(asyncBus.complete(asyncReq,true));endpoint.poll();assert(endpoint.state()==TxState::Admitted);
 assert(!endpoint.expire(29,10));assert(endpoint.expire(30,10));
 assert(endpoint.state()==TxState::Uncertain&&!asyncBus.allows(asyncReq,30,100));
 assert(endpoint.reset(2));assert(endpoint.submit(44,31,false));
 assert(endpoint.preemptStop(99,32,2));
 assert(asyncBus.take(asyncReq)&&asyncReq.stop&&asyncReq.frame==99);
 assert(asyncBus.complete(asyncReq,true));
 assert(asyncEvents.publishOutcome({asyncReq.ticket,33,PhyOutcome::LocalComplete}));
 endpoint.poll();assert(endpoint.state()==TxState::LocalComplete);
 assert(!endpoint.submit(45,34,false));
 assert(asyncBus.take(asyncReq)&&!asyncBus.allows(asyncReq,34,100));
 assert(asyncEvents.publishHealth({34,true,false,true,false}));
 endpoint.poll();assert(!endpoint.healthy(34,100));

 Transport<uint32_t> saturatedStopBus;
 PhyEvents<uint32_t> saturatedStopEvents;
 AppRadio<uint32_t> stopEndpoint(saturatedStopBus,saturatedStopEvents);
 assert(stopEndpoint.reset(1));
 assert(stopEndpoint.preemptStop(100,10,1));
 assert(!stopEndpoint.preemptStop(101,11,1));
 assert(stopEndpoint.stopRetryPending());
 assert(!stopEndpoint.rearm(1,true));
 assert(saturatedStopBus.take(asyncReq)&&asyncReq.frame==100);
 assert(!saturatedStopBus.allows(asyncReq,12,100));
 assert(stopEndpoint.serviceStop(12,100));
 assert(!stopEndpoint.stopRetryPending());
 assert(saturatedStopBus.take(asyncReq)&&asyncReq.frame==101&&asyncReq.queuedAt==11);
 assert(saturatedStopBus.allows(asyncReq,12,100));
 assert(saturatedStopBus.complete(asyncReq,true));
 assert(saturatedStopEvents.publishOutcome({asyncReq.ticket,13,PhyOutcome::LocalComplete}));
 stopEndpoint.poll();assert(stopEndpoint.state()==TxState::LocalComplete);
 assert(!stopEndpoint.rearm(1,false));assert(!stopEndpoint.submit(9,14,false));
 assert(stopEndpoint.rearm(1,true));assert(stopEndpoint.submit(9,14,false));

 // Strict arbitration: failed highest-priority admission never falls through.
 for(unsigned mask=0;mask<256;++mask){
  unsigned calls=0;
  auto reject=[&](TxPriority p){++calls;assert(p==choosePriority(static_cast<uint8_t>(mask)));return false;};
  assert(!servicePriority(static_cast<uint8_t>(mask),false,reject));assert(calls==0);
  assert(!servicePriority(static_cast<uint8_t>(mask),true,reject));
  assert(calls==((mask&127U)?1U:0U));
 }
 unsigned primaryCalls=0,backupCalls=0;
 assert(!servicePriority(0x7f,true,[&](TxPriority p){++primaryCalls;assert(p==TxPriority::Stop);return false;}));
 assert(servicePriority(0x04,true,[&](TxPriority p){++backupCalls;assert(p==TxPriority::Handshake);return true;}));
 assert(primaryCalls==1&&backupCalls==1);

 Transport<uint32_t> dualP,dualB;
 PhyEvents<uint32_t> dualPE,dualBE;
 flytec_r30h121::DualAppRadio<uint32_t> dualEndpoint(dualP,dualPE,dualB,dualBE);
 assert(dualEndpoint.primary().reset(1)&&dualEndpoint.backup().reset(1));
 assert(dualPE.publishHealth({10,true,true,false,false}));
 assert(dualBE.publishHealth({10,true,true,false,false}));
 unsigned dpCalls=0,dbCalls=0;
 auto dpService=[&](AppRadio<uint32_t>& ep,TxPriority p){++dpCalls;assert(p==TxPriority::Telemetry);return ep.submit(11,10,false);};
 auto dbService=[&](AppRadio<uint32_t>& ep,TxPriority p){++dbCalls;assert(p==TxPriority::Telemetry);return ep.submit(22,10,false);};
 dualEndpoint.step(10,100,200,32,32,dpService,dbService);
 assert(dpCalls==1&&dbCalls==1);
 Transport<uint32_t>::Request dpr{},dbr{};
 assert(dualP.take(dpr)&&dualB.take(dbr));
 assert(dualP.complete(dpr,true)&&dualB.complete(dbr,true));
 assert(dualBE.publishOutcome({dbr.ticket,11,PhyOutcome::LocalComplete}));
 dualEndpoint.step(11,100,200,0,0,dpService,dbService);
 assert(dualEndpoint.primary().state()==TxState::Admitted);
 assert(dualEndpoint.backup().state()==TxState::LocalComplete);
 assert(dualPE.publishHealth({12,true,false,true,false}));
 dualEndpoint.step(12,100,200,32,32,dpService,dbService);
 assert(dpCalls==1&&dbCalls==2);
 assert(dualEndpoint.primary().preemptStop(90,13,1));
 assert(!dualEndpoint.primary().preemptStop(91,14,1));
 assert(dualEndpoint.primary().stopRetryPending());
 assert(dualP.take(dpr)&&!dualP.allows(dpr,15,100));
 assert(dualB.take(dbr)&&dualB.complete(dbr,true));
 assert(dualBE.publishOutcome({dbr.ticket,15,PhyOutcome::LocalComplete}));
 dualEndpoint.step(15,100,200,32,32,dpService,dbService);
 assert(!dualEndpoint.primary().stopRetryPending());
 assert(dualP.take(dpr)&&dpr.stop&&dpr.frame==91&&dpr.queuedAt==14);
 assert(dpCalls==1&&dbCalls==3);
}
