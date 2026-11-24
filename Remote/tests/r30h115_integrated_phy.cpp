#include "remote_i2c_request_scheduler.h"
#include "remote_i2c_phy_service.h"
#include "nrf_phy_adapter.h"
#include "e22_phy_adapter.h"
#include <cassert>
#include <thread>
using namespace flytec_rtos;
using D=RemoteI2cDispatcher;
using S=RemoteI2cRequestScheduler;
constexpr auto P=D::Operation::PollPrimary;
constexpr auto B=D::Operation::PollBackup;
struct MockNrf {
 bool active=true,pending=false,ok=false;
 unsigned polls=0,sends=0;
 bool ready()const{return active;}
 bool txPending()const{return pending;}
 bool lastTxSuccess()const{return ok;}
 void update(){assert(remoteI2cAllowed());++polls;if(pending){pending=false;ok=active;}}
 bool receiveFrame(RadioFrame&){assert(remoteI2cAllowed());return false;}
 bool sendFrame(const RadioFrame&){assert(remoteI2cAllowed());++sends;pending=true;ok=false;return true;}
};
struct MockE22 {
 enum class Admission{Accepted,Busy,DutyLimited,Invalid,Unavailable,Uncertain};
 E22TxState state;
 uint32_t now=0;
 unsigned polls=0,sends=0;
 bool configured()const{return !state.fault();}
 bool transportFault()const{return state.fault();}
 bool txPending()const{return state.pending();}
 E22TxState::Outcome txOutcome()const{return state.outcome();}
 void update(){assert(remoteI2cAllowed());++polls;state.poll(now,true);}
 bool receiveFrame(RadioFrame&){assert(remoteI2cAllowed());return false;}
 Admission trySendFrame(const RadioFrame&,uint32_t t){
  assert(remoteI2cAllowed());++sends;state.accepted(t);return Admission::Accepted;
 }
};
int main(){
 assert(bindRemoteI2cOwner());
 D d;assert(d.begin());S s(d);RadioFrameHandoff h;assert(h.begin());
 TxGenerationGate g;assert(g.transition(1));
 MockNrf n;MockE22 e;
 NrfPhyAdapter<MockNrf> p(n);E22PhyAdapter<MockE22> b(e);
 RemoteI2cPhyService<decltype(p),decltype(b)> owner(d,p,b,h,g);
 auto offer=[&](PhyLink l,uint32_t ticket,uint32_t generation,bool critical){
  TxRequest r{};r.link=l;r.ticket=ticket;r.generation=generation;
  if(l==PhyLink::Primary)return critical?h.primaryCritical.offer(r):h.primaryTx.offer(r);
  return critical?h.backupCritical.offer(r):h.backupTx.offer(r);
 };
 auto run=[&](uint32_t start,uint32_t end){
  for(uint32_t t=start;t<end;++t){
   std::thread producer([&]{assert(!remoteI2cAllowed());s.tick(t);assert(!owner.service(t));});
   producer.join();e.now=t;assert(owner.service(t));
  }
 };
 assert(offer(PhyLink::Primary,1,1,false));assert(offer(PhyLink::Backup,2,1,false));
 run(0,80);
 TxCompletion c{};
 assert(h.takeCompletion(PhyLink::Primary,c)&&c.ticket==1&&c.generation==1&&c.success&&c.evidence==TxEvidence::NrfAck);
 assert(h.takeCompletion(PhyLink::Backup,c)&&c.ticket==2&&c.generation==1&&!c.success&&c.evidence==TxEvidence::LocalReady);
 assert(n.sends==1&&e.sends==1);
 n.active=false;const unsigned prior=e.polls;
 run(80,100);
 assert(s.status(P)==S::State::Fault&&s.status(B)!=S::State::Fault&&e.polls>prior);
 assert(g.latchStop(2));
 assert(offer(PhyLink::Backup,3,1,false));
 assert(offer(PhyLink::Backup,4,2,false));
 assert(offer(PhyLink::Backup,5,2,true));
 run(100,200);
 assert(h.takeCompletion(PhyLink::Backup,c)&&c.ticket==5&&!c.success&&c.evidence==TxEvidence::LocalReady);
 assert(h.takeCompletion(PhyLink::Backup,c)&&c.ticket==3&&!c.success&&c.evidence==TxEvidence::None);
 assert(h.takeCompletion(PhyLink::Backup,c)&&c.ticket==4&&!c.success&&c.evidence==TxEvidence::None);
 assert(!h.takeCompletion(PhyLink::Backup,c)&&e.sends==2&&n.sends==1);
 assert(!g.rearm(3,false));assert(g.rearm(3,true));
 assert(offer(PhyLink::Backup,6,3,false));run(200,280);
 assert(h.takeCompletion(PhyLink::Backup,c)&&c.ticket==6&&c.generation==3&&c.evidence==TxEvidence::LocalReady);
 assert(e.sends==3&&s.status(P)==S::State::Fault);
}
