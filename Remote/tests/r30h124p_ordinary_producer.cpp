#include "remote_async_ordinary.h"
#include <cassert>
using namespace flytec_rtos;
struct Tx{
 enum class Link:uint8_t{Primary,Backup};enum class Submit:uint8_t{Queued,Busy,Denied,Expired,Fault};
 enum class Kind:uint8_t{Ordinary};enum class Outcome:uint8_t{NrfAck,Failed};
 struct Result{Link link;Kind kind;Outcome outcome;bool stop;uint8_t messageType,epoch;uint16_t commandId,sequence;uint32_t ticket,generation,createdMs,lifetimeMs;};
 bool busy=false,fault=false;unsigned offers=0;Submit next=Submit::Queued;Result last{};
 bool pending(Link)const{return busy;}bool linkFault(Link)const{return fault;}uint32_t generation()const{return 3;}
 Submit submit(Link l,const RadioFrame& f,uint32_t created,uint32_t ttl,uint32_t,uint32_t& ticket){
  assert(l==Link::Primary);++offers;if(next!=Submit::Queued)return next;
  ticket=offers;last={l,Kind::Ordinary,Outcome::NrfAck,false,f.type,f.sessionEpoch,f.commandId,f.sequence,ticket,generation(),created,ttl};busy=true;return next;
 }
};
int main(){
 Tx tx;RemoteAsyncOrdinary<Tx> producer(tx);OrdinaryAuthorization a{};
 a.primaryFresh=a.sessionReady=a.stopConfirmed=a.manualConfirmed=a.actuatorReady=a.joystickValid=true;
 a.stopped=a.missionLocked=a.failsafe=false;
 uint16_t seq=0;uint8_t key[32]={1};BoatCommand c{};c.type=CMD_MANUAL_MOTORS;c.commandId=1;c.createdAtMs=100;
 auto encode=[](const BoatCommand& c,uint8_t s,uint8_t d,uint8_t e,RadioFrame& f,const uint8_t*){
  f.type=c.type==CMD_MANUAL_MOTORS?RADIO_MSG_CONTROL:RADIO_MSG_ACTION;f.sourceId=s;f.destinationId=d;f.sessionEpoch=e;
  f.sequence=uint16_t(c.sequence);f.commandId=uint16_t(c.commandId);f.payload[0]=uint8_t(c.type);return true;
 };
 auto tick=[&](uint32_t now,bool gap){return producer.service(a,now,gap,1,seq,30000,key,encode);};
 assert(producer.intent().stage(c,a,100));assert(tick(101,false)&&seq==0&&tx.offers==0);
 tx.busy=true;assert(tick(102,true)&&seq==0);tx.busy=false;
 assert(tick(103,true)&&seq==1&&tx.offers==1&&tx.last.createdMs==100&&tx.last.lifetimeMs==50);
 auto wrong=tx.last;wrong.link=Tx::Link::Backup;assert(!producer.observeCompletion(wrong,104));
 a.primaryFresh=false;assert(!tick(104,true));a.primaryFresh=true;
 assert(producer.observeCompletion(tx.last,105));assert(!producer.observeCompletion(tx.last,105));tx.busy=false;
 c.commandId=2;c.createdAtMs=110;assert(producer.intent().stage(c,a,110));
 tx.next=Tx::Submit::Denied;assert(tick(111,true)&&producer.intent().state()==OrdinaryIntent::State::Canceled);
 c.commandId=3;c.createdAtMs=120;assert(producer.intent().stage(c,a,120));
 assert(tick(170,true)&&producer.intent().state()==OrdinaryIntent::State::Canceled&&tx.offers==2);
}
