#include "radio_frame_handoff.h"
#include <cassert>
using namespace flytec_rtos;
int main(){
 RadioFrameHandoff h;assert(h.begin());
 RxFrame r{};r.link=PhyLink::Primary;
 for(unsigned i=0;i<8;++i){r.frame.sequence=i;assert(h.primaryRx.offer(r));}
 assert(!h.primaryRx.offer(r)&&h.fault());
 r.link=PhyLink::Backup;r.frame.sequence=99;assert(h.backupRx.offer(r));
 RxFrame out{};assert(h.backupRx.take(out)&&out.frame.sequence==99);
 for(unsigned i=0;i<8;++i){assert(h.primaryRx.take(out)&&out.frame.sequence==i);}
 assert(!h.primaryRx.take(out)&&h.fault());
 TxRequest tx{};tx.ticket=42;tx.generation=7;tx.link=PhyLink::Backup;
 assert(h.backupTx.offer(tx));TxRequest req{};assert(h.backupTx.take(req));
 assert(req.ticket==42&&req.generation==7);
 TxCompletion c{req.ticket,req.generation,req.link,true};
 assert(h.completions.offer(c));TxCompletion result{};
 assert(h.completions.take(result)&&result.ticket==42&&result.generation==7&&result.success);
 // Ordinary traffic cannot consume critical capacity.
 for(unsigned i=0;i<4;++i){tx.ticket=i;assert(h.primaryTx.offer(tx));}
 tx.ticket=100;tx.link=PhyLink::Primary;assert(h.primaryCritical.offer(tx));
 tx.ticket=101;assert(h.primaryCritical.offer(tx));
 assert(!h.primaryCritical.offer(tx));
 assert(h.takeTx(PhyLink::Primary,req)&&req.ticket==100);
 assert(h.takeTx(PhyLink::Primary,req)&&req.ticket==101);
 for(unsigned i=0;i<4;++i){assert(h.takeTx(PhyLink::Primary,req)&&req.ticket==i);}
 assert(!h.takeTx(PhyLink::Primary,req));
}
