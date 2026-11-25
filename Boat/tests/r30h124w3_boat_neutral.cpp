#include "boat_neutral_receipt.h"
#include "remote_neutral_evidence.h"
#include <cassert>
using namespace flytec_rtos;
int main(){
 BoatNeutralReceipt boat;MissionStickLease lease;RemoteNeutralEvidence remote;
 MissionStickInput sample{9,0,0,true};uint8_t payload[8]{},key[32]={1};
 assert(!boat.offer(sample,11,1,3,100,100,false));
 assert(lease.receive(sample,9,100,true)&&boat.offer(sample,11,1,3,100,100,true));
 assert(!boat.take(9,1,3,101,false,true,lease,payload)); // priority traffic
 assert(!boat.take(9,1,3,102,true,true,lease,payload)); // not retained
 assert(boat.offer(sample,11,1,3,100,102,true));lease.reset();
 assert(!boat.take(9,1,3,103,true,true,lease,payload)); // STOP/revoke
 assert(lease.receive(sample,9,104,true)&&boat.offer(sample,12,1,3,104,104,true));
 assert(!boat.take(9,2,3,105,true,true,lease,payload));
 assert(boat.offer(sample,12,1,3,104,105,true));assert(!boat.take(9,1,4,105,true,true,lease,payload));
 assert(boat.offer(sample,12,1,3,104,105,true));assert(!boat.take(9,1,3,105,true,false,lease,payload));
 assert(boat.offer(sample,12,1,3,104,105,true));assert(!boat.take(9,1,3,154,true,true,lease,payload));
 assert(lease.receive(sample,9,200,true)&&boat.offer(sample,13,1,4,200,200,true));
 assert(remote.offered(9,13,1,4,199));assert(boat.take(9,1,4,201,true,true,lease,payload));
 RadioFrame f{};radio_frameInit(f,RADIO_MSG_MISSION_NEUTRAL_ACK,2,1,22,9,201,1);
 for(unsigned i=0;i<8;++i)f.payload[i]=payload[i];
 assert(radio_frameSeal(f,key)&&radio_frameVerify(f,key));uint16_t id=0,seq=0;
 assert(missionNeutralDecode(f.payload,id,seq)&&remote.receipt(id,seq,1,4,202,202,true));
 assert(remote.fresh(9,1,4,298,true)&&!remote.fresh(9,1,4,299,true));
 assert(lease.neutral(9,399)&&!lease.neutral(9,400));
 assert(!boat.offer({9,5,0,false},14,1,4,205,205,true));
 assert(lease.receive(sample,9,UINT32_MAX-5,true)&&boat.offer(sample,15,1,4,UINT32_MAX-5,0,true));
 assert(boat.take(9,1,4,1,true,true,lease,payload));
}
