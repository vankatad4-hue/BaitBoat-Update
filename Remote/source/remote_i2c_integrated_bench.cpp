// R30H-114: standalone mock PHY bench, no Wire, RF, LVGL or actuators.
#if defined(FLYTEC_I2C_INTEGRATED_BENCH) && FLYTEC_I2C_INTEGRATED_BENCH
#include <Arduino.h>
#include "dual_phy_worker.h"
#include "remote_worker_runtime.h"
#include "remote_async_tx.h"
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
 uint32_t lastAdmissionUs=0;
 bool configured()const{return !state.fault();}
 bool transportFault()const{return state.fault();}
 bool txPending()const{return state.pending();}
 E22TxState::Outcome txOutcome()const{return state.outcome();}
 void update(){++polls;state.poll(millis(),true);}
 bool receiveFrame(RadioFrame&){return false;}
 Admission trySendFrame(const RadioFrame&,uint32_t now){
  if(state.pending())return Admission::Busy;
  lastAdmissionUs=micros();state.accepted(now);return Admission::Accepted;
 }
};
static uint32_t clockMs(){return millis();}
static uint32_t clockUs(){return micros();}
static D dispatcher(clockUs);
static S scheduler(dispatcher);
static RadioFrameHandoff handoff;
static TxGenerationGate gate;
static RemoteRadioOwner radioOwner;
static RemoteAsyncTx asyncTx(radioOwner,handoff,gate);
static MockNrf nrf;
static MockE22 lora;
static NrfPhyAdapter<MockNrf> primary(nrf);
static E22PhyAdapter<MockE22> backup(lora);
static RemoteI2cPhyService<decltype(primary),decltype(backup)> owner(dispatcher,primary,backup,handoff,gate);
static TimedPoller<S> timedScheduler(scheduler,clockUs);
static DualPhyWorker<decltype(timedScheduler)> worker(timedScheduler,clockMs);
static RemoteWorkerRuntime<decltype(worker),decltype(owner)> runtime(worker,owner);
static PollTiming ownerTiming;
static uint32_t stopQueuedUs=0,stopAdmissionUs=0,stopLocalUs=0,faultInjectedMs=0,faultObservedMs=0;
static bool stopLocal=false,faultObserved=false;
static bool running=false, injected=false, primaryEvidence=false, backupEvidence=false, failed=false;
static uint32_t started=0,backupBefore=0;
static uint32_t primaryTicket=0,backupTicket=0,stopTicket=0;
static bool submit(PhyLink link,uint16_t id,bool stop,uint32_t& ticket){
 // Mock frames only: the live application must provide sealed frames.
 RadioFrame f{};f.type=RADIO_MSG_ACTION;f.commandId=id;f.sessionEpoch=1;
 f.flags=link==PhyLink::Backup?RADIO_FLAG_BACKUP_LINK:0;
 f.payload[0]=stop?CMD_EMERGENCY_STOP:CMD_SET_MODE;
 const uint32_t now=millis();
 return asyncTx.submit(link,f,now,stop?700U:400U,now,ticket)==RemoteAsyncTx::Submit::Queued;
}
void setup(){
 Serial.begin(115200);
 Serial.println("R30H-114 mock PHY / owner-task bench; no hardware drivers");
 if(xPortGetCoreID()!=1||!bindRemoteI2cOwner()||!dispatcher.begin()||
    !handoff.begin()||!radioOwner.claimWorker(true)||!asyncTx.beginSession(1)||
    !asyncTx.rearm(true)||!submit(PhyLink::Primary,1,false,primaryTicket)||
    !submit(PhyLink::Backup,2,false,backupTicket)||!runtime.begin(true,true)){
  Serial.println("FAIL setup");return;
 }
 started=millis();running=true;
}
void loop(){
 if(!running){delay(20);return;}
 const uint32_t now=millis(),elapsed=now-started;
 ownerTiming.begin(micros());
 if(!runtime.tick(now))failed=true;
 ownerTiming.end(micros());
 RemoteAsyncTx::Result c;
 if(asyncTx.take(PhyLink::Primary,millis(),c)){
  primaryEvidence=c.ticket==primaryTicket&&c.commandId==1&&!c.stop&&c.outcome==RemoteAsyncTx::Outcome::NrfAck;
  if(!primaryEvidence)failed=true;
 }
 if(asyncTx.take(PhyLink::Backup,millis(),c)){
  if(c.ticket==backupTicket&&c.commandId==2&&!c.stop&&c.outcome==RemoteAsyncTx::Outcome::LocalReady)backupEvidence=true;
  else if(c.ticket==stopTicket&&c.commandId==3&&c.stop&&c.outcome==RemoteAsyncTx::Outcome::LocalReady){
   stopLocal=true;stopAdmissionUs=lora.lastAdmissionUs-stopQueuedUs;stopLocalUs=micros()-stopQueuedUs;
  }else failed=true;
 }
 if(!injected&&elapsed>=500){
  injected=true;backupBefore=lora.polls;nrf.active=false;faultInjectedMs=now;
  stopQueuedUs=micros();if(!asyncTx.requestStop(3)||!submit(PhyLink::Backup,3,true,stopTicket))failed=true;
 }
 if(injected&&!faultObserved&&scheduler.status(P)==S::State::Fault){faultObserved=true;faultObservedMs=now;}
 if(elapsed>=1200||failed){
  const bool pass=!failed&&primaryEvidence&&backupEvidence&&stopLocal&&faultObserved&&injected&&
    scheduler.status(P)==S::State::Fault&&scheduler.status(B)!=S::State::Fault&&
    lora.polls>backupBefore&&runtime.permit();
  asyncTx.invalidateSession();runtime.requestStop();worker.stop();radioOwner.retire();running=false;
  const auto& t=timedScheduler.timing();
  const auto pm=dispatcher.metrics(P),bm=dispatcher.metrics(B);
  Serial.printf("core0 callbacks=%lu mean_us=%lu max_us=%lu gap_max_us=%lu\\n",(unsigned long)t.duration.count,(unsigned long)t.duration.mean(),(unsigned long)t.duration.max,(unsigned long)t.gap.max);
  Serial.printf("core1 service mean_us=%lu max_us=%lu gap_max_us=%lu\\n",(unsigned long)ownerTiming.duration.mean(),(unsigned long)ownerTiming.duration.max,(unsigned long)ownerTiming.gap.max);
  Serial.printf("primary queue_max_ms=%lu phy_max_us=%lu phy_gap_max_us=%lu backup queue_max_ms=%lu phy_max_us=%lu phy_gap_max_us=%lu\\n",(unsigned long)pm.queueAgeMs.max,(unsigned long)pm.executionUs.duration.max,(unsigned long)pm.executionUs.gap.max,(unsigned long)bm.queueAgeMs.max,(unsigned long)bm.executionUs.duration.max,(unsigned long)bm.executionUs.gap.max);
  Serial.printf("simulated critical_admission_us=%lu local_completion_us=%lu primary_fault_detection_ms=%lu observed=%u\\n",(unsigned long)stopAdmissionUs,(unsigned long)stopLocalUs,(unsigned long)(faultObserved?faultObservedMs-faultInjectedMs:0),(unsigned)faultObserved);
  Serial.printf("primary=%u backup=%u nrfPolls=%u loraPolls=%u %s\n",
    (unsigned)primaryEvidence,(unsigned)backupEvidence,nrf.polls,lora.polls,pass?"PASS":"FAIL");
 }
 delay(1);
}
#endif
