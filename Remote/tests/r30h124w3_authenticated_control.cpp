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
 TxRequest last{};unsigned starts=0;
 static bool send(void* p,const TxRequest& r){auto& d=*static_cast<Driver*>(p);d.last=r;++d.starts;return true;}
};
int main(){
 assert(bindRemoteI2cOwner());RemoteRadioOwner owner;assert(owner.claimWorker(true));
 RadioFrameHandoff h;assert(h.begin());TxGenerationGate gate;RemoteAsyncTx tx(owner,h,gate);
 RemoteAuthenticatedMockApp app(owner,h,tx);RemoteMockPeer peer(mac);BoundedTxService service;Driver d;
 assert(app.initialize(peer.root(),mac));constexpr auto P=PhyLink::Primary;
 auto tick=[&](uint32_t n){return app.tick(n,123,false,true);};
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
 assert(!app.missionNeutral(at));assert(app.requestMissionControl(MCTRL_PAUSE,++at)&&tick(at));poll(at);
 deliver(++at);assert(tick(at)&&app.missionPhase()==MC_PAUSED);
 assert(!app.requestMissionControl(MCTRL_RESUME,at));
 // Neutral sample may be rate-limited after the previous one.
 at+=50;assert(tick(at));poll(at);
 if(d.last.frame.type==RADIO_MSG_HEARTBEAT){deliver(++at);assert(tick(at));poll(at);}
 assert(d.last.frame.type==RADIO_MSG_MISSION_STICKS);deliver(++at);assert(tick(at)&&app.missionNeutral(at));
 assert(app.requestMissionControl(MCTRL_RESUME,++at)&&tick(at));poll(at);deliver(++at);
 assert(tick(at)&&app.missionPhase()==MC_OUTBOUND);
 assert(app.requestMissionControl(MCTRL_CANCEL,++at)&&tick(at));poll(at);deliver(++at);
 assert(tick(at)&&app.missionPhase()==MC_ABORTED&&gate.stopped()&&!app.rearmed());
 assert(!app.requestClear(19,at)&&peer.mockReleases()==0);
 assert(app.requestStop(20,++at)&&tick(at));poll(at);
 assert(d.last.frame.type==RADIO_MSG_ACTION&&d.last.frame.payload[0]==CMD_EMERGENCY_STOP);
 deliver(++at);assert(tick(at));assert(peer.status(status,false,++at));assert(h.primaryRx.offer({status,at,P}));
 assert(tick(at)&&app.confirmed()&&!app.missionAuthorized()&&gate.stopped());
}
