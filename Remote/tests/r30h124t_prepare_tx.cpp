#include "remote_mission_prepare_tx.h"
#include <cassert>
using namespace flytec_rtos;
struct Tx{
 enum class Link:uint8_t{Primary,Backup};enum class Submit:uint8_t{Queued,Busy,Denied};enum class Kind:uint8_t{Mission};
 struct Result{Link link;Kind kind;bool stop;uint8_t messageType,epoch;uint16_t commandId,sequence;uint32_t ticket,generation,createdMs,lifetimeMs;};
 bool busy=false;unsigned offers=0;RadioFrame last{};Result result{};Submit next=Submit::Queued;
 bool pending(Link)const{return busy;}bool linkFault(Link)const{return false;}uint32_t generation()const{return 3;}
 Submit submitPrepare(const RadioFrame& f,uint32_t created,uint32_t ttl,uint32_t,uint32_t& ticket,bool safe){
  assert(safe&&scopedPrepareFrame(f));++offers;if(next!=Submit::Queued)return next;
  last=f;ticket=offers;busy=true;result={Link::Primary,Kind::Mission,false,f.type,f.sessionEpoch,f.commandId,f.sequence,ticket,3,created,ttl};return next;
 }
};
int main(){
 Tx tx;RemoteMissionPrepareTx<Tx> producer(tx);auto& up=producer.upload();MissionAssembler peer;
 uint8_t key[32]={1};uint16_t seq=0;MissionPlan plan{9,1,15,8,300,431000000,270000000,431010000,270010000};
 assert(up.begin(plan,1,10,true));auto tick=[&](uint32_t now,bool safe=true,bool gap=true){return producer.service(now,safe,gap,1,seq,30000,key);};
 assert(tick(10,true,false)&&seq==0);tx.next=Tx::Submit::Busy;assert(tick(11)&&up.reservations()==0);
 tx.next=Tx::Submit::Queued;
 for(unsigned op=1;op<=6;++op){uint32_t at=12+2*op;assert(tick(at));assert(tx.last.payload[0]==op&&radio_frameVerify(tx.last,key));
  auto wrong=tx.last;wrong.flags=RADIO_FLAG_BACKUP_LINK;assert(!scopedPrepareFrame(wrong));wrong=tx.last;wrong.commandId++;assert(!scopedPrepareFrame(wrong));
  wrong=tx.last;wrong.payload[0]=MISSION_CANCEL;assert(!scopedPrepareFrame(wrong));wrong=tx.last;wrong.type=RADIO_MSG_MISSION_CONTROL;assert(!scopedPrepareFrame(wrong));
  auto result=peer.ingest(tx.last.payload,at+1,true,true);assert(up.acknowledge(9,static_cast<MissionOpcode>(op),result,1,at+1,at+1,true));
  auto stale=tx.result;stale.generation--;assert(!producer.observeCompletion(stale));assert(producer.observeCompletion(tx.result));tx.busy=false;
 }
 MissionPlan received{};assert(up.prepared(30)&&peer.getPrepared(received,30)&&missionPlan_digest(received)==missionPlan_digest(plan));
 assert(!tick(31,false)&&!up.prepared(31));up.transportRevoked();plan.transaction++;assert(up.begin(plan,1,40,true));assert(tick(40));
 assert(!tick(41,false)&&up.inFlight());up.transportRevoked();assert(!producer.observeCompletion(tx.result));
}
