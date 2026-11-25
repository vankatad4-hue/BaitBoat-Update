// R30H-124J isolated authenticated mock: no Wire, display, RF or actuators.
#if defined(FLYTEC_AUTHENTICATED_MOCK_BENCH) && FLYTEC_AUTHENTICATED_MOCK_BENCH
#include <Arduino.h>
#include "radio_session_platform.h"
#include "remote_authenticated_mock_app.h"
#include "remote_mock_peer.h"
#include "remote_exclusive_runtime.h"
#include "remote_i2c_request_scheduler.h"
#include "remote_i2c_phy_service.h"
#include "dual_phy_worker.h"
#include "nrf_phy_adapter.h"
#include "e22_phy_adapter.h"
using namespace flytec_rtos;
using D=RemoteI2cDispatcher;using S=RemoteI2cRequestScheduler;
constexpr auto P=D::Operation::PollPrimary,B=D::Operation::PollBackup;
static RemoteMockPeer peer(radioPlatformHmac);
struct Inbox{
 RadioFrame frame{};bool pending=false,statusDue=false;
 void send(const RadioFrame& f,bool backup){
  pending=peer.exchange(f,frame,backup,millis());
  statusDue=pending&&frame.type==RADIO_MSG_ACK;
 }
 bool receive(RadioFrame& out,bool backup){
  if(!pending&&statusDue){statusDue=false;pending=peer.status(frame,backup,millis());return false;}
  if(!pending)return false;
  out=frame;pending=false;return true;
 }
};
struct MockNrf{
 Inbox inbox;bool active=true,pending=false;
 bool ready()const{return active;}
 bool txPending()const{return pending;}
 bool lastTxSuccess()const{return active;}
 void update(){pending=false;}
 bool receiveFrame(RadioFrame& f){return inbox.receive(f,false);}
 bool sendFrame(const RadioFrame& f){if(!active||pending)return false;pending=true;inbox.send(f,false);return true;}
};
struct MockE22{
 enum class Admission{Accepted,Busy,DutyLimited,Invalid,Unavailable,Uncertain};
 Inbox inbox;E22TxState state;bool active=true;
 bool configured()const{return active&&!state.fault();}
 bool transportFault()const{return !active||state.fault();}
 bool txPending()const{return state.pending();}
 bool ready()const{return configured()&&!txPending();}
 E22TxState::Outcome txOutcome()const{return state.outcome();}
 void update(){state.poll(millis(),true);}
 bool receiveFrame(RadioFrame& f){return !state.pending()&&inbox.receive(f,true);}
 Admission trySendFrame(const RadioFrame& f,uint32_t now){
  if(!active)return Admission::Unavailable;
  if(state.pending())return Admission::Busy;
  state.accepted(now);inbox.send(f,true);return Admission::Accepted;
 }
};
static uint32_t clockMs(){return millis();}
static uint32_t clockUs(){return micros();}
static D dispatcher(clockUs);static S scheduler(dispatcher);
static RadioFrameHandoff handoff;static TxGenerationGate gate;static RemoteRadioOwner owner;
static RemoteAsyncTx tx(owner,handoff,gate);
// Synthetic RF profile only; never copied to a physical worker configuration.
static const LoRaAirtimeProfile mockProfile=[](){LoRaAirtimeProfile p;p.commissioned=true;return p;}();
static LoRaAirtimePolicy mockBudget;
static RemoteAuthenticatedMockApp app(owner,handoff,tx,LoRaOwnerTiming::fromProfile(mockProfile,true));
static MockNrf nrf;static MockE22 lora;
static NrfPhyAdapter<MockNrf> primary(nrf);
static E22AdaptiveAdmission<MockE22> admission(lora,mockBudget,20,true);
static E22PhyAdapter<MockE22> backup(lora,admission);
static RemoteI2cPhyService<decltype(primary),decltype(backup)> phy(dispatcher,primary,backup,handoff,gate);
static DualPhyWorker<S> worker(scheduler,clockMs);
static RemoteWorkerRuntime<decltype(worker),decltype(phy)> runtime(worker,phy);
static RemoteExclusiveRuntime<decltype(runtime)> exclusive(owner,handoff,runtime);
static bool init(void*){return mockBudget.configure(mockProfile,millis(),true)&&dispatcher.begin()&&app.initialize(peer.root(),radioPlatformHmac);}
static void revoke(void*){app.retire(millis());}
static bool running=false,first=false,primaryConfirmed=false,second=false,backupSeen=false;
static uint32_t start=0;
static void finish(bool pass){
 exclusive.requestStop();worker.stop();running=false;
 Serial.printf("R30H-124J primary=%u backup=%u retired=%u %s\n",
  unsigned(primaryConfirmed),unsigned(backupSeen),unsigned(!owner.worker()&&exclusive.exited()),pass&&!owner.worker()&&exclusive.exited()?"PASS":"FAIL");
}
void setup(){
 Serial.begin(115200);
 if(xPortGetCoreID()!=1||!bindRemoteI2cOwner()||!exclusive.begin(true,&init,&revoke,nullptr)){Serial.println("FAIL startup");return;}
 start=millis();running=true;
}
void loop(){
 if(!running){delay(20);return;}
 const uint32_t now=millis();
 const bool pFault=scheduler.status(P)==S::State::Fault,bFault=scheduler.status(B)==S::State::Fault;
 if(!exclusive.tick(now,pFault,bFault)){
  finish(backupSeen&&pFault&&bFault);return;
 }
 if(exclusive.permit()&&!app.tick(now,radioPlatformNonce(),pFault,bFault)){finish(false);return;}
 if(app.established()&&!first){first=app.requestStop(17,now);if(!first){finish(false);return;}}
 if(first&&!primaryConfirmed&&app.confirmed()){primaryConfirmed=true;nrf.active=false;}
 if(primaryConfirmed&&pFault&&!second){second=app.requestStop(18,now);if(!second){finish(false);return;}}
 if(second&&app.backupConfirmed()){
  if(app.confirmed()||peer.lastStop()!=18){finish(false);return;}
  backupSeen=true;lora.active=false;
 }
 if(uint32_t(now-start)>=16000U){finish(false);return;}
 delay(1);
}
#endif
