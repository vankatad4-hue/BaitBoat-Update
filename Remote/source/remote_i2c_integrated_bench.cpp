// R30H-114: standalone mock PHY bench, no Wire, RF, LVGL or actuators.
#if defined(FLYTEC_I2C_INTEGRATED_BENCH) && FLYTEC_I2C_INTEGRATED_BENCH
#include <Arduino.h>
#include "dual_phy_worker.h"
#include "remote_i2c_request_scheduler.h"
#include "remote_i2c_phy_service.h"
#include "nrf_phy_adapter.h"
#include "e22_phy_adapter.h"
using namespace flytec_rtos;
using D=RemoteI2cDispatcher;
using S=RemoteI2cRequestScheduler;
constexpr auto P=D::Operation::PollPrimary;
constexpr auto B=D::Operation::PollBackup;
struct MockNrf {
 bool active=true,pending=false,ok=false;
 unsigned polls=0;
 bool ready()const{return active;}
 bool txPending()const{return pending;}
 bool lastTxSuccess()const{return ok;}
 void update(){++polls;if(pending){pending=false;ok=true;}}
 bool receiveFrame(RadioFrame&){return false;}
 bool sendFrame(const RadioFrame&){if(!active||pending)return false;pending=true;ok=false;return true;}
};
struct MockE22 {
 enum class Admission {Accepted,Busy,DutyLimited,Invalid,Unavailable,Uncertain};
 E22TxState state;
 unsigned polls=0;
 bool configured()const{return !state.fault();}
 bool transportFault()const{return state.fault();}
 bool txPending()const{return state.pending();}
 E22TxState::Outcome txOutcome()const{return state.outcome();}
 void update(){++polls;state.poll(millis(),true);}
 bool receiveFrame(RadioFrame&){return false;}
 Admission trySendFrame(const RadioFrame&,uint32_t now){
  if(state.pending())return Admission::Busy;
  state.accepted(now);return Admission::Accepted;
 }
};
static uint32_t clockMs(){return millis();}
static D dispatcher;
static S scheduler(dispatcher);
static RadioFrameHandoff handoff;
static TxGenerationGate gate;
static MockNrf nrf;
static MockE22 lora;
static NrfPhyAdapter<MockNrf> primary(nrf);
static E22PhyAdapter<MockE22> backup(lora);
static RemoteI2cPhyService<decltype(primary),decltype(backup)> owner(dispatcher,primary,backup,handoff,gate);
static DualPhyWorker<S> worker(scheduler,clockMs);
static bool running=false, injected=false, primaryEvidence=false, backupEvidence=false, failed=false;
static uint32_t started=0,backupBefore=0;
static bool submit(PhyLink link,uint32_t ticket){
 TxRequest r{};r.ticket=ticket;r.generation=1;r.link=link;
 return link==PhyLink::Primary?handoff.primaryTx.offer(r):handoff.backupTx.offer(r);
}
void setup(){
 Serial.begin(115200);
 Serial.println("R30H-114 mock PHY / owner-task bench; no hardware drivers");
 if(xPortGetCoreID()!=1||!bindRemoteI2cOwner()||!dispatcher.begin()||
    !handoff.begin()||!gate.transition(1)||!submit(PhyLink::Primary,1)||
    !submit(PhyLink::Backup,2)||!worker.begin(true)){
  Serial.println("FAIL setup");return;
 }
 started=millis();running=true;
}
void loop(){
 if(!running){delay(20);return;}
 const uint32_t now=millis(),elapsed=now-started;
 if(!owner.service(now))failed=true;
 const auto watchdog=worker.supervise();
 if(watchdog==RadioWatchdog::State::Fault)failed=true;
 TxCompletion c;
 if(handoff.takeCompletion(PhyLink::Primary,c)){
  primaryEvidence=c.ticket==1&&c.success&&c.evidence==TxEvidence::NrfAck;
  if(!primaryEvidence)failed=true;
 }
 if(handoff.takeCompletion(PhyLink::Backup,c)){
  backupEvidence=c.ticket==2&&!c.success&&c.evidence==TxEvidence::LocalReady;
  if(!backupEvidence)failed=true;
 }
 if(!injected&&elapsed>=500){
  injected=true;backupBefore=lora.polls;nrf.active=false;
 }
 if(elapsed>=1200||failed){
  const bool pass=!failed&&primaryEvidence&&backupEvidence&&injected&&
    scheduler.status(P)==S::State::Fault&&scheduler.status(B)!=S::State::Fault&&
    lora.polls>backupBefore&&worker.permit();
  worker.stop();running=false;
  Serial.printf("primary=%u backup=%u nrfPolls=%u loraPolls=%u %s\n",
    (unsigned)primaryEvidence,(unsigned)backupEvidence,nrf.polls,lora.polls,pass?"PASS":"FAIL");
 }
 delay(1);
}
#endif
