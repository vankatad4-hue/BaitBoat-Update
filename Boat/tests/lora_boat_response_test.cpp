#include "radio_protocol.h"
#include "boat_radio_services.h"
#include <cassert>
using namespace flytec_r30h118;
using namespace flytec_r30h120;
using namespace flytec_r30h121;
using namespace flytec_r30h123;
using namespace flytec_rtos;
struct Driver {
 enum class Admission{Accepted,Busy,DutyLimited,Invalid,Unavailable,Uncertain};
 using O=E22TxState::Outcome;O state=O::Idle;RadioFrame last{};unsigned writes=0;
 bool ready()const{return !txPending()&&!transportFault();}
 bool configured()const{return !transportFault();}
 bool transportFault()const{return state==O::Uncertain;}
 bool txPending()const{return state==O::Pending;}
 O txOutcome()const{return state;}
 Admission trySendFrame(const RadioFrame& f,uint32_t){++writes;last=f;state=O::Pending;return Admission::Accepted;}
 bool receiveFrame(RadioFrame&){return false;}
 void update(){}
};
// Independent lanes: a temporary ordinary denial must not block STOP, and
// a retained request must never be reissued after generation or age expiry.
static void workerRetention(){
 using A=AdmissionResult;
 Transport<RadioFrame> bus;HandshakeLane<RadioFrame> hs;SharedPriority pr;
 SharedPhyService<RadioFrame> worker(bus,hs,pr);
 assert(bus.invalidate(1)&&hs.begin());
 RadioFrame ordinary{},stop{},handshake{};
 ordinary.type=1;stop.type=5;handshake.type=11;
 Ticket initial{},replacement{};
 auto shape=[](const RadioFrame& f){return f.type==11;};
 unsigned calls=0,stopCalls=0,hsCalls=0;bool blocked=true;
 auto app=[&](const Transport<RadioFrame>::Request& r){
  if(r.stop){++stopCalls;return A::Accepted;}
  ++calls;return blocked?A::Deferred:A::Accepted;
 };
 auto hand=[&](const Transport<RadioFrame>::Request&){++hsCalls;return blocked?A::Deferred:A::Accepted;};
 auto run=[&](uint32_t now){worker.step(now,100,false,true,shape,app,hand);};
 auto select=[&](TxPriority p){pr.publish(uint8_t(1U<<static_cast<unsigned>(p)));};
 bool admitted=false;
 assert(bus.submit(ordinary,UINT32_MAX-4,false,&initial,20));
 select(TxPriority::Heartbeat);run(UINT32_MAX-4);run(0);
 assert(calls==2&&!bus.poll(admitted));
 // STOP revokes the retained ordinary generation before the next driver call.
 assert(bus.invalidate(1)&&bus.submit(stop,1,true,&replacement,20));
 select(TxPriority::Stop);run(1);
 assert(stopCalls==1&&calls==2&&bus.poll(admitted)&&admitted);
 // A stale ordinary result must not be delivered as the new STOP result.
 assert(!bus.poll(admitted));
 assert(bus.submit(ordinary,2,false,&initial,5));
 select(TxPriority::Heartbeat);run(2);run(6);assert(calls==4&&!bus.poll(admitted));
 run(7);assert(calls==4&&bus.poll(admitted)&&!admitted);
 // Handshake deferral is separate, and revoked handshake is never retried.
 assert(hs.submit(handshake,8,shape));select(TxPriority::Handshake);run(8);run(9);
 assert(hsCalls==2&&!hs.poll(admitted));
 hs.close();run(10);assert(hsCalls==2&&!hs.poll(admitted));
 // A temporary denial can recover without requeue or a new ticket.
 assert(bus.submit(ordinary,11,false,&initial,20));select(TxPriority::Heartbeat);run(11);
 blocked=false;run(12);assert(calls==6&&bus.poll(admitted)&&admitted);
}
static void backupStatusCadence(){
 boat_r30h19::StatusOutbox box;boat_r30h19::BackupStatusCadence cadence;
 RadioDecodedPayload s{};s.kind=RADIO_PAYLOAD_STATUS;s.batteryPercent=80;s.safetyFlags=1;
 assert(cadence.offer(box,s,UINT32_MAX-5,UINT32_MAX-5,true,false,false));
 auto rev=box.revision();
 assert(cadence.offer(box,s,0,0,true,false,false)&&box.revision()!=rev);
 rev=box.revision();
 assert(!cadence.offer(box,s,1,1,true,true,false)&&box.revision()==rev);
 assert(!cadence.offer(box,s,2,2,true,true,true)&&!box.pending());
 assert(!cadence.offer(box,s,3,3,true,false,false));
 s.stopState=1;
 assert(cadence.offer(box,s,4,4,true,false,true));rev=box.revision();
 cadence.suppressNonStopped(box,true);assert(box.pending()&&box.revision()==rev);
 s.batteryPercent=15;
 assert(!cadence.offer(box,s,5,5,true,true,true)&&!box.pending());
 assert(!cadence.offer(box,s,6,6,true,false,false));
 assert(!cadence.offer(box,s,6,3004,true,false,false));
 assert(cadence.offer(box,s,3004,3004,true,false,false));
 assert(!cadence.offer(box,s,3005,3005,false,false,false)&&!box.pending());
 cadence.reset();s.stopState=0;
 assert(!cadence.offer(box,s,3006,3006,true,false,true));
 assert(cadence.offer(box,s,3007,3007,true,false,false));
 cadence.suppressNonStopped(box,true);assert(!box.pending());
 s.batteryPercent=200;
 assert(!cadence.offer(box,s,6008,6008,true,false,false)&&!box.pending());
}
// Exercise real owner cycle, worker, ticket correlation and adaptive policy
// under synthetic timing. No physical RF or actual STOP evidence implied.
static void integratedOwnerCycle(){
 uint8_t key[32]={9};uint16_t seq=0;uint32_t now=0;
 auto verify=[&](const RadioFrame& f){return radio_frameVerify(f,key);};
 auto make=[&](RadioMessageType type,uint16_t id){
  RadioFrame f{};radio_frameInit(f,type,2,1,++seq,id,now,1);f.flags=1;return f;
 };
 Transport<RadioFrame> bus;PhyEvents<RadioFrame> events;AppRadio<RadioFrame> app(bus,events);
 HandshakeLane<RadioFrame> hs;SharedPriority priority;OwnerLink<RadioFrame> owner(app,hs,events,priority);
 SharedPhyService<RadioFrame> worker(bus,hs,priority);
 boat_r30h18::BackupStopAckOutbox acks;
 boat_r30h19::StatusOutbox statuses;boat_r30h19::BackupStatusCadence cadence;
 StopAckService<RadioFrame,boat_r30h18::BackupStopAckOutbox> ack(owner,acks,AckLink::Backup);
 StatusService<RadioFrame> status(owner,statuses);
 OwnerCycle<RadioFrame,boat_r30h18::BackupStopAckOutbox> cycle(owner,ack,status,acks,statuses,500);
 assert(cycle.resetSession(1));
 LoRaAirtimeProfile profile;profile.commissioned=true;profile.hourlyMs=200;profile.reserveMs=50;profile.packetMs=25;
 LoRaAirtimePolicy budget;assert(budget.configure(profile,0,true));
 Driver d;E22AdaptiveAdmission<Driver> admission(d,budget,20,true);
 E22Adapter<RadioFrame,Driver> phy(d,events,admission);
 OrdinaryTx<RadioFrame> ordinaryTx;
 bool heartbeatWanted=true,missionWanted=false;unsigned missionComplete=0;
 auto buildAck=[&](const boat_r30h18::StopAck& a,RadioFrame& f){
  f=make(RADIO_MSG_ACK,a.commandId);
  return radio_encodeAckPayload(a.result,1,a.commandId,a.stopState,a.interlocks,f.payload)&&radio_frameSeal(f,key);
 };
 auto buildStatus=[&](const RadioDecodedPayload& s,RadioFrame& f){
  f=make(RADIO_MSG_TELEMETRY,0);
  return radio_encodeTelemetryStatusPayload(s.boatMode,s.stopState,s.safetyFlags,s.batteryPercent,0,0,f.payload)&&radio_frameSeal(f,key);
 };
 auto result=[&](const Ticket& t,TxState state){
  ordinaryTx.complete(t,state,now,1,[&](const RadioFrame& f,TxState outcome,uint32_t){
   if(f.type==RADIO_MSG_MISSION_FEEDBACK&&outcome==TxState::LocalComplete){missionWanted=false;++missionComplete;}
  });
 };
 auto ordinary=[&](OwnerLink<RadioFrame>& o){
  if(!o.feedbackReady()||ordinaryTx.pending())return;
  if(missionWanted){
   auto f=make(RADIO_MSG_MISSION_FEEDBACK,9);
   assert(missionFeedbackEncode({9,MC_PAUSED,MCTRL_PAUSE,2,MC_ACCEPTED},f.payload)&&radio_frameSeal(f,key));
   assert(ordinaryTx.submit(o,f,now,TxPriority::Mission,12000,verify,true));
  }else if(heartbeatWanted){
   auto f=make(RADIO_MSG_HEARTBEAT,0);
   assert(radio_encodeHeartbeatPayload(1,1,f.payload)&&radio_frameSeal(f,key));
   assert(ordinaryTx.submit(o,f,now,TxPriority::Heartbeat,6000,verify,true));heartbeatWanted=false;
  }
 };
 RadioDecodedPayload sample{};sample.kind=RADIO_PAYLOAD_STATUS;sample.batteryPercent=80;sample.safetyFlags=1;
 auto tick=[&](uint32_t at,bool sampleNow=false){
  now=at;
  if(sampleNow)cadence.offer(statuses,sample,now,now,true,status.pending(),missionWanted);
  cadence.suppressNonStopped(statuses,missionWanted);
  phy.poll(now);
  cycle.step(now,12000,12000,true,true,buildAck,buildStatus,verify,result,[](const Ticket&,HandshakeTx){},ordinary);
  worker.step(now,100,false,true,[](const RadioFrame&){return false;},
   [&](const Transport<RadioFrame>::Request& r){return phy.tryAdmitReply(r,now);},
   [](const Transport<RadioFrame>::Request&){return false;});
 };
 tick(0);assert(d.writes==1&&d.last.type==RADIO_MSG_HEARTBEAT);
 d.state=Driver::O::LocalReady;tick(10);tick(30);assert(!ordinaryTx.pending());
 tick(100,true);assert(status.pending()&&d.writes==1);
 missionWanted=true;tick(110);assert(!status.pending()&&ordinaryTx.pending()&&d.writes==1);
 // STOP preempts a deferred mission, preserving the correlated response.
 assert(acks.offer(22,1,RADIO_ACK_EXECUTED,1,0,200));sample.stopState=1;
 tick(200,true);assert(ack.pending()&&acks.attempts()==1&&d.writes==1);
 for(uint32_t at=300;at<=500;at+=100)tick(at,true);
 tick(530,true);assert(d.writes==2&&d.last.type==RADIO_MSG_ACK&&verify(d.last));
 d.state=Driver::O::LocalReady;tick(540,true);tick(560,true);
 assert(!acks.pending()&&statuses.pending()&&!status.pending()&&d.writes==2);
 for(uint32_t at=660;at<1060;at+=100){tick(at,true);assert(!status.pending()&&d.writes==2);}
 tick(1060,true);assert(d.writes==3&&d.last.type==RADIO_MSG_TELEMETRY&&d.last.payload[2]==1&&verify(d.last));
 d.state=Driver::O::LocalReady;tick(1070,true);tick(1090,true);
 assert(!statuses.pending()&&ordinaryTx.pending()&&missionWanted&&d.writes==3);
 tick(1590,true);assert(d.writes==4&&d.last.type==RADIO_MSG_MISSION_FEEDBACK&&verify(d.last));
 d.state=Driver::O::LocalReady;tick(1600);tick(1620);
 assert(missionComplete==1&&!missionWanted&&!ordinaryTx.pending()&&budget.usedMs(1620)==100);
 // Expired stopped evidence cannot become fresh merely by waiting.
 assert(statuses.offer(sample,2000));tick(2301);assert(!statuses.pending()&&d.writes==4);
 // Epoch zero intentionally disables the application generation.
 assert(!cycle.resetSession(0));assert(!bus.activeGeneration()&&!statuses.pending()&&!acks.pending());
}
int main(){
 workerRetention();
 backupStatusCadence();
 integratedOwnerCycle();
 using T=LoRaAirtimePolicy::Traffic;
 uint8_t key[32]={7};uint16_t seq=0;
 auto verify=[&](const RadioFrame& f){return radio_frameVerify(f,key);};
 auto make=[&](RadioMessageType type,uint16_t id,uint32_t now){RadioFrame f{};radio_frameInit(f,type,2,1,++seq,id,now,1);f.flags=1;return f;};
 Transport<RadioFrame> bus;PhyEvents<RadioFrame> events;AppRadio<RadioFrame> app(bus,events);
 HandshakeLane<RadioFrame> hs;SharedPriority priority;OwnerLink<RadioFrame> owner(app,hs,events,priority);assert(owner.setSession(1));
 SharedPhyService<RadioFrame> worker(bus,hs,priority);
 LoRaAirtimeProfile profile;profile.commissioned=true;profile.hourlyMs=75;profile.reserveMs=50;profile.packetMs=25;
 LoRaAirtimePolicy budget;assert(budget.configure(profile,0,true));Driver d;E22AdaptiveAdmission<Driver> admission(d,budget,20,true);
 E22Adapter<RadioFrame,Driver> phy(d,events,admission);
 auto step=[&](uint32_t now){worker.step(now,100,false,true,[](const RadioFrame&){return false;},[&](const Transport<RadioFrame>::Request& r){return phy.tryAdmitReply(r,now);},[](const Transport<RadioFrame>::Request&){return false;});};
 auto poll=[&](uint32_t now){phy.poll(now);owner.poll(now,12000,12000);};
 Ticket ticket{};TxState state=TxState::Idle;
 auto hb=make(RADIO_MSG_HEARTBEAT,0,0);assert(radio_encodeHeartbeatPayload(1,1,hb.payload)&&radio_frameSeal(hb,key));
 assert(owner.submitFeedback(hb,0,TxPriority::Heartbeat,1000,verify));step(0);assert(d.writes==1&&verify(d.last));
 assert(!phy.admit(owner.currentTicket(),hb,0));d.state=Driver::O::LocalReady;poll(10);assert(!owner.takeApplication(ticket,state));poll(30);
 assert(owner.takeApplication(ticket,state)&&state==TxState::LocalComplete&&budget.usedMs(30)==25);
 boat_r30h18::BackupStopAckOutbox box;StopAckService<RadioFrame,boat_r30h18::BackupStopAckOutbox> ack(owner,box,AckLink::Backup);
 assert(box.offer(22,1,RADIO_ACK_EXECUTED,1,0,530));
 auto buildAck=[&](const boat_r30h18::StopAck& a,RadioFrame& f){f=make(RADIO_MSG_ACK,a.commandId,530);return radio_encodeAckPayload(a.result,1,a.commandId,a.stopState,a.interlocks,f.payload)&&radio_frameSeal(f,key);};
 assert(ack.step(530,true,true,buildAck,verify));step(530);assert(d.writes==2&&d.last.type==RADIO_MSG_ACK&&verify(d.last));
 d.state=Driver::O::LocalReady;poll(540);assert(box.pending()&&!owner.takeApplication(ticket,state));poll(560);
 assert(owner.takeApplication(ticket,state)&&ack.complete(ticket,state)&&!box.pending());
 boat_r30h19::StatusOutbox status;StatusService<RadioFrame> service(owner,status);
 RadioDecodedPayload snapshot{};snapshot.kind=RADIO_PAYLOAD_STATUS;snapshot.stopState=1;snapshot.batteryPercent=80;
 assert(status.offer(snapshot,1060));
 auto buildStatus=[&](const RadioDecodedPayload& v,RadioFrame& f){f=make(RADIO_MSG_TELEMETRY,0,1060);return radio_encodeTelemetryStatusPayload(v.boatMode,v.stopState,v.safetyFlags,v.batteryPercent,0,0,f.payload)&&radio_frameSeal(f,key);};
 assert(service.step(1060,true,true,false,buildStatus,verify));step(1060);assert(d.writes==3&&d.last.type==RADIO_MSG_TELEMETRY&&verify(d.last));
 d.state=Driver::O::LocalReady;poll(1070);poll(1090);assert(owner.takeApplication(ticket,state)&&service.complete(ticket,state)&&!status.pending());assert(budget.usedMs(1090)==75);
 assert(box.offer(23,1,RADIO_ACK_EXECUTED,1,0,1590)&&ack.step(1590,true,true,buildAck,verify));step(1590);poll(1590);
 assert(d.writes==3&&!owner.takeApplication(ticket,state)&&box.pending()&&box.inFlight()&&box.attempts()==1);
 // Repeated budget denials retain the same original ticket and deadline.
 const auto retained=owner.currentTicket();
 for(uint32_t now=1591;now<1700;++now){step(now);poll(now);}
 assert(d.writes==3&&sameTicket(retained,owner.currentTicket())&&box.attempts()==1&&!owner.takeApplication(ticket,state));
 step(13590);poll(13590);
 assert(d.writes==3&&owner.takeApplication(ticket,state)&&state==TxState::Uncertain&&ack.complete(ticket,state));
 ack.reconcile(13590,true,true);assert(!box.pending());
 // Immutable classification cannot promote a malformed or wrong-direction frame.
 Transport<RadioFrame>::Request r{};r.ticket={1,1,1};r.frame=d.last;r.stop=true;r.reply=ReplyClass::Status;T traffic=T::Routine;
 assert(classifyBackupReply<RadioFrame>(r,traffic)&&traffic==T::Emergency);
 r.reply=ReplyClass::Unclassified;assert(!classifyBackupReply<RadioFrame>(r,traffic));r.reply=ReplyClass::StopAck;assert(!classifyBackupReply<RadioFrame>(r,traffic));
 r.reply=ReplyClass::Status;r.stop=false;assert(!classifyBackupReply<RadioFrame>(r,traffic));r.frame.payload[2]=0;r.frame.payload[3]=1;r.frame.payload[4]=80;assert(classifyBackupReply<RadioFrame>(r,traffic)&&traffic==T::Routine);
 r.frame.payload[4]=15;assert(classifyBackupReply<RadioFrame>(r,traffic)&&traffic==T::Mission);r.frame.flags=0;assert(!classifyBackupReply<RadioFrame>(r,traffic));
 r.frame=make(RADIO_MSG_MISSION_FEEDBACK,9,2000);r.reply=ReplyClass::Mission;
 assert(missionFeedbackEncode({9,MC_PAUSED,MCTRL_PAUSE,2,MC_ACCEPTED},r.frame.payload));assert(classifyBackupReply<RadioFrame>(r,traffic)&&traffic==T::Mission);
 assert(missionFeedbackEncode({9,MC_OUTBOUND,MCTRL_START,2,MC_ACCEPTED},r.frame.payload));assert(!classifyBackupReply<RadioFrame>(r,traffic));
 assert(missionFeedbackEncode({9,MC_PAUSED,0,0,MC_STATUS},r.frame.payload));assert(classifyBackupReply<RadioFrame>(r,traffic)&&traffic==T::Routine);
}
