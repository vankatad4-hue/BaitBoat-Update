#include "e22_phy_adapter.h"
#include <cassert>
using namespace flytec_rtos;
struct Driver{
 enum class Admission{Accepted,Busy,DutyLimited,Invalid,Unavailable,Uncertain};
 Admission next=Admission::Busy;
 E22TxState state;unsigned writes=0;
 bool ready()const{return configured()&&!txPending();}
 bool configured()const{return !state.fault();}
 bool transportFault()const{return state.fault();}
 bool txPending()const{return state.pending();}
 E22TxState::Outcome txOutcome()const{return state.outcome();}
 void update(){}
 bool receiveFrame(RadioFrame&){return false;}
 Admission trySendFrame(const RadioFrame&,uint32_t now){
  ++writes;if(next==Admission::Accepted)state.accepted(now);
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
 // Opt-in path: routine budget exhaustion must not consume STOP reserve.
 RadioFrameHandoff h2;assert(h2.begin());TxGenerationGate g2;assert(g2.transition(1));
 Driver d2;d2.next=Driver::Admission::Accepted;
 LoRaAirtimeProfile profile;profile.commissioned=true;profile.hourlyMs=50;profile.reserveMs=25;profile.packetMs=25;
 LoRaAirtimePolicy policy;assert(policy.configure(profile,0,true));
 E22AdaptiveAdmission<Driver> admission(d2,policy,20,true);
 using Traffic=LoRaAirtimePolicy::Traffic;
 const auto classify=+[](void*,const TxRequest& req,Traffic& traffic){
  traffic=req.ticket==33?Traffic::Emergency:Traffic::Routine;return true; // synthetic test-only class
 };
 E22PhyAdapter<Driver> adaptive(d2,admission,classify,nullptr);
 TxRequest q{};q.link=PhyLink::Backup;q.generation=1;q.ticket=31;
 assert(h2.backupCritical.offer(q)&&adaptive.tick(h2,g2,0));assert(d2.writes==1);
 d2.state.poll(34,true);assert(adaptive.tick(h2,g2,34)&&!h2.takeCompletion(q.link,c));
 assert(adaptive.tick(h2,g2,54)&&h2.takeCompletion(q.link,c)&&c.ticket==31&&c.evidence==TxEvidence::LocalReady);
 q.ticket=32;assert(h2.backupCritical.offer(q)&&adaptive.tick(h2,g2,3054)&&d2.writes==1);
 assert(g2.latchStop(2)&&g2.stopped());q.generation=2;q.ticket=33;assert(h2.backupCritical.offer(q));
 assert(adaptive.tick(h2,g2,3055)&&h2.takeCompletion(q.link,c)&&c.ticket==32&&c.evidence==TxEvidence::None);
 assert(adaptive.tick(h2,g2,3056)&&d2.writes==2); // STOP may use reserve
 d2.state.poll(3090,true);assert(adaptive.tick(h2,g2,3090)&&!h2.takeCompletion(q.link,c));
 assert(adaptive.tick(h2,g2,3110)&&h2.takeCompletion(q.link,c)&&c.ticket==33&&c.evidence==TxEvidence::LocalReady);
 assert(policy.usedMs(3110)==50);
}
