#include "mission_neutral_wire.h"
#include "remote_neutral_evidence.h"
#include "radio_protocol.h"
#include <cassert>
using namespace flytec_rtos;
int main(){
 RemoteNeutralEvidence e;uint8_t key[32]={7};RadioFrame f{};uint16_t id=0,seq=0;
 assert(e.offered(9,11,1,3,100));assert(!e.fresh(9,1,3,101,true));
 radio_frameInit(f,RADIO_MSG_MISSION_NEUTRAL_ACK,2,1,20,9,101,1);
 assert(missionNeutralEncode(9,11,f.payload)&&radio_frameSeal(f,key)&&radio_frameVerify(f,key));
 assert(missionNeutralDecode(f.payload,id,seq)&&id==9&&seq==11);
 auto corrupt=f;corrupt.payload[2]++;assert(!radio_frameVerify(corrupt,key));
 corrupt=f;corrupt.payload[7]=1;assert(!missionNeutralDecode(corrupt.payload,id,seq));
 assert(!e.receipt(id,seq,1,3,101,102,false));assert(!e.receipt(id,12,1,3,101,102,true));
 assert(!e.receipt(8,seq,1,3,101,102,true));assert(!e.receipt(id,seq,2,3,101,102,true));
 assert(!e.receipt(id,seq,1,4,101,102,true));assert(!e.receipt(id,seq,1,3,99,102,true));
 assert(e.receipt(id,seq,1,3,101,102,true));assert(!e.receipt(id,seq,1,3,103,103,true));
 assert(e.fresh(9,1,3,199,true)&&!e.fresh(9,1,3,200,true));
 assert(!e.fresh(9,1,3,103,false)&&!e.fresh(9,1,4,103,true));
 assert(e.offered(9,12,1,3,110));assert(!e.fresh(9,1,3,111,true));
 assert(!e.receipt(9,11,1,3,111,111,true));assert(!e.receipt(9,12,1,3,210,210,true));
 e.reset();assert(!e.receipt(9,12,1,3,111,111,true));
 assert(e.offered(9,13,1,4,UINT32_MAX-5));assert(e.receipt(9,13,1,4,0,1,true));
 assert(e.fresh(9,1,4,93,true)&&!e.fresh(9,1,4,94,true));
}
