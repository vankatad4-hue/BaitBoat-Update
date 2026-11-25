#include "remote_async_tx.h"
#include "bounded_tx_service.h"
#include <cassert>
using namespace flytec_rtos;
using A=RemoteAsyncTx;
struct Driver{
 TxRequest last{};unsigned starts=0;bool ready=true;
 static bool send(void* p,const TxRequest& r){auto& d=*static_cast<Driver*>(p);if(!d.ready)return false;d.last=r;++d.starts;return true;}
};
int main(){
 assert(bindRemoteI2cOwner());RemoteRadioOwner owner;assert(owner.claimWorker(true));
 RadioFrameHandoff h;assert(h.begin());TxGenerationGate gate;A tx(owner,h,gate);BoundedTxService service;Driver d;
 constexpr auto P=PhyLink::Primary,B=PhyLink::Backup;
 uint32_t t=0;uint16_t seq=0;A::Result result{};
 assert(tx.beginSession(1));MissionPlan plan{9,1,15,8,300,431000000,270000000,431010000,270010000};
 auto frame=[&](RadioMessageType type,uint16_t id){RadioFrame f{};f.type=type;f.sourceId=1;f.destinationId=2;f.sessionEpoch=1;f.commandId=id;f.sequence=++seq;return f;};
 auto drain=[&](uint32_t now){assert(service.tick(h,P,gate,&Driver::send,&d,now));assert(h.offerCompletion(P,{d.last.ticket,d.last.generation,P,true,TxEvidence::NrfAck}));assert(tx.take(P,now,result));};
 auto f=frame(RADIO_MSG_MISSION_PREPARE,9);assert(missionPlan_encode(plan,MISSION_BEGIN,f.payload));
 assert(tx.submitPrepare(f,1,100,1,t,true)==A::Submit::Queued);drain(2);
 assert(!tx.handoffMission(9,true)&&!tx.rearm(true));
 f=frame(RADIO_MSG_MISSION_PREPARE,9);assert(missionPlan_encode(plan,MISSION_COMMIT,f.payload));
 assert(tx.submitPrepare(f,3,100,3,t,true)==A::Submit::Queued);assert(!tx.handoffMission(9,true));drain(4);
 assert(!tx.handoffMission(10,true)&&!tx.handoffMission(9,false)&&tx.handoffMission(9,true));
 auto clear=frame(RADIO_MSG_ACTION,21);clear.payload[0]=CMD_CLEAR_STOP;
 assert(tx.submitClear(clear,5,100,5,t,true)==A::Submit::Denied);
 assert(tx.submitMissionClear(10,clear,5,100,5,t,true)==A::Submit::Denied);
 assert(tx.submitMissionClear(9,clear,5,100,5,t,true)==A::Submit::Queued);drain(6);
 assert(!tx.confirmMissionClear(9,21,false)&&!tx.confirmMissionClear(9,22,true));
 assert(tx.confirmMissionClear(9,21,true)&&gate.stopped()&&!tx.rearm(true));
 auto control=frame(RADIO_MSG_MISSION_CONTROL,9);assert(missionControlEncode({MCTRL_START,9,1},control.payload));
 assert(tx.submit(P,control,7,100,7,t)==A::Submit::Denied&&tx.submit(B,control,7,100,7,t)==A::Submit::Denied);
 assert(tx.submitMission(control,7,100,7,t,false)==A::Submit::Denied);
 auto sticks=frame(RADIO_MSG_MISSION_STICKS,9);assert(missionStickEncode({9,10,10,false},sticks.payload));
 assert(tx.submitMission(sticks,7,50,7,t,true)==A::Submit::Denied);
 assert(missionStickEncode({9,0,0,true},sticks.payload));
 assert(tx.submitMission(sticks,7,51,7,t,true)==A::Submit::Denied);
 assert(tx.submitMission(sticks,7,50,7,t,true)==A::Submit::Queued);drain(8);
 assert(tx.submitMission(control,9,100,9,t,true)==A::Submit::Queued);
 d.ready=false;assert(service.tick(h,P,gate,&Driver::send,&d,9));const auto before=d.starts;
 assert(tx.requestStop(22));d.ready=true;assert(service.tick(h,P,gate,&Driver::send,&d,10));assert(d.starts==before);
 assert(tx.submitMission(control,10,100,10,t,true)==A::Submit::Denied&&gate.stopped());
}
