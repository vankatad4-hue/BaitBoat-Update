#include "remote_async_control.h"
#include "radio_session_v2.h"
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
 RemoteAsyncControl ctl(owner,tx);assert(ctl.beginSession(1,0));MissionPlan plan{9,1,15,8,300,431000000,270000000,431010000,270010000};
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
 // Reject transfer of a wrong transaction or without primary loss evidence.
 assert(!ctl.transferMissionToBackup(10,7,true)&&!ctl.transferMissionToBackup(9,7,false));
 auto old=frame(RADIO_MSG_MISSION_CONTROL,9);assert(missionControlEncode({MCTRL_START,9,1},old.payload));
 assert(tx.submitMission(old,7,100,7,t,true)==A::Submit::Queued);
 d.ready=false;assert(service.tick(h,P,gate,&Driver::send,&d,7));const auto starts=d.starts,gen=tx.generation();
 assert(ctl.transferMissionToBackup(9,8,true)&&tx.generation()!=gen&&gate.stopped());
 d.ready=true;assert(service.tick(h,P,gate,&Driver::send,&d,8)&&d.starts==starts);
 assert(!tx.rearm(true)&&tx.submitMission(old,8,100,8,t,true)==A::Submit::Denied);
 assert(tx.submitClear(clear,8,100,8,t,true)==A::Submit::Denied);
 auto f2=frame(RADIO_MSG_MISSION_CONTROL,9);f2.flags=RADIO_FLAG_BACKUP_LINK;
 assert(missionControlEncode({MCTRL_START,9,2},f2.payload));
 assert(tx.submitBackupMission(f2,8,250,8,t,true)==A::Submit::Denied);
 assert(missionControlEncode({MCTRL_PAUSE,10,2},f2.payload));
 assert(tx.submitBackupMission(f2,8,250,8,t,true)==A::Submit::Denied);
 assert(missionControlEncode({MCTRL_PAUSE,9,2},f2.payload));
 assert(tx.submit(B,f2,8,250,8,t)==A::Submit::Denied);
 assert(tx.submitBackupMission(f2,8,251,8,t,true)==A::Submit::Denied);
 assert(tx.submitBackupMission(f2,8,250,8,t,false)==A::Submit::Denied);
 BackupMissionControl ledger;BackupMissionAuthority a{true,true,true,true,true,false,false};
 assert(ledger.bind(9,1,tx.generation(),1,true)&&ledger.request(MCTRL_PAUSE,8,a));
 uint8_t key[32]={7};ctl.deferBackup(8);
 assert(ctl.serviceBackupMission(ledger,a,3007,seq,RadioSessionCounter::REKEY_AT,key)&&!tx.pending(B));
 assert(ctl.serviceBackupMission(ledger,a,3008,seq,RadioSessionCounter::REKEY_AT,key)&&tx.pending(B));
 Driver backup;assert(service.tick(h,B,gate,&Driver::send,&backup,3008)&&backup.starts==1);
 assert(backup.last.frame.flags==RADIO_FLAG_BACKUP_LINK&&radio_frameVerify(backup.last.frame,key));
 MissionControlRequest q{};assert(missionControlDecode(backup.last.frame.payload,q)&&q.op==MCTRL_PAUSE&&q.request==2);
 // A valid LoRa completion may arrive after the 250 ms admission limit.
 assert(backup.last.lifetimeMs==250U);
 assert(!tx.take(B,3258,result)&&tx.pending(B)&&ledger.maintain(3258,a));
 MissionControlFeedback ack{9,MC_PAUSED,MCTRL_PAUSE,2,MC_ACCEPTED};
 // Authenticated feedback may precede local completion; no new request yet.
 assert(ledger.observe(ack,1,tx.generation(),3300,3300,true,a));
 assert(!ledger.request(MCTRL_CANCEL,3301,a)&&ledger.inFlight());
 assert(h.offerCompletion(B,{backup.last.ticket,backup.last.generation,B,false,TxEvidence::LocalReady}));
 assert(tx.take(B,5008,result)&&result.outcome==A::Outcome::LocalReady);
 assert(ctl.observeBackupMissionCompletion(ledger,result,5008));
 assert(ledger.state()==BackupMissionControl::State::Accepted&&!ledger.inFlight());
 assert(ledger.request(MCTRL_CANCEL,5009,a));
 assert(ctl.serviceBackupMission(ledger,a,8007,seq,RadioSessionCounter::REKEY_AT,key)&&!tx.pending(B));
 assert(ctl.serviceBackupMission(ledger,a,8008,seq,RadioSessionCounter::REKEY_AT,key)&&tx.pending(B));
 backup.ready=false;assert(service.tick(h,B,gate,&Driver::send,&backup,8008));
 const auto before=backup.starts;
 BoatCommand stop{};stop.commandId=22;stop.type=CMD_EMERGENCY_STOP;stop.mode=MODE_MANUAL;
 assert(ctl.requestStop(stop,8009));ledger.transportRevoked();
 backup.ready=true;assert(service.tick(h,B,gate,&Driver::send,&backup,8010)&&backup.starts==before);
 assert(tx.submitBackupMission(f2,8010,250,8010,t,true)==A::Submit::Denied);
 assert(ctl.tick(11009,false,true,1,seq,RadioSessionCounter::REKEY_AT,key,true));
 assert(service.tick(h,B,gate,&Driver::send,&backup,11009)&&backup.starts==before+1);
 assert(backup.last.frame.type==RADIO_MSG_ACTION&&backup.last.frame.payload[0]==CMD_EMERGENCY_STOP&&backup.last.frame.commandId==22);
}
