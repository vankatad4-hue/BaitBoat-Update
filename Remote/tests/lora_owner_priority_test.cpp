#include "remote_async_control.h"
#include "radio_session_v2.h"
#include "e22_phy_adapter.h"
#include <cassert>
using namespace flytec_rtos;
struct Driver {
 enum class Admission{Accepted,Busy,DutyLimited,Invalid,Unavailable,Uncertain};
 E22TxState state;RadioFrame last{};unsigned writes=0;uint32_t now=0;
 bool configured()const{return !state.fault();}
 bool transportFault()const{return state.fault();}
 bool txPending()const{return state.pending();}
 bool ready()const{return configured()&&!txPending();}
 E22TxState::Outcome txOutcome()const{return state.outcome();}
 void update(){state.poll(now,true);}
 bool receiveFrame(RadioFrame&){return false;}
 Admission trySendFrame(const RadioFrame& f,uint32_t at){++writes;last=f;state.accepted(at);return Admission::Accepted;}
};
int main(){
 using T=LoRaAirtimePolicy::Traffic;
 LoRaAirtimeProfile profile;profile.hourlyMs=50;profile.reserveMs=25;profile.packetMs=25;
 assert(LoRaOwnerTiming::fromProfile(profile,true).urgentGap()==3000);
 profile.commissioned=true;
 assert(LoRaOwnerTiming::fromProfile(profile,false).urgentGap()==3000);
 const auto timing=LoRaOwnerTiming::fromProfile(profile,true);assert(timing.urgentGap()==500);
 assert(bindRemoteI2cOwner());RemoteRadioOwner owner;assert(owner.claimWorker(true));
 RadioFrameHandoff h;assert(h.begin());TxGenerationGate gate;RemoteAsyncTx tx(owner,h,gate);
 RemoteAsyncControl control(owner,tx,timing);assert(control.beginSession(1,0));
 LoRaAirtimePolicy budget;assert(budget.configure(profile,0,true));Driver driver;
 E22AdaptiveAdmission<Driver> admission(driver,budget,20,true);E22PhyAdapter<Driver> phy(driver,admission);
 constexpr auto B=PhyLink::Backup;uint8_t key[32]={7};uint16_t seq=0;
 auto ownerTick=[&](uint32_t now){assert(control.tick(now,false,true,1,seq,RadioSessionCounter::REKEY_AT,key));};
 auto poll=[&](uint32_t now){driver.now=now;assert(phy.tick(h,gate,now));};
 ownerTick(0);assert(tx.pending(B));poll(0);assert(driver.writes==1&&driver.last.type==RADIO_MSG_HEARTBEAT&&radio_frameVerify(driver.last,key));
 poll(34);poll(54);ownerTick(54);assert(!tx.pending(B));
 BoatCommand stop{};stop.type=CMD_EMERGENCY_STOP;stop.commandId=22;stop.mode=MODE_MANUAL;
 assert(control.requestStop(stop,55)&&gate.stopped());
 ownerTick(553);assert(!tx.pending(B));ownerTick(554);assert(tx.pending(B));poll(554);
 assert(driver.writes==2&&driver.last.type==RADIO_MSG_ACTION&&driver.last.commandId==22&&radio_frameVerify(driver.last,key));
 poll(588);poll(608);ownerTick(608);assert(!tx.pending(B));assert(budget.usedMs(608)==50);
 ownerTick(1108);poll(1108);assert(tx.pending(B)&&driver.writes==2); // full duty budget, even STOP waits
 // Neither a raw STOP nor a forged traffic class with wrong wire shape gets reserve.
 TxRequest r{};r.link=B;r.ticket=9;r.generation=tx.generation();r.frame=driver.last;T kind=T::Routine;
 assert(!E22PhyAdapter<Driver>::classifyRemoteOwner(nullptr,r,kind));r.traffic=TxTraffic::EmergencyStop;
 r.frame.type=RADIO_MSG_CONTROL;assert(!E22PhyAdapter<Driver>::classifyRemoteOwner(nullptr,r,kind));
 r.traffic=TxTraffic::BackupMission;r.frame.type=RADIO_MSG_MISSION_CONTROL;r.frame.commandId=9;
 assert(missionControlEncode({MCTRL_START,9,1},r.frame.payload));assert(!E22PhyAdapter<Driver>::classifyRemoteOwner(nullptr,r,kind));
 assert(missionControlEncode({MCTRL_CANCEL,9,1},r.frame.payload));assert(E22PhyAdapter<Driver>::classifyRemoteOwner(nullptr,r,kind)&&kind==T::Mission);
 // Retry spacing can shorten, but the final feedback window remains 3 seconds.
 BackupMissionControl ledger(timing);BackupMissionAuthority safe{true,true,true,true,true,false,false};MissionControlRequest q{};
 assert(ledger.bind(9,1,4,1,true)&&ledger.request(MCTRL_PAUSE,100,safe));
 assert(ledger.due(100,safe,true,q)&&ledger.queued(1,4,1,1,100,250));assert(ledger.complete(1,4,1,1,9,100,250));
 assert(!ledger.due(599,safe,true,q)&&ledger.due(600,safe,true,q)&&ledger.queued(2,4,2,1,600,250));assert(ledger.complete(2,4,2,1,9,600,250));
 assert(ledger.due(1100,safe,true,q)&&ledger.queued(3,4,3,1,1100,250));assert(ledger.complete(3,4,3,1,9,1100,250));
 assert(ledger.maintain(4099,safe)&&!ledger.maintain(4100,safe));
}
