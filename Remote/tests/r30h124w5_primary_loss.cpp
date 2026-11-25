#include "remote_authenticated_mock_app.h"
#include "remote_mock_peer.h"
#include "bounded_tx_service.h"
#include <mbedtls/md.h>
#include <cassert>
using namespace flytec_rtos;
static bool mac(const uint8_t k[32],const uint8_t* d,size_t n,uint8_t out[32]){
 return mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),k,32,d,n,out)==0;
}
struct Driver{
 TxRequest last{};unsigned starts=0;bool ready=true;
 static bool send(void* p,const TxRequest& r){auto& d=*static_cast<Driver*>(p);if(!d.ready)return false;d.last=r;++d.starts;return true;}
};
int main(){
 assert(bindRemoteI2cOwner());RemoteRadioOwner owner;assert(owner.claimWorker(true));
 RadioFrameHandoff h;assert(h.begin());TxGenerationGate gate;RemoteAsyncTx tx(owner,h,gate);
 RemoteAuthenticatedMockApp app(owner,h,tx);RemoteMockPeer peer(mac);BoundedTxService service;Driver d,b;
 assert(app.initialize(peer.root(),mac));constexpr auto P=PhyLink::Primary,B=PhyLink::Backup;
 auto tick=[&](uint32_t n,bool fault=false){return app.tick(n,123,fault,false);};
 auto poll=[&](uint32_t n){assert(service.tick(h,P,gate,&Driver::send,&d,n));};
 auto deliver=[&](uint32_t n){RadioFrame f{};assert(peer.exchange(d.last.frame,f,false,n));assert(h.primaryRx.offer({f,n,P}));assert(h.offerCompletion(P,{d.last.ticket,d.last.generation,P,true,TxEvidence::NrfAck}));};
 assert(tick(1));poll(1);deliver(2);assert(tick(2));poll(2);deliver(3);assert(tick(3)&&app.established());
 assert(app.requestStop(17,4));assert(tick(4));poll(4);if(d.last.frame.commandId!=17)poll(4);
 assert(d.last.frame.commandId==17);deliver(5);assert(tick(5));RadioFrame status{};
 assert(peer.status(status,false,6));assert(h.primaryRx.offer({status,6,P}));assert(tick(6)&&app.confirmed());
 MissionPlan plan{9,1,15,8,300,431000000,270000000,431010000,270010000};
 assert(!app.requestMission(plan,6));app.mockInput(true,true);assert(app.requestMission(plan,6));
 assert(!app.requestClear(18,6));
 uint32_t at=7;assert(tick(at));
 for(unsigned op=1;op<=6;++op){
  poll(at);assert(d.last.frame.type==RADIO_MSG_MISSION_PREPARE&&d.last.frame.payload[0]==op&&gate.stopped());
  assert(!tx.rearm(true));deliver(++at);assert(tick(at));
 }
 MissionPlan got{};assert(app.missionPrepared(at)&&peer.prepared(got,at)&&missionPlan_digest(got)==missionPlan_digest(plan));
 assert(!app.requestClear(18,at)&&!app.rearmed()&&gate.stopped());
 assert(!app.acceptPreparedHandoff(at));
 RadioFrame feedback{};assert(peer.missionStatus(feedback,++at));assert(h.primaryRx.offer({feedback,at,P}));assert(tick(at));
 assert(app.acceptPreparedHandoff(at)&&app.missionHandedOff()&&gate.stopped());
 assert(app.requestMissionClear(18,++at));poll(at);
 assert(d.last.frame.payload[0]==CMD_CLEAR_STOP);deliver(++at);assert(tick(at)&&!app.missionAuthorized());
 assert(peer.status(status,false,++at));assert(h.primaryRx.offer({status,at,P}));assert(tick(at)&&app.missionAuthorized()&&gate.stopped());
 assert(!app.requestMissionControl(MCTRL_START,at));
 poll(at);assert(d.last.frame.type==RADIO_MSG_MISSION_STICKS);
 // Lose authenticated receipt but retain local transport success.
 RadioFrame lost{};assert(peer.exchange(d.last.frame,lost,false,++at));
 assert(h.offerCompletion(P,{d.last.ticket,d.last.generation,P,true,TxEvidence::NrfAck}));
 assert(tick(at)&&!app.missionNeutral(at)&&!app.requestMissionControl(MCTRL_START,at));
 at+=50;assert(tick(at));poll(at);assert(d.last.frame.type==RADIO_MSG_MISSION_STICKS);
 deliver(++at);assert(tick(at)&&app.missionNeutral(at));
 assert(app.requestMissionControl(MCTRL_START,++at)&&tick(at));poll(at);
 assert(d.last.frame.type==RADIO_MSG_MISSION_CONTROL);deliver(++at);assert(tick(at)&&app.missionPhase()==MC_OUTBOUND);
 // PAUSE is staged and removed from the queue by the busy worker,
 // but has never been admitted to the PHY. Lose primary before admission.
 assert(app.requestMissionControl(MCTRL_PAUSE,++at)&&tick(at));
 d.ready=false;poll(at);const auto starts=d.starts,generation=tx.generation();
 assert(tick(++at,true)&&app.established()&&app.primaryFault()&&!app.backupFault());
 assert(tx.generation()!=generation&&gate.stopped()&&!app.missionAuthorized()&&app.missionLocked());
 assert(app.missionOutcomeUnknown()&&!app.missionNeutral(at)&&!app.confirmed()&&!app.backupConfirmed());
 d.ready=true;poll(++at);assert(d.starts==starts);
 // A lost primary is not a new STOP instruction. Last confirmed phase
 // is historical; peer can remain autonomous with backup still available.
 RadioFrame remotePhase{};assert(peer.missionStatus(remotePhase,at));
 MissionControlFeedback observed{};assert(missionFeedbackDecode(remotePhase.payload,observed)&&observed.phase==MC_OUTBOUND);
 assert(peer.lastStop()==17&&peer.mockReleases()==0);
 assert(!app.requestMissionControl(MCTRL_CANCEL,at)&&!app.requestMissionControl(MCTRL_RESUME,at));
 assert(!app.requestClear(19,at)&&!app.requestMissionClear(19,at));
 BoatCommand ordinary{};ordinary.type=CMD_MANUAL_MOTORS;ordinary.commandId=19;ordinary.createdAtMs=at;ordinary.leftMotor=100;
 assert(!app.requestOrdinary(ordinary,at) );
 // Drain the old backup root/heartbeat generation before fresh STOP.
 assert(service.tick(h,B,gate,&Driver::send,&b,at));
 assert(b.starts==0); // stale queued traffic cannot reach RF
 // W8.3b backup mission scope cannot bypass fresh authenticated evidence.
 assert(app.backupMissionBound()&&app.backupMissionHighWater()==2);
 assert(!app.requestBackupMissionControl(MCTRL_PAUSE,at));
 peer.mockPrimaryUnavailable(true);
 RadioFrame phase{};
 assert(peer.status(status,true,++at)&&h.backupRx.offer({status,at,B}));
 assert(peer.missionStatus(phase,++at,true)&&h.backupRx.offer({phase,at,B}));
 assert(tick(at,true)&&app.backupStatusFresh(at));
 assert(!app.requestBackupMissionControl(MCTRL_START,at)&&!app.requestBackupMissionControl(MCTRL_RESUME,at));
 assert(app.requestBackupMissionControl(MCTRL_PAUSE,at)&&app.backupMissionHighWater()==3);
 at+=3000;assert(tick(at,true)&&service.tick(h,B,gate,&Driver::send,&b,at));
 MissionControlRequest q{};assert(missionControlDecode(b.last.frame.payload,q)&&q.op==MCTRL_PAUSE&&q.request==3);
 RadioFrame reply{};assert(peer.exchange(b.last.frame,reply,true,++at));
 assert(h.backupRx.offer({reply,at,B})&&h.offerCompletion(B,{b.last.ticket,b.last.generation,B,false,TxEvidence::LocalReady}));
 assert(tick(at,true)&&app.backupMissionState()==BackupMissionControl::State::Accepted&&gate.stopped());
 assert(peer.status(status,true,++at)&&h.backupRx.offer({status,at,B})&&tick(at,true));
 assert(app.requestBackupMissionControl(MCTRL_CANCEL,at)&&app.backupMissionHighWater()==4);
 at+=3000;assert(tick(at,true)&&service.tick(h,B,gate,&Driver::send,&b,at));
 assert(missionControlDecode(b.last.frame.payload,q)&&q.op==MCTRL_CANCEL&&q.request==4);
 assert(peer.exchange(b.last.frame,reply,true,++at));
 assert(h.backupRx.offer({reply,at,B})&&h.offerCompletion(B,{b.last.ticket,b.last.generation,B,false,TxEvidence::LocalReady}));
 assert(tick(at,true)&&app.backupMissionState()==BackupMissionControl::State::Accepted&&!app.missionAuthorized());
 assert(!app.requestStop(17,at));
 assert(app.requestStop(20,at)&&tick(at,true));
 at+=3000;assert(tick(at,true));assert(service.tick(h,B,gate,&Driver::send,&b,at));
 assert(b.last.frame.type==RADIO_MSG_ACTION&&b.last.frame.commandId==20&&b.last.frame.payload[0]==CMD_EMERGENCY_STOP);
 RadioFrame ack{};assert(peer.exchange(b.last.frame,ack,true,++at));
 assert(h.backupRx.offer({ack,at,B}));assert(h.offerCompletion(B,{b.last.ticket,b.last.generation,B,false,TxEvidence::LocalReady}));
 assert(tick(at,true)&&!app.confirmed()&&!app.backupConfirmed());
 assert(peer.status(status,true,++at)&&h.backupRx.offer({status,at,B})&&tick(at,true));
 assert(app.backupConfirmed()&&!app.confirmed()&&!app.missionAuthorized()&&gate.stopped());
 assert(peer.lastStop()==20&&peer.mockReleases()==0&&!app.rearmed());
 // W8.4b: recover the primary transport only, preserving STOP and the
 // old motion interlock. A local nRF ACK alone is never recovery evidence.
 peer.mockPrimaryUnavailable(false);
 at+=1;assert(tick(at,false)&&app.primaryFault());
 poll(at);if(d.last.frame.type!=RADIO_MSG_LINK_PROBE)poll(at);
 assert(d.last.frame.type==RADIO_MSG_LINK_PROBE);
 RadioFrame echo{};assert(peer.exchange(d.last.frame,echo,false,++at));
 assert(h.offerCompletion(P,{d.last.ticket,d.last.generation,P,true,TxEvidence::NrfAck}));
 assert(tick(at,false)&&app.primaryFault()); // drop first authenticated echo
 at+=500;assert(tick(at,false));poll(at);
 assert(d.last.frame.type==RADIO_MSG_LINK_PROBE);
 for(unsigned i=0;i<3;++i){
  deliver(++at);assert(tick(at,false));
  if(i<2){assert(app.primaryFault());at+=499;assert(tick(at,false));poll(at);assert(d.last.frame.type==RADIO_MSG_LINK_PROBE);}
 }
 assert(!app.primaryFault()&&!app.rearmed()&&!app.missionAuthorized()&&!app.backupMissionBound()&&gate.stopped());
 assert(peer.lastStop()==20&&peer.mockReleases()==0&&!app.requestOrdinary(ordinary,at));
 assert(!app.tick(++at,123,true,true)&&!app.established()&&gate.stopped());
}
