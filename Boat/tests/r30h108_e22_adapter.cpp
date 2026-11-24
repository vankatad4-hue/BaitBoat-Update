#include "e22_phy_adapter.h"
#include <cassert>
using namespace flytec_rtos;
struct Driver{
 enum class Admission{Accepted,Busy,DutyLimited,Invalid,Unavailable,Uncertain};
 Admission next=Admission::Busy;
 E22TxState state;
 bool configured()const{return !state.fault();}
 bool transportFault()const{return state.fault();}
 bool txPending()const{return state.pending();}
 E22TxState::Outcome txOutcome()const{return state.outcome();}
 void update(){}
 bool receiveFrame(RadioFrame&){return false;}
 Admission trySendFrame(const RadioFrame&,uint32_t now){
  if(next==Admission::Accepted)state.accepted(now);
  if(next==Admission::Uncertain)state.uncertain();
  return next;
 }
};
int main(){
 RadioFrameHandoff h;assert(h.begin());TxGenerationGate gate;assert(gate.transition(1));
 Driver d;E22PhyAdapter<Driver> a(d);
 TxRequest r{};r.link=PhyLink::Backup;r.generation=1;r.ticket=21;
 assert(h.backupCritical.offer(r));assert(a.tick(h,gate,0));
 TxCompletion c;assert(!h.takeCompletion(r.link,c));
 d.next=Driver::Admission::DutyLimited;assert(a.tick(h,gate,1));
 d.next=Driver::Admission::Accepted;assert(a.tick(h,gate,2));
 assert(!h.takeCompletion(r.link,c));d.state.poll(36,true);
 assert(a.tick(h,gate,36));assert(h.takeCompletion(r.link,c));
 assert(c.ticket==21&&!c.success&&c.evidence==TxEvidence::LocalReady);
 r.ticket=22;assert(h.backupTx.offer(r));d.next=Driver::Admission::Uncertain;
 assert(!a.tick(h,gate,37));assert(h.takeCompletion(r.link,c)&&c.ticket==22&&!c.success&&c.evidence==TxEvidence::None);
 assert(!a.tick(h,gate,38));assert(!h.takeCompletion(r.link,c));
}
