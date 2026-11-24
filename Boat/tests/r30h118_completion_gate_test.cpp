#include "../source/boat_r30h18_stop_ack_outbox.h"
#include "../source/r30h121_app_radio.h"
#include "../source/r30h123_stop_ack_service.h"
#include "../source/r30h123_owner_cycle.h"
#include "../source/r30h123_owner_rx.h"
#include "../source/r30h123_ordinary_tx.h"
#include "../source/r30h123_telemetry_slots.h"
#include "../source/r30h123_runtime_owner.h"
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
 assert(!hsDomain.invalidate(0));assert(hsDomain.activeGeneration()==0);assert(hsDomain.invalidate(1));
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
 Ticket auditDone{};TxState auditState=TxState::Idle;
 assert(!endpoint.submit(43,20,false));
 assert(endpoint.takeTerminal(auditDone,auditState)&&auditState==TxState::LocalComplete);
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
 assert(!stopEndpoint.rearm(1,true));
 assert(stopEndpoint.takeTerminal(auditDone,auditState));
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
 assert(dualEndpoint.backup().takeTerminal(auditDone,auditState));
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
 // Backup terminal must be consumed before another ordinary admission.
 assert(dpCalls==1&&dbCalls==2);
 assert(dualEndpoint.backup().terminalPending());
 assert(dualEndpoint.backup().takeTerminal(auditDone,auditState)&&auditState==TxState::LocalComplete);
 flytec_r30h121::DualAppRadio<uint32_t>::serviceLink(dualEndpoint.backup(),16,100,32,dbService);
 assert(dpCalls==1&&dbCalls==3);

 PhyEvents<uint32_t>::Rx freshRx{};
 assert(asyncEvents.publishRx({1,10}));
 assert(asyncEvents.publishRx({2,20}));
 assert(asyncEvents.publishRx({3,99}));
 assert(endpoint.receiveFresh(freshRx,100,10)&&freshRx.frame==3);
 assert(!endpoint.receiveFresh(freshRx,100,10));
 assert(asyncEvents.publishRx({4,UINT32_MAX-2}));
 assert(endpoint.receiveFresh(freshRx,1,10)&&freshRx.frame==4);
 assert(asyncEvents.publishRx({5,10}));
 assert(!endpoint.receiveFresh(freshRx,20,10));
 assert(asyncEvents.publishRx({6,20}));
 assert(!endpoint.receiveFresh(freshRx,20,0));

 Transport<uint32_t> terminalBus;
 PhyEvents<uint32_t> terminalEvents;
 AppRadio<uint32_t> terminalEndpoint(terminalBus,terminalEvents);
 Ticket terminalTicket{};TxState terminalState=TxState::Idle;
 assert(terminalEndpoint.reset(1));
 assert(!terminalEndpoint.takeTerminal(terminalTicket,terminalState));
 assert(terminalEndpoint.submit(70,10,false));
 assert(!terminalEndpoint.takeTerminal(terminalTicket,terminalState));
 assert(terminalBus.take(asyncReq)&&terminalBus.complete(asyncReq,true));
 terminalEndpoint.poll();
 assert(!terminalEndpoint.takeTerminal(terminalTicket,terminalState));
 assert(terminalEvents.publishOutcome({asyncReq.ticket,11,PhyOutcome::Failed}));
 terminalEndpoint.poll();
 assert(terminalEndpoint.takeTerminal(terminalTicket,terminalState));
 assert(sameTicket(terminalTicket,asyncReq.ticket)&&terminalState==TxState::Failed);
 assert(!terminalEndpoint.takeTerminal(terminalTicket,terminalState));
 assert(terminalEndpoint.submit(71,12,false));
 assert(terminalBus.take(asyncReq));
 assert(terminalEndpoint.expire(22,10));
 assert(terminalEndpoint.takeTerminal(terminalTicket,terminalState));
 assert(sameTicket(terminalTicket,asyncReq.ticket)&&terminalState==TxState::Uncertain);
 assert(terminalEvents.publishOutcome({asyncReq.ticket,23,PhyOutcome::LocalComplete}));
 terminalEndpoint.poll();
 assert(!terminalEndpoint.takeTerminal(terminalTicket,terminalState));
 assert(terminalEndpoint.reset(2));
 assert(!terminalEndpoint.takeTerminal(terminalTicket,terminalState));

 using flytec_r30h121::AckLink;
 using flytec_r30h121::StopAckCorrelation;
 boat_r30h18::StopAckOutbox ackBox;
 StopAckCorrelation corr(AckLink::Primary);
 Transport<uint32_t> ackBus;
 Ticket ackTicket{};
 assert(ackBus.invalidate(1)&&ackBus.submit(1,10,true,&ackTicket));
 assert(ackBox.offer(10,1,1,0,0,10));
 ackBox.admitted(10);
 assert(corr.track(ackTicket,ackBox));
 assert(!corr.complete(ackBox,AckLink::Backup,ackTicket,TxState::LocalComplete));
 assert(ackBox.inFlight());
 assert(corr.complete(ackBox,AckLink::Primary,ackTicket,TxState::Uncertain));
 assert(ackBox.pending()&&!ackBox.inFlight());
 assert(!corr.complete(ackBox,AckLink::Primary,ackTicket,TxState::LocalComplete));
 ackBox.admitted(90);
 assert(corr.track(ackTicket,ackBox));
 assert(ackBox.offer(11,1,1,0,0,91));
 ackBox.admitted(91);
 assert(!corr.complete(ackBox,AckLink::Primary,ackTicket,TxState::LocalComplete));
 assert(ackBox.pending()&&ackBox.inFlight()&&ackBox.ticket().commandId==11);
 corr.reset();
 assert(!corr.complete(ackBox,AckLink::Primary,ackTicket,TxState::LocalComplete));

 // Exact ACK payload promotion must survive an older successful completion.
 boat_r30h18::BackupStopAckOutbox promotedAck;
 StopAckCorrelation promotedCorrelation(AckLink::Backup);
 assert(promotedAck.offer(12,1,0,0,0,100));
 promotedAck.admitted(100);
 assert(promotedCorrelation.track(ackTicket,promotedAck));
 assert(promotedAck.offer(12,1,1,1,3,101));
 assert(promotedCorrelation.complete(promotedAck,AckLink::Backup,ackTicket,TxState::LocalComplete));
 assert(promotedAck.pending()&&!promotedAck.inFlight()&&promotedAck.ticket().result==1);
 assert(!promotedAck.due(174,true,true,false));
 assert(promotedAck.due(175,true,true,false));
 promotedAck.admitted(175);
 assert(promotedCorrelation.track(ackTicket,promotedAck));
 assert(promotedCorrelation.complete(promotedAck,AckLink::Backup,ackTicket,TxState::LocalComplete));
 assert(!promotedAck.pending());

 // Authentication policy is mocked here; actual HMAC is tested separately.
 HandshakeLane<HandshakeFrame> authLane;
 Transport<HandshakeFrame> rolloverApp;
 assert(authLane.begin()&&rolloverApp.invalidate(1));
 hf={13,1,2,0,0,0,0};
 Ticket confirmTicket{};
 unsigned verifyCalls=0;
 auto rejectAuth=[&](const HandshakeFrame&){++verifyCalls;return false;};
 auto acceptAuth=[&](const HandshakeFrame&){++verifyCalls;return true;};
 assert(!authLane.submitAuthenticated(hf,100,policy,rejectAuth,&confirmTicket));
 assert(verifyCalls==1&&!authLane.take(hr));
 hf.flags=1;
 assert(!authLane.submitAuthenticated(hf,100,policy,acceptAuth,&confirmTicket));
 assert(verifyCalls==1);
 hf.flags=0;
 assert(authLane.submitAuthenticated(hf,100,policy,acceptAuth,&confirmTicket));
 assert(confirmTicket.domain==TicketDomain::Handshake);
 assert(rolloverApp.invalidate(2));
 assert(authLane.take(hr)&&sameTicket(hr.ticket,confirmTicket));
 assert(authLane.allows(hr,101,100,policy));
 authLane.close();
 assert(!authLane.allows(hr,101,100,policy));
 assert(authLane.complete(hr,true));
 assert(!authLane.poll(handshakeAdmitted));
 assert(authLane.begin());
 assert(authLane.submitAuthenticated(hf,102,policy,acceptAuth,&confirmTicket));
 assert(authLane.take(hr)&&sameTicket(hr.ticket,confirmTicket));
 assert(authLane.complete(hr,true)&&authLane.poll(handshakeAdmitted)&&handshakeAdmitted);

 // Exhaustive shared-PHY selection, including failed admission.
 for(unsigned mask=0;mask<256;++mask){
  for(unsigned busy=0;busy<2;++busy)for(unsigned enabled=0;enabled<2;++enabled)
   for(unsigned accept=0;accept<2;++accept){
    unsigned appCalls=0,hsCalls=0;
    const auto selected=choosePriority(static_cast<uint8_t>(mask));
    const bool eligible=enabled&&!busy&&selected!=TxPriority::None;
    const bool result=serviceSharedPhy(static_cast<uint8_t>(mask),busy,enabled,
     [&](TxPriority p){++appCalls;assert(p==selected&&p!=TxPriority::Handshake);return accept!=0;},
     [&](){++hsCalls;assert(selected==TxPriority::Handshake);return accept!=0;});
    assert(result==(eligible&&accept));
    assert(appCalls==(eligible&&selected!=TxPriority::Handshake?1U:0U));
    assert(hsCalls==(eligible&&selected==TxPriority::Handshake?1U:0U));
   }
 }
 unsigned sharedPrimaryCalls=0,sharedBackupCalls=0;
 assert(!serviceSharedPhy(0x05,true,true,[&](TxPriority){++sharedPrimaryCalls;return true;},[&](){++sharedPrimaryCalls;return true;}));
 assert(serviceSharedPhy(0x05,false,true,[&](TxPriority p){++sharedBackupCalls;assert(p==TxPriority::Stop);return true;},[&](){++sharedBackupCalls;return false;}));
 assert(sharedPrimaryCalls==0&&sharedBackupCalls==1);

 // Atomic publication is independent per PHY; reserved bit is ignored.
 SharedPriority primaryPriority,backupPriority;
 assert(primaryPriority.snapshot()==0&&backupPriority.snapshot()==0);
 for(unsigned mask=0;mask<256;++mask){
  primaryPriority.publish(static_cast<uint8_t>(mask));
  assert(primaryPriority.snapshot()==(mask&127U));
  assert(backupPriority.snapshot()==0);
 }
 primaryPriority.publish(5);backupPriority.publish(4);
 unsigned priorityAppCalls=0,priorityHsCalls=0;
 auto priorityApp=[&](TxPriority p){++priorityAppCalls;assert(p==TxPriority::Stop);return true;};
 auto priorityHs=[&](){++priorityHsCalls;return true;};
 assert(!primaryPriority.service(true,true,priorityApp,priorityHs));
 assert(!primaryPriority.service(false,false,priorityApp,priorityHs));
 assert(primaryPriority.service(false,true,priorityApp,priorityHs));
 assert(backupPriority.service(false,true,priorityApp,priorityHs));
 assert(priorityAppCalls==1&&priorityHsCalls==1);
 primaryPriority.publish(0);
 assert(!primaryPriority.service(false,true,priorityApp,priorityHs));
 assert(backupPriority.snapshot()==4);
 // Worker handshake service: disabled, stale and valid requests.
 HandshakeLane<HandshakeFrame> workerLane;
 flytec_r30h120::HandshakeService<HandshakeFrame> hsWorker(workerLane);
 unsigned hsAdmissions=0;
 auto hsAdmit=[&](const HandshakeLane<HandshakeFrame>::Request &req){
  ++hsAdmissions;assert(req.ticket.domain==TicketDomain::Handshake);return true;
 };
 assert(workerLane.begin());
 assert(workerLane.submit(hf,100,policy));
 assert(!hsWorker.step(101,10,false,policy,hsAdmit));
 assert(!hsWorker.step(110,10,true,policy,hsAdmit));
 assert(workerLane.poll(handshakeAdmitted)&&!handshakeAdmitted&&hsAdmissions==0);
 assert(workerLane.submit(hf,111,policy));
 assert(hsWorker.step(112,10,true,policy,hsAdmit));
 assert(workerLane.poll(handshakeAdmitted)&&handshakeAdmitted&&hsAdmissions==1);
 assert(!hsWorker.resultPending());
 // Shared admission: STOP before handshake, no admission while busy.
 Transport<HandshakeFrame> sharedAppBus;
 HandshakeLane<HandshakeFrame> sharedHsLane;
 SharedPriority sharedMask;
 flytec_r30h120::SharedPhyService<HandshakeFrame> sharedWorker(sharedAppBus,sharedHsLane,sharedMask);
 assert(sharedAppBus.invalidate(1)&&sharedHsLane.begin());
 assert(sharedAppBus.submit(hf,200,true)&&sharedHsLane.submit(hf,200,policy));
 unsigned sharedAppAdmissions=0,sharedHsAdmissions=0;
 auto sharedAppAdmit=[&](const Transport<HandshakeFrame>::Request &req){++sharedAppAdmissions;assert(req.stop);return true;};
 auto sharedHsAdmit=[&](const HandshakeLane<HandshakeFrame>::Request &req){++sharedHsAdmissions;assert(req.ticket.domain==TicketDomain::Handshake);return true;};
 sharedMask.publish(5);
 sharedWorker.step(201,100,true,true,policy,sharedAppAdmit,sharedHsAdmit);
 sharedWorker.step(201,100,false,false,policy,sharedAppAdmit,sharedHsAdmit);
 assert(sharedAppAdmissions==0&&sharedHsAdmissions==0);
 sharedWorker.step(202,100,false,true,policy,sharedAppAdmit,sharedHsAdmit);
 assert(sharedAppAdmissions==1&&sharedHsAdmissions==0);
 assert(sharedAppBus.poll(handshakeAdmitted)&&handshakeAdmitted);
 sharedMask.publish(4);
 sharedWorker.step(203,100,false,true,policy,sharedAppAdmit,sharedHsAdmit);
 assert(sharedHsAdmissions==1&&sharedHsLane.poll(handshakeAdmitted)&&handshakeAdmitted);
 assert(sharedAppBus.submit(hf,204,false));
 sharedMask.publish(1);
 sharedWorker.step(205,100,false,true,policy,sharedAppAdmit,sharedHsAdmit);
 assert(sharedAppAdmissions==1&&!sharedAppBus.poll(handshakeAdmitted));
 assert(sharedAppBus.takeOrdinary(hr));
 assert(sharedAppBus.complete(hr,false)&&sharedAppBus.poll(handshakeAdmitted)&&!handshakeAdmitted);
 // Domain-specific outcomes with identical numeric ticket identifiers.
 PhyEvents<uint32_t> domainEvents;
 Ticket appOutcomeTicket{1,1,1,TicketDomain::Application};
 Ticket hsOutcomeTicket{1,1,1,TicketDomain::Handshake};
 for(unsigned i=0;i<4;++i)assert(domainEvents.publishOutcome({appOutcomeTicket,i,PhyOutcome::LocalComplete}));
 assert(!domainEvents.publishOutcome({appOutcomeTicket,4,PhyOutcome::Failed}));
 for(unsigned i=0;i<4;++i)assert(domainEvents.publishOutcome({hsOutcomeTicket,i,PhyOutcome::Uncertain}));
 assert(!domainEvents.publishOutcome({hsOutcomeTicket,4,PhyOutcome::Failed}));
 PhyEvents<uint32_t>::Outcome routed{};
 for(unsigned i=0;i<4;++i){
  assert(domainEvents.takeOutcome(routed)&&sameTicket(routed.ticket,appOutcomeTicket)&&routed.observedAt==i);
 }
 assert(!domainEvents.takeOutcome(routed));
 for(unsigned i=0;i<4;++i){
  assert(domainEvents.takeHandshakeOutcome(routed)&&sameTicket(routed.ticket,hsOutcomeTicket)&&routed.observedAt==i);
 }
 assert(!domainEvents.takeHandshakeOutcome(routed));
 // Handshake endpoint accepts outcome before admission, rejects stale tickets.
 HandshakeLane<HandshakeFrame> endpointHsLane;
 PhyEvents<HandshakeFrame> endpointHsEvents;
 flytec_r30h120::HandshakeEndpoint<HandshakeFrame> endpointHs(endpointHsLane,endpointHsEvents);
 using HsState=flytec_r30h120::HandshakeTx;
 Ticket hsDone{}; HsState hsState=HsState::Idle;
 assert(endpointHs.begin());
 assert(endpointHs.submit(hf,300,policy,acceptAuth));
 assert(!endpointHs.submit(hf,300,policy,acceptAuth));
 assert(endpointHsLane.take(hr));
 assert(endpointHsEvents.publishOutcome({hr.ticket,301,PhyOutcome::LocalComplete}));
 endpointHs.poll(301,100);
 assert(endpointHs.state()==HsState::Pending);
 assert(endpointHsLane.complete(hr,true));
 endpointHs.poll(302,100);
 assert(endpointHs.takeTerminal(hsDone,hsState)&&sameTicket(hsDone,hr.ticket)&&hsState==HsState::LocalComplete);
 assert(!endpointHs.takeTerminal(hsDone,hsState));
 assert(endpointHs.submit(hf,303,policy,acceptAuth));
 assert(endpointHsLane.take(hr));
 assert(endpointHsEvents.publishOutcome({hsDone,304,PhyOutcome::LocalComplete}));
 assert(endpointHsLane.complete(hr,true));
 endpointHs.poll(304,100);
 assert(endpointHs.state()==HsState::Pending);
 endpointHs.poll(403,100);
 assert(endpointHs.takeTerminal(hsDone,hsState)&&hsState==HsState::Uncertain);
 assert(endpointHsLane.activeGeneration()==0);
 assert(endpointHs.begin());
 // STOP preemption preserves prior ticket without blocking safety admission.
 Transport<uint32_t> supersedeBus;
 PhyEvents<uint32_t> supersedeEvents;
 AppRadio<uint32_t> supersede(supersedeBus,supersedeEvents);
 assert(supersede.reset(1)&&supersede.submit(1,10,false));
 const Ticket original=supersede.currentTicket();
 assert(supersede.preemptStop(2,11,1));
 const Ticket firstStop=supersede.currentTicket();
 assert(!sameTicket(original,firstStop));
 Transport<uint32_t>::Request supersedeReq{};
 assert(supersedeBus.take(supersedeReq)&&supersedeReq.stop);
 assert(supersede.preemptStop(3,12,1));
 assert(supersede.supersededOverflow()==1);
 Ticket saved{};TxState savedState=TxState::Idle;
 assert(supersede.takeSuperseded(saved,savedState)&&sameTicket(saved,original)&&savedState==TxState::Uncertain);
 assert(!supersede.takeSuperseded(saved,savedState));
 const Ticket secondStop=supersede.currentTicket();
 assert(supersede.reset(2));
 assert(supersede.takeSuperseded(saved,savedState)&&sameTicket(saved,secondStop)&&savedState==TxState::Uncertain);
 assert(!supersede.takeSuperseded(saved,savedState));
 // Deterministic priority change inside handshake validation.
 Transport<HandshakeFrame> raceApp;
 HandshakeLane<HandshakeFrame> raceHs;
 SharedPriority raceMask;
 flytec_r30h120::SharedPhyService<HandshakeFrame> raceWorker(raceApp,raceHs,raceMask);
 assert(raceApp.invalidate(1)&&raceHs.begin());
 assert(raceHs.submit(hf,500,policy));
 raceMask.publish(4);
 unsigned raceHsCalls=0,raceStopCalls=0;
 auto racePolicy=[&](const HandshakeFrame &f){
  raceMask.publish(5);
  return policy(f);
 };
 auto raceAppAdmit=[&](const Transport<HandshakeFrame>::Request &req){assert(req.stop);++raceStopCalls;return true;};
 auto raceHsAdmit=[&](const HandshakeLane<HandshakeFrame>::Request &){++raceHsCalls;return true;};
 raceWorker.step(501,100,false,true,racePolicy,raceAppAdmit,raceHsAdmit);
 assert(raceHsCalls==0&&raceStopCalls==0);
 assert(raceHs.poll(handshakeAdmitted)&&!handshakeAdmitted);
 assert(raceApp.submit(hf,502,true));
 raceWorker.step(503,100,false,true,policy,raceAppAdmit,raceHsAdmit);
 assert(raceStopCalls==1&&raceHsCalls==0);
 assert(raceApp.poll(handshakeAdmitted)&&handshakeAdmitted);
 // A stale ordinary priority must leave STOP queued for the next STOP cycle.
 assert(raceApp.invalidate(1));
 assert(raceApp.submit(hf,510,true));
 raceMask.publish(32);
 raceWorker.step(511,100,false,true,policy,raceAppAdmit,raceHsAdmit);
 assert(raceStopCalls==1&&raceHsCalls==0);
 assert(!raceApp.poll(handshakeAdmitted));
 raceMask.publish(1);
 raceWorker.step(512,100,false,true,policy,raceAppAdmit,raceHsAdmit);
 assert(raceStopCalls==2&&raceHsCalls==0);
 assert(raceApp.poll(handshakeAdmitted)&&handshakeAdmitted);
 // Ordinary admission result saturation must not consume STOP capacity.
 Transport<uint32_t> pressure;
 flytec_r30h120::PhyService<uint32_t> pressureWorker(pressure);
 Transport<uint32_t>::Request pressureReq{};
 for(unsigned i=0;i<3;++i){
  assert(pressure.invalidate(1)&&pressure.submit(i,600,false));
  assert(pressure.takeOrdinary(pressureReq)&&pressure.complete(pressureReq,true));
 }
 assert(pressure.invalidate(1)&&pressure.submit(9,601,false));
 pressureWorker.step(602,100,true,[](const Transport<uint32_t>::Request &){return true;});
 assert(pressureWorker.resultPending());
 assert(pressure.invalidate(1)&&pressure.submit(10,603,true));
 unsigned pressureStops=0;
 pressureWorker.stepStop(604,100,[&](const Transport<uint32_t>::Request &req){assert(req.stop);++pressureStops;return true;});
 assert(pressureStops==1&&pressure.poll(handshakeAdmitted)&&handshakeAdmitted);
 assert(!pressure.poll(handshakeAdmitted));
 assert(pressureWorker.flush());
 // Dedicated STOP retry deadline across millis rollover.
 Transport<uint32_t> timingBus;
 PhyEvents<uint32_t> timingEvents;
 AppRadio<uint32_t> timing(timingBus,timingEvents);
 assert(timing.reset(1));
 assert(timing.preemptStop(1,UINT32_MAX-20,1));
 assert(!timing.preemptStop(2,UINT32_MAX-10,1,50));
 assert(timing.stopRetryPending());
 auto noTimingService=[](AppRadio<uint32_t>&,flytec_r30h120::TxPriority){return false;};
 flytec_r30h121::DualAppRadio<uint32_t>::serviceLink(timing,5,1,1,noTimingService);
 assert(timing.stopRetryPending());
 Transport<uint32_t>::Request timingReq{};
 assert(timingBus.takeStop(timingReq));
 flytec_r30h121::DualAppRadio<uint32_t>::serviceLink(timing,10,1,1,noTimingService);
 assert(!timing.stopRetryPending());
 assert(timingBus.takeStop(timingReq)&&timingReq.queuedAt==UINT32_MAX-10);
 assert(timingBus.allows(timingReq,10,50));
 assert(!timingBus.allows(timingReq,39,50));
 // Expiry at the exact boundary while queue remains occupied.
 assert(timing.preemptStop(3,100,1,50));
 assert(!timing.preemptStop(4,101,1,50));
 assert(timing.stopRetryPending());
 assert(!timing.serviceStop(151)&&!timing.stopRetryPending());
 assert(timingBus.activeGeneration()==0);
 // Explicit request deadline overrides the default, including rollover.
 Transport<uint32_t> deadlineBus;
 Transport<uint32_t>::Request deadlineReq{};
 assert(deadlineBus.invalidate(1));
 assert(deadlineBus.submit(1,UINT32_MAX-10,true,nullptr,50));
 assert(deadlineBus.takeStop(deadlineReq)&&deadlineReq.maxAge==50);
 assert(deadlineBus.allows(deadlineReq,38,10));
 assert(!deadlineBus.allows(deadlineReq,39,100));
 assert(deadlineBus.complete(deadlineReq,true));
 bool deadlineAdmitted=false;
 assert(deadlineBus.poll(deadlineAdmitted)&&deadlineAdmitted);
 assert(deadlineBus.submit(2,100,false));
 assert(deadlineBus.takeOrdinary(deadlineReq)&&deadlineReq.maxAge==0);
 assert(deadlineBus.allows(deadlineReq,109,10));
 assert(!deadlineBus.allows(deadlineReq,110,10));
 assert(!deadlineBus.allows(deadlineReq,100,0));
 deadlineBus.invalidate(0);
 assert(!deadlineBus.allows(deadlineReq,101,100));
 // Physical terminal backpressure must remain isolated by traffic class.
 PhyEvents<uint32_t> isolatedEvents;
 PhyPump<uint32_t> isolatedPump(isolatedEvents);
 Ticket ordinaryPhy{1,1,1,TicketDomain::Application};
 Ticket stopPhy{1,2,1,TicketDomain::Application};
 Ticket handshakePhy{1,1,1,TicketDomain::Handshake};
 for(unsigned i=0;i<4;++i){
  assert(isolatedEvents.publishOutcome({ordinaryPhy,i,PhyOutcome::LocalComplete}));
  assert(isolatedEvents.publishOutcome({handshakePhy,i,PhyOutcome::LocalComplete}));
 }
 assert(isolatedPump.track(ordinaryPhy));
 assert(isolatedPump.terminal(PhyOutcome::Failed,10));
 isolatedPump.flushOutcome();
 assert(!isolatedPump.canAdmit(ordinaryPhy));
 assert(isolatedPump.track(handshakePhy));
 assert(!isolatedPump.canAdmit(stopPhy,true));
 assert(isolatedPump.terminal(PhyOutcome::Uncertain,11));
 isolatedPump.flushOutcome();
 assert(isolatedPump.track(stopPhy,true));
 assert(isolatedPump.terminal(PhyOutcome::LocalComplete,12));
 isolatedPump.flushOutcome();
 PhyEvents<uint32_t>::Outcome isolatedResult{};
 assert(isolatedEvents.takeOutcome(isolatedResult)&&isolatedResult.stop&&sameTicket(isolatedResult.ticket,stopPhy));
 assert(isolatedEvents.takeOutcome(isolatedResult)&&!isolatedResult.stop);
 assert(isolatedEvents.takeHandshakeOutcome(isolatedResult));
 isolatedPump.flushOutcome();
 assert(isolatedPump.canAdmit());
 // Owner coordinator keeps root-key CONFIRM separate from application epoch.
 Transport<HandshakeFrame> ownerBus;
 PhyEvents<HandshakeFrame> ownerEvents;
 AppRadio<HandshakeFrame> ownerApp(ownerBus,ownerEvents);
 HandshakeLane<HandshakeFrame> ownerHs;
 SharedPriority ownerMask;
 flytec_r30h123::OwnerLink<HandshakeFrame> ownerLink(ownerApp,ownerHs,ownerEvents,ownerMask);
 assert(ownerLink.beginHandshake());
 HandshakeFrame ownerConfirm{13,1,2,0,0,0,0};
 assert(!ownerLink.submitHandshake(ownerConfirm,700,policy,rejectAuth));
 assert(ownerMask.snapshot()==0);
 assert(ownerLink.submitHandshake(ownerConfirm,700,policy,acceptAuth));
 assert(ownerMask.snapshot()==4);
 assert(ownerLink.setSession(7));
 assert(ownerMask.snapshot()==4);
 HandshakeFrame ownerOrdinary{1,1,2,0,7,1,0};
 assert(!ownerLink.submitApplication(ownerOrdinary,701,TxPriority::Telemetry,100,rejectAuth));
 assert(ownerLink.submitApplication(ownerOrdinary,701,TxPriority::Telemetry,100,acceptAuth));
 assert(ownerMask.snapshot()==36);
 assert(ownerLink.submitStop(ownerOrdinary,702,100,acceptAuth));
 assert(ownerMask.snapshot()==5);
 Transport<HandshakeFrame>::Request ownerReq{};
 assert(ownerBus.takeStop(ownerReq)&&ownerReq.stop);
 assert(ownerBus.complete(ownerReq,true));
 assert(ownerEvents.publishOutcome({ownerReq.ticket,703,PhyOutcome::LocalComplete,true}));
 ownerLink.poll(703,100,100);
 assert(ownerMask.snapshot()==4);
 assert(!ownerLink.rearm(true));
 assert(ownerLink.takeApplication(auditDone,auditState)&&auditState==TxState::LocalComplete);
 assert(ownerLink.rearm(true));
 assert(ownerHs.take(ownerReq));
 assert(ownerHs.complete(ownerReq,true));
 assert(ownerEvents.publishOutcome({ownerReq.ticket,704,PhyOutcome::LocalComplete}));
 ownerLink.poll(704,100,100);
 assert(ownerMask.snapshot()==0);
 HandshakeTx ownerHsState=HandshakeTx::Idle;
 assert(ownerLink.takeHandshake(auditDone,ownerHsState)&&ownerHsState==HandshakeTx::LocalComplete);
 ownerLink.close();
 assert(!ownerLink.submitApplication(ownerOrdinary,705,TxPriority::Telemetry,100,acceptAuth));
 assert(ownerMask.snapshot()==0);
 // Independent owner STOP ACK services: deferred admission and promotion.
 Transport<HandshakeFrame> ackPBus,ackBBus;
 PhyEvents<HandshakeFrame> ackPEvents,ackBEvents;
 AppRadio<HandshakeFrame> ackPApp(ackPBus,ackPEvents),ackBApp(ackBBus,ackBEvents);
 HandshakeLane<HandshakeFrame> ackPHs,ackBHs;
 SharedPriority ackPMask,ackBMask;
 flytec_r30h123::OwnerLink<HandshakeFrame> ackPOwner(ackPApp,ackPHs,ackPEvents,ackPMask),ackBOwner(ackBApp,ackBHs,ackBEvents,ackBMask);
 boat_r30h18::StopAckOutbox ackPBox;
 boat_r30h18::BackupStopAckOutbox ackBBox;
 flytec_r30h123::StopAckService<HandshakeFrame,decltype(ackPBox)> ackPSvc(ackPOwner,ackPBox,AckLink::Primary);
 flytec_r30h123::StopAckService<HandshakeFrame,decltype(ackBBox)> ackBSvc(ackBOwner,ackBBox,AckLink::Backup);
 auto buildAck=[](const boat_r30h18::StopAck& ack,HandshakeFrame& f){
  f={1,1,2,0,ack.epoch,1,ack.commandId};return true;
 };
 assert(ackPOwner.setSession(7)&&ackBOwner.setSession(7));
 assert(ackPOwner.submitStop(ownerOrdinary,800,100,acceptAuth));
 assert(ackPBox.offer(21,7,0,0,0,800)&&ackBBox.offer(22,7,0,0,0,800));
 assert(ackPSvc.step(810,true,true,buildAck,acceptAuth));
 assert(ackPSvc.pending()&&ackPOwner.stopRetryPending()&&ackPBox.attempts()==1);
 assert(ackBSvc.step(810,true,true,buildAck,acceptAuth));
 assert(ackBBus.takeStop(ownerReq)&&ownerReq.frame.commandId==22&&ownerReq.maxAge==11990);
 assert(ackBBus.complete(ownerReq,true));
 assert(ackBEvents.publishOutcome({ownerReq.ticket,811,PhyOutcome::LocalComplete,true}));
 ackBOwner.poll(811,100,100);
 assert(ackBOwner.takeApplication(auditDone,auditState)&&ackBSvc.complete(auditDone,auditState));
 assert(!ackBBox.pending()&&ackPBox.pending());
 assert(ackPBox.offer(21,7,1,1,3,812));
 assert(ackPBus.takeStop(ownerReq)&&!ackPBus.allows(ownerReq,812,100));
 ackPOwner.poll(812,100,100);
 ackPSvc.syncDeferred();
 assert(ackPBus.takeStop(ownerReq)&&ownerReq.frame.commandId==21&&ownerReq.queuedAt==810&&ownerReq.maxAge==2490);
 assert(ackPBus.complete(ownerReq,true));
 assert(ackPEvents.publishOutcome({ownerReq.ticket,813,PhyOutcome::LocalComplete,true}));
 ackPOwner.poll(813,100,100);
 assert(ackPOwner.takeApplication(auditDone,auditState)&&ackPSvc.complete(auditDone,auditState));
 assert(ackPBox.pending()&&!ackPBox.inFlight()&&ackPBox.ticket().result==1);
 assert(!ackPSvc.step(884,true,true,buildAck,acceptAuth));
 assert(ackPSvc.step(885,true,true,buildAck,acceptAuth));
 assert(ackPBus.takeStop(ownerReq)&&ownerReq.maxAge==2415);
 assert(ackPBus.complete(ownerReq,true));
 assert(ackPEvents.publishOutcome({ownerReq.ticket,886,PhyOutcome::LocalComplete,true}));
 ackPOwner.poll(886,100,100);
 assert(ackPOwner.takeApplication(auditDone,auditState)&&ackPSvc.complete(auditDone,auditState));
 assert(!ackPBox.pending());
 ackPSvc.reset();ackPOwner.close();
 assert(!ackPSvc.pending()&&!ackPBox.pending());
 // Independent STATUS feedback, ACK precedence, replacement and expiry.
 Transport<HandshakeFrame> statusPBus,statusBBus;
 PhyEvents<HandshakeFrame> statusPEvents,statusBEvents;
 AppRadio<HandshakeFrame> statusPApp(statusPBus,statusPEvents),statusBApp(statusBBus,statusBEvents);
 HandshakeLane<HandshakeFrame> statusPHs,statusBHs;
 SharedPriority statusPMask,statusBMask;
 flytec_r30h123::OwnerLink<HandshakeFrame> statusPOwner(statusPApp,statusPHs,statusPEvents,statusPMask),statusBOwner(statusBApp,statusBHs,statusBEvents,statusBMask);
 boat_r30h19::StatusOutbox statusPBox,statusBBox;
 flytec_r30h123::StatusService<HandshakeFrame> statusPSvc(statusPOwner,statusPBox),statusBSvc(statusBOwner,statusBBox);
 auto buildStatus=[](const RadioDecodedPayload& feedback,HandshakeFrame& f){
  f={1,1,2,0,7,1,static_cast<uint16_t>(feedback.stopState)};return true;
 };
 RadioDecodedPayload stoppedFeedback{};
 stoppedFeedback.kind=RADIO_PAYLOAD_STATUS;stoppedFeedback.stopState=1;
 assert(statusPOwner.setSession(7)&&statusBOwner.setSession(7));
 assert(statusPBox.offer(stoppedFeedback,1000)&&statusBBox.offer(stoppedFeedback,1000));
 assert(!statusPSvc.step(1001,true,true,true,buildStatus,acceptAuth));
 assert(statusBSvc.step(1001,true,true,false,buildStatus,acceptAuth));
 assert(statusBBus.takeStop(ownerReq)&&ownerReq.maxAge==300);
 assert(statusBBus.complete(ownerReq,true));
 assert(statusBEvents.publishOutcome({ownerReq.ticket,1002,PhyOutcome::LocalComplete,true}));
 statusBOwner.poll(1002,100,100);
 assert(statusBOwner.takeApplication(auditDone,auditState)&&statusBSvc.complete(auditDone,auditState));
 assert(!statusBBox.pending()&&statusPBox.pending());
 assert(statusPSvc.step(1003,true,true,false,buildStatus,acceptAuth));
 assert(statusPBus.takeStop(ownerReq));
 const auto obsoleteStatusTicket=ownerReq.ticket;
 stoppedFeedback.stopState=2;
 assert(statusPBox.offer(stoppedFeedback,1004));
 statusPSvc.reconcile(1004,true,true,false);
 assert(!statusPBus.allows(ownerReq,1004,100));
 assert(statusPOwner.takeApplication(auditDone,auditState)&&auditState==TxState::Uncertain);
 assert(!statusPSvc.complete(obsoleteStatusTicket,TxState::LocalComplete));
 assert(statusPBox.pending()&&statusPBox.status().stopState==2);
 assert(statusPSvc.step(1005,true,true,false,buildStatus,acceptAuth));
 assert(statusPBus.takeOrdinary(ownerReq)&&ownerReq.maxAge==300);
 statusPSvc.reconcile(1305,true,true,false);
 assert(!statusPBox.pending()&&!statusPSvc.pending());
 assert(!statusPBus.allows(ownerReq,1305,100));
 assert(statusPOwner.takeApplication(auditDone,auditState));
 stoppedFeedback.stopState=1;
 assert(statusPBox.offer(stoppedFeedback,1400));
 assert(statusPSvc.step(1401,true,true,false,buildStatus,acceptAuth));
 assert(statusPBus.takeStop(ownerReq));
 statusPSvc.reconcile(1402,true,true,true);
 assert(!statusPBus.allows(ownerReq,1402,100));
 assert(statusPBox.pending()&&!statusPSvc.pending());
 // Unified owner cycle: ACK before STATUS, independent backup, and
 // cancellation before deferred admission when trust or freshness is lost.
 boat_r30h19::StatusOutbox cyclePBox,cycleBBox;
 flytec_r30h123::StatusService<HandshakeFrame> cyclePSvc(ackPOwner,cyclePBox),cycleBSvc(ackBOwner,cycleBBox);
 flytec_r30h123::OwnerCycle<HandshakeFrame,decltype(ackPBox)> cycleP(ackPOwner,ackPSvc,cyclePSvc,ackPBox,cyclePBox);
 flytec_r30h123::OwnerCycle<HandshakeFrame,decltype(ackBBox)> cycleB(ackBOwner,ackBSvc,cycleBSvc,ackBBox,cycleBBox);
 assert(cycleP.resetSession(7)&&cycleB.resetSession(7));
 unsigned cycleOrdinary=0,cycleOther=0,cycleHandshake=0;
 auto cycleAppResult=[&](const Ticket&,TxState){++cycleOther;};
 auto cycleHsResult=[&](const Ticket&,HandshakeTx){++cycleHandshake;};
 auto cycleOrd=[&](flytec_r30h123::OwnerLink<HandshakeFrame>&){++cycleOrdinary;};
 auto tickP=[&](uint32_t now,bool session,bool trust){
  cycleP.step(now,100,100,session,trust,buildAck,buildStatus,acceptAuth,cycleAppResult,cycleHsResult,cycleOrd);
 };
 auto tickB=[&](uint32_t now){
  cycleB.step(now,100,100,true,true,buildAck,buildStatus,acceptAuth,cycleAppResult,cycleHsResult,cycleOrd);
 };
 assert(ackPBox.offer(31,7,1,1,0,1500));
 assert(cyclePBox.offer(stoppedFeedback,1500)&&cycleBBox.offer(stoppedFeedback,1500));
 tickP(1501,true,true);tickB(1501);
 assert(cycleOrdinary==0&&ackPBox.inFlight()&&cyclePBox.pending());
 assert(ackPBus.takeStop(ownerReq)&&ownerReq.frame.commandId==31);
 assert(ackPBus.complete(ownerReq,true));
 assert(ackPEvents.publishOutcome({ownerReq.ticket,1502,PhyOutcome::LocalComplete,true}));
 tickP(1502,true,true);
 assert(!ackPBox.pending()&&cyclePSvc.pending());
 assert(ackPBus.takeStop(ownerReq)&&ownerReq.frame.commandId==1);
 assert(ackPBus.complete(ownerReq,true));
 assert(ackPEvents.publishOutcome({ownerReq.ticket,1503,PhyOutcome::LocalComplete,true}));
 tickP(1503,true,true);
 assert(!cyclePBox.pending()&&cycleOrdinary==1);
 assert(ackBBus.takeStop(ownerReq)&&ownerReq.frame.commandId==1);
 assert(ackBBus.complete(ownerReq,true));
 assert(ackBEvents.publishOutcome({ownerReq.ticket,1503,PhyOutcome::LocalComplete,true}));
 tickB(1503);
 assert(!cycleBBox.pending()&&cycleOrdinary==2);
 // A full STOP queue forces reservation, then link trust cancels it.
 assert(ackPOwner.submitStop(ownerOrdinary,1600,100,acceptAuth));
 assert(ackPBox.offer(32,7,0,0,0,1600));
 tickP(1601,true,true);
 assert(ackPSvc.pending()&&ackPOwner.stopRetryPending());
 tickP(1602,true,false);
 assert(!ackPSvc.pending()&&!ackPBox.pending()&&!ackPOwner.stopRetryPending());
 assert(ackPBus.takeStop(ownerReq)&&!ackPBus.allows(ownerReq,1602,100));
 assert(!ackPBus.takeStop(ownerReq));
 assert(!ackPOwner.submitApplication(ownerOrdinary,1603,TxPriority::Telemetry,100,acceptAuth));
 // Deadline expires even if the reserved queue never drains.
 assert(ackPOwner.submitStop(ownerOrdinary,1700,100,acceptAuth));
 assert(ackPBox.offer(33,7,0,0,0,1700));
 tickP(1701,true,true);
 assert(ackPOwner.stopRetryPending());
 tickP(4200,true,true);
 assert(!ackPSvc.pending()&&!ackPBox.pending()&&!ackPOwner.stopRetryPending());
 assert(ackPBus.takeStop(ownerReq)&&!ackPBus.allows(ownerReq,4200,100));
 assert(!ackPBus.takeStop(ownerReq));
 cycleP.close();cycleB.close();
 assert(ackPMask.snapshot()==0&&ackBMask.snapshot()==0&&cycleHandshake==0);
 // Owner RX validates local capture age, direction, epoch and authentication.
 Transport<HandshakeFrame> rxOwnerBus;
 PhyEvents<HandshakeFrame> rxOwnerEvents;
 AppRadio<HandshakeFrame> rxOwnerApp(rxOwnerBus,rxOwnerEvents);
 flytec_r30h123::OwnerRx<HandshakeFrame> ownerRx(rxOwnerApp,1,2,false);
 uint8_t rxEpoch=7;
 unsigned rxDelivered=0,rxRootChecks=0,rxSessionChecks=0;
 auto epochNow=[&](){return rxEpoch;};
 auto rootCheck=[&](const HandshakeFrame&){++rxRootChecks;return true;};
 auto sessionCheck=[&](const HandshakeFrame& f){++rxSessionChecks;return f.commandId!=99;};
 auto rxDispatch=[&](const HandshakeFrame& f,uint32_t capturedAt){
  ++rxDelivered;
  if(f.type==12){assert(capturedAt==1001);rxEpoch=8;}
  else assert(f.sessionEpoch==rxEpoch);
 };
 HandshakeFrame rxProof{12,1,2,0,0,0,0};
 HandshakeFrame rxOld{1,1,2,0,7,1,0};
 HandshakeFrame rxNew{1,1,2,0,8,1,0};
 assert(rxOwnerEvents.publishRx({rxOld,900}));
 assert(rxOwnerEvents.publishRx({rxProof,1001}));
 assert(rxOwnerEvents.publishRx({rxOld,1002}));
 assert(rxOwnerEvents.publishRx({rxNew,1003}));
 assert(ownerRx.drain(1010,50,epochNow,rootCheck,sessionCheck,rxDispatch)==2);
 assert(rxDelivered==2&&rxRootChecks==1&&rxSessionChecks==1);
 auto rxWrong=rxNew;rxWrong.flags=1;
 auto rxForged=rxNew;rxForged.commandId=99;
 auto rxReply=rxProof;rxReply.type=13;
 assert(rxOwnerEvents.publishRx({rxWrong,1010}));
 assert(rxOwnerEvents.publishRx({rxForged,1010}));
 assert(rxOwnerEvents.publishRx({rxReply,1010}));
 assert(rxOwnerEvents.publishRx({rxNew,UINT32_MAX-2}));
 assert(ownerRx.drain(1011,50,epochNow,rootCheck,sessionCheck,rxDispatch)==0);
 assert(rxOwnerEvents.publishRx({rxNew,UINT32_MAX-2}));
 assert(ownerRx.drain(1,50,epochNow,rootCheck,sessionCheck,rxDispatch)==1);
 // Ordinary TX local completion is exact-ticket, one-shot and age limited.
 Transport<HandshakeFrame> ordinaryBus;
 PhyEvents<HandshakeFrame> ordinaryEvents;
 AppRadio<HandshakeFrame> ordinaryApp(ordinaryBus,ordinaryEvents);
 HandshakeLane<HandshakeFrame> ordinaryHs;
 SharedPriority ordinaryMask;
 flytec_r30h123::OwnerLink<HandshakeFrame> ordinaryOwner(ordinaryApp,ordinaryHs,ordinaryEvents,ordinaryMask);
 flytec_r30h123::OrdinaryTx<HandshakeFrame> ordinaryTrack;
 assert(ordinaryOwner.setSession(7));
 unsigned ordinaryResults=0;
 auto ordinaryResult=[&](const HandshakeFrame& f,TxState state,uint32_t at){
  assert(f.sessionEpoch==7);
  if(ordinaryResults==0){assert(state==TxState::LocalComplete&&at==5000);}
  else assert(state==TxState::Uncertain);
  ++ordinaryResults;
 };
 assert(ordinaryTrack.submit(ordinaryOwner,ownerOrdinary,5000,TxPriority::Heartbeat,100,acceptAuth));
 assert(ordinaryResults==0&&ordinaryTrack.pending());
 assert(ordinaryBus.takeOrdinary(ownerReq));
 const auto oldOrdinaryTicket=ownerReq.ticket;
 assert(!ordinaryTrack.complete(oldOrdinaryTicket,TxState::Admitted,5001,7,ordinaryResult));
 assert(ordinaryBus.complete(ownerReq,true));
 assert(ordinaryEvents.publishOutcome({ownerReq.ticket,5002,PhyOutcome::LocalComplete}));
 ordinaryOwner.poll(5002,100,100);
 assert(ordinaryOwner.takeApplication(auditDone,auditState));
 assert(ordinaryTrack.complete(auditDone,auditState,5002,7,ordinaryResult));
 assert(!ordinaryTrack.complete(auditDone,auditState,5002,7,ordinaryResult));
 assert(ordinaryTrack.submit(ordinaryOwner,ownerOrdinary,5100,TxPriority::Mission,100,acceptAuth));
 assert(!ordinaryTrack.complete(oldOrdinaryTicket,TxState::LocalComplete,5101,7,ordinaryResult));
 assert(ordinaryBus.takeOrdinary(ownerReq));
 assert(ordinaryBus.complete(ownerReq,true));
 assert(ordinaryEvents.publishOutcome({ownerReq.ticket,5102,PhyOutcome::LocalComplete}));
 ordinaryOwner.poll(5102,100,100);
 assert(ordinaryOwner.takeApplication(auditDone,auditState));
 assert(ordinaryTrack.complete(auditDone,auditState,5200,7,ordinaryResult));
 assert(ordinaryTrack.submit(ordinaryOwner,ownerOrdinary,5300,TxPriority::Telemetry,100,acceptAuth));
 assert(ordinaryBus.takeOrdinary(ownerReq));
 assert(ordinaryBus.complete(ownerReq,true));
 assert(ordinaryEvents.publishOutcome({ownerReq.ticket,5301,PhyOutcome::LocalComplete}));
 ordinaryOwner.poll(5301,100,100);
 assert(ordinaryOwner.takeApplication(auditDone,auditState));
 assert(ordinaryTrack.complete(auditDone,auditState,5301,8,ordinaryResult));
 assert(ordinaryResults==3&&!ordinaryTrack.pending());
 // Lost transport trust invalidates the exact outstanding snapshot and
 // discards its delayed success without touching any newer ticket.
 assert(ordinaryTrack.submit(ordinaryOwner,ownerOrdinary,5400,TxPriority::Heartbeat,100,acceptAuth));
 assert(ordinaryBus.takeOrdinary(ownerReq));
 const auto lostTrustRequest=ownerReq;
 assert(ordinaryTrack.cancel(ordinaryOwner));
 assert(!ordinaryTrack.pending()&&!ordinaryTrack.cancel(ordinaryOwner));
 assert(!ordinaryBus.allows(lostTrustRequest,5401,100));
 assert(ordinaryOwner.takeApplication(auditDone,auditState)&&auditState==TxState::Uncertain);
 assert(!ordinaryTrack.complete(auditDone,auditState,5401,7,ordinaryResult));
 assert(ordinaryTrack.submit(ordinaryOwner,ownerOrdinary,5402,TxPriority::Heartbeat,100,acceptAuth));
 assert(!ordinaryTrack.complete(lostTrustRequest.ticket,TxState::LocalComplete,5403,7,ordinaryResult));
 assert(ordinaryTrack.pending());
 ordinaryOwner.close();
 // Outbound Boat feedback can continue without clearing the STOP latch.
 Transport<HandshakeFrame> stoppedBus;
 PhyEvents<HandshakeFrame> stoppedEvents;
 AppRadio<HandshakeFrame> stoppedApp(stoppedBus,stoppedEvents);
 HandshakeLane<HandshakeFrame> stoppedHs;
 SharedPriority stoppedMask;
 flytec_r30h123::OwnerLink<HandshakeFrame> stoppedOwner(stoppedApp,stoppedHs,stoppedEvents,stoppedMask);
 assert(stoppedOwner.setSession(7));
 assert(stoppedOwner.submitStop(ownerOrdinary,6000,100,acceptAuth));
 assert(stoppedBus.takeStop(ownerReq)&&stoppedBus.complete(ownerReq,true));
 assert(stoppedEvents.publishOutcome({ownerReq.ticket,6001,PhyOutcome::LocalComplete,true}));
 stoppedOwner.poll(6001,100,100);
 assert(stoppedOwner.takeApplication(auditDone,auditState));
 assert(!stoppedOwner.ordinaryReady()&&stoppedOwner.feedbackReady());
 auto movementFrame=ownerOrdinary;movementFrame.type=2;
 assert(!stoppedOwner.submitFeedback(movementFrame,6002,TxPriority::Heartbeat,100,acceptAuth));
 assert(!stoppedOwner.submitFeedback(ownerOrdinary,6002,TxPriority::Mission,100,acceptAuth));
 assert(!stoppedOwner.submitApplication(ownerOrdinary,6002,TxPriority::Heartbeat,100,acceptAuth));
 assert(stoppedOwner.submitFeedback(ownerOrdinary,6002,TxPriority::Heartbeat,100,acceptAuth));
 assert(stoppedBus.takeOrdinary(ownerReq)&&stoppedBus.complete(ownerReq,true));
 assert(stoppedEvents.publishOutcome({ownerReq.ticket,6003,PhyOutcome::LocalComplete}));
 stoppedOwner.poll(6003,100,100);
 assert(stoppedOwner.takeApplication(auditDone,auditState)&&auditState==TxState::LocalComplete);
 assert(!stoppedOwner.ordinaryReady()&&stoppedOwner.feedbackReady());
 assert(!stoppedOwner.rearm(false));
 // A short-lived feedback request must not monopolize the endpoint until
 // the general 2500 ms timeout. Old worker results cannot affect a new ticket.
 assert(stoppedOwner.submitFeedback(ownerOrdinary,6100,TxPriority::Heartbeat,20,acceptAuth));
 assert(stoppedBus.takeOrdinary(ownerReq));
 const auto expiredFeedback=ownerReq;
 stoppedOwner.poll(6119,2500,2500);
 assert(!stoppedOwner.feedbackReady());
 stoppedOwner.poll(6120,2500,2500);
 assert(stoppedOwner.takeApplication(auditDone,auditState)&&auditState==TxState::Uncertain);
 assert(!stoppedBus.allows(expiredFeedback,6120,2500));
 assert(!stoppedOwner.ordinaryReady()&&stoppedOwner.feedbackReady());
 assert(stoppedOwner.submitFeedback(ownerOrdinary,6121,TxPriority::Heartbeat,100,acceptAuth));
 assert(stoppedBus.complete(expiredFeedback,true));
 assert(stoppedEvents.publishOutcome({expiredFeedback.ticket,6122,PhyOutcome::LocalComplete}));
 stoppedOwner.poll(6122,2500,2500);
 assert(!stoppedOwner.takeApplication(auditDone,auditState));
 assert(stoppedBus.takeOrdinary(ownerReq)&&stoppedBus.complete(ownerReq,true));
 assert(stoppedEvents.publishOutcome({ownerReq.ticket,6123,PhyOutcome::LocalComplete}));
 stoppedOwner.poll(6123,2500,2500);
 assert(stoppedOwner.takeApplication(auditDone,auditState)&&auditState==TxState::LocalComplete);
 assert(!stoppedOwner.ordinaryReady()&&stoppedOwner.feedbackReady());
 stoppedOwner.close();
 // Latest telemetry replacement, coordinate pairing and unsigned clock wrap.
 flytec_r30h123::TelemetrySlots telemetrySlots;
 uint8_t telemetryOut[8]{},batterySlot[8]={2,1},latSlot[8]={3,9},lonSlot[8]={4,9};
 uint32_t remaining=0;
 assert(telemetrySlots.offer(batterySlot,7000));
 batterySlot[1]=2;assert(telemetrySlots.offer(batterySlot,7010));
 assert(telemetrySlots.take(7011,telemetryOut,remaining)&&telemetryOut[1]==2&&remaining==249);
 assert(!telemetrySlots.take(7012,telemetryOut,remaining));
 assert(telemetrySlots.offer(latSlot,7020));
 assert(!telemetrySlots.take(7021,telemetryOut,remaining));
 assert(telemetrySlots.offer(lonSlot,7022));
 assert(!telemetrySlots.take(7023,telemetryOut,remaining)); // mismatched snapshot
 assert(telemetrySlots.offer(latSlot,7022));
 assert(telemetrySlots.take(7024,telemetryOut,remaining)&&telemetryOut[0]==3);
 assert(telemetrySlots.take(7025,telemetryOut,remaining)&&telemetryOut[0]==4&&remaining==247);
 assert(telemetrySlots.offer(latSlot,7030)&&telemetrySlots.offer(lonSlot,7030));
 assert(telemetrySlots.take(7031,telemetryOut,remaining));
 latSlot[1]=10;lonSlot[1]=10;
 assert(telemetrySlots.offer(latSlot,7032));
 assert(!telemetrySlots.take(7033,telemetryOut,remaining));
 assert(telemetrySlots.offer(lonSlot,7032));
 assert(telemetrySlots.take(7034,telemetryOut,remaining)&&telemetryOut[1]==10);
 assert(telemetrySlots.take(7035,telemetryOut,remaining)&&telemetryOut[1]==10);
 assert(telemetrySlots.offer(batterySlot,UINT32_MAX-4));
 assert(telemetrySlots.take(5,telemetryOut,remaining)&&remaining==240);
 assert(telemetrySlots.offer(batterySlot,8000));
 assert(!telemetrySlots.take(8250,telemetryOut,remaining));
 batterySlot[0]=1;assert(!telemetrySlots.offer(batterySlot,8251));
 telemetrySlots.reset();assert(!telemetrySlots.take(8251,telemetryOut,remaining));
 // Exclusive runtime ownership is irreversible even after task exit.
 flytec_r30h123::RuntimeOwner legacyRuntime,workerRuntime;
 assert(!legacyRuntime.claimWorker(false));
 assert(legacyRuntime.claimLegacy()&&legacyRuntime.legacy());
 assert(!legacyRuntime.claimWorker(true)&&!legacyRuntime.claimLegacy());
 legacyRuntime.retire();
 assert(!legacyRuntime.claimLegacy()&&!legacyRuntime.claimWorker(true));
 assert(workerRuntime.claimWorker(true)&&workerRuntime.worker());
 assert(!workerRuntime.claimLegacy()&&!workerRuntime.claimWorker(true));
 workerRuntime.retire();
 assert(!workerRuntime.legacy()&&!workerRuntime.worker()&&!workerRuntime.claimLegacy());
 // Physical availability must not confuse a busy E22 with a link fault.
 Transport<HandshakeFrame> healthBus;
 PhyEvents<HandshakeFrame> healthEvents;
 AppRadio<HandshakeFrame> healthApp(healthBus,healthEvents);
 assert(!healthApp.available(9000,250));
 assert(healthEvents.publishHealth({9000,true,false,false,true}));
 healthApp.pollHealth();
 assert(healthApp.available(9001,250)&&!healthApp.healthy(9001,250));
 assert(!healthApp.available(9250,250));
 assert(healthEvents.publishHealth({9251,true,true,false,false}));
 healthApp.pollHealth();
 assert(healthApp.available(9252,250)&&healthApp.healthy(9252,250));
 assert(healthEvents.publishHealth({9253,true,false,true,false}));
 healthApp.pollHealth();
 assert(!healthApp.available(9254,250));
 assert(healthEvents.publishHealth({9255,true,true,false,false}));
 healthApp.pollHealth();
 assert(!healthApp.available(9256,250)); // sticky fault; no automatic recovery
}
