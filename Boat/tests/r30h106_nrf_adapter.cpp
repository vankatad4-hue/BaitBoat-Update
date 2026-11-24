#include "nrf_phy_adapter.h"
#include <cassert>
using namespace flytec_rtos;
struct Driver{
 bool active=true,pending=false,success=false;
 unsigned sends=0;
 bool ready()const{return active;}
 bool txPending()const{return pending;}
 bool lastTxSuccess()const{return success;}
 void update(){}
 bool receiveFrame(RadioFrame&){return false;}
 bool sendFrame(const RadioFrame&){++sends;pending=true;success=false;return true;}
};
int main(){
 RadioFrameHandoff h;assert(h.begin());TxGenerationGate g;assert(g.transition(1));
 Driver d;NrfPhyAdapter<Driver> a(d);TxRequest r{};r.link=PhyLink::Primary;r.generation=1;r.ticket=7;
 assert(h.primaryTx.offer(r));assert(a.tick(h,g,0)&&d.sends==1);
 r.ticket=8;assert(h.primaryTx.offer(r));assert(a.tick(h,g,1)&&d.sends==1);
 TxCompletion c{};assert(!h.takeCompletion(r.link,c));
 d.pending=false;d.success=true;assert(a.tick(h,g,2)&&d.sends==2);
 assert(h.takeCompletion(r.link,c)&&c.ticket==7&&c.generation==1&&c.success);
 d.active=false;d.pending=false;assert(!a.tick(h,g,3));
 assert(h.takeCompletion(r.link,c)&&c.ticket==8&&!c.success);
 assert(!h.takeCompletion(r.link,c));
}
