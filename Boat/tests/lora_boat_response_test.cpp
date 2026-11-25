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
int main(){
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
 auto step=[&](uint32_t now){worker.step(now,100,false,true,[](const RadioFrame&){return false;},[&](const Transport<RadioFrame>::Request& r){return phy.admitReply(r,now);},[](const Transport<RadioFrame>::Request&){return false;});};
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
 assert(d.writes==3&&owner.takeApplication(ticket,state)&&state==TxState::Uncertain&&ack.complete(ticket,state)&&box.pending());
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
