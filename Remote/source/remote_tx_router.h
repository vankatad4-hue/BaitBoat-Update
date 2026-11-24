#pragma once
#include <stdint.h>
// One application-owner drain per cycle, before producers reserve new slots.
// Local evidence is NOT peer acceptance or execution. No callback may submit
// or change the session during this drain; return false to request fail-closed
// reset by the caller after both lanes have been inspected.
namespace flytec_rtos {
template<class Tx,class Handshake,class Control,class Ordinary,class Mission>
bool routeRemoteTx(Tx& tx,uint32_t now,Handshake handshake,Control control,Ordinary ordinary,Mission mission){
 using Link=typename Tx::Link;
 using Kind=typename Tx::Kind;
 bool ok=true;
 const Link links[]={Link::Primary,Link::Backup};
 for(const auto link:links){
  typename Tx::Result result{};
  if(!tx.take(link,now,result))continue;
  bool consumed=false;
  if(result.link==link){
   switch(result.kind){
    case Kind::Handshake:consumed=handshake(result,now);break;
    case Kind::Stop:case Kind::Heartbeat:consumed=control(result,now);break;
    case Kind::Ordinary:consumed=ordinary(result,now);break;
    case Kind::Mission:consumed=mission(result,now);break;
    default:break;
   }
  }
  ok=consumed&&ok;
 }
 return ok;
}
}
