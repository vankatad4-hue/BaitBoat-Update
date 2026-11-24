#include "r30h118_phy_adapters.h"
#include <cassert>
using namespace flytec_r30h118;
struct Nrf {
 bool up=true,pending=false,success=false;
 bool ready()const{return up;}
 bool txPending()const{return pending;}
 bool lastTxSuccess()const{return success;}
 bool sendFrame(const uint32_t&){pending=true;return true;}
 bool receiveFrame(uint32_t&){return false;}
 void update(){}
};
struct E22 {
 enum class Admission {Accepted,Busy,DutyLimited,Invalid,Unavailable,Uncertain};
 using O=flytec_rtos::E22TxState::Outcome;
 Admission next=Admission::Accepted;
 O state=O::Idle;
 bool configured()const{return true;}
 bool transportFault()const{return state==O::Uncertain;}
 bool txPending()const{return state==O::Pending;}
 bool ready()const{return !txPending()&&!transportFault();}
 O txOutcome()const{return state;}
 Admission trySendFrame(const uint32_t&,uint32_t){
 if(next==Admission::Accepted)state=O::Pending;
 if(next==Admission::Uncertain)state=O::Uncertain;
 return next;
 }
 bool receiveFrame(uint32_t&){return false;}
 void update(){}
};
int main(){
 Ticket t{1,1,1};
 PhyEvents<uint32_t> ne,ee;
 PhyEvents<uint32_t>::Outcome o{};
 Nrf n;NrfAdapter<uint32_t,Nrf> na(n,ne);
 assert(na.admit(t,42));assert(!na.admit(t,42));
 n.pending=false;n.success=true;na.poll(1);
 assert(ne.takeOutcome(o)&&sameTicket(o.ticket,t)&&o.result==PhyOutcome::LocalComplete);
 assert(na.admit(t,42));n.pending=false;n.success=false;na.poll(2);
 assert(ne.takeOutcome(o)&&o.result==PhyOutcome::Failed);
 assert(na.admit(t,42));n.pending=false;n.up=false;na.poll(3);
 assert(ne.takeOutcome(o)&&o.result==PhyOutcome::Uncertain);
 E22 e;E22Adapter<uint32_t,E22> ea(e,ee);
 e.next=E22::Admission::Busy;assert(!ea.admit(t,42,0));
 e.next=E22::Admission::Accepted;assert(ea.admit(t,42,0));assert(!ea.admit(t,42,0));
 e.state=E22::O::LocalReady;ea.poll(40);
 assert(ee.takeOutcome(o)&&o.result==PhyOutcome::LocalComplete);
 assert(ea.admit(t,42,50));e.state=E22::O::Uncertain;ea.poll(2050);
 assert(ee.takeOutcome(o)&&o.result==PhyOutcome::Uncertain);
 e.state=E22::O::Idle;e.next=E22::Admission::Uncertain;
 assert(!ea.admit(t,42,3000));ea.poll(3001);
 assert(ee.takeOutcome(o)&&sameTicket(o.ticket,t)&&o.result==PhyOutcome::Uncertain);
}
