#include "remote_tx_router.h"
#include <cassert>
using namespace flytec_rtos;
struct Tx{
 enum class Link:uint8_t{Primary,Backup};
 enum class Kind:uint8_t{Unsupported,Handshake,Stop,Heartbeat,Ordinary,Mission};
 struct Result{Kind kind=Kind::Unsupported;Link link=Link::Primary;uint32_t ticket=0;};
 Result p{},b{};bool haveP=false,haveB=false;unsigned calls=0;
 bool take(Link l,uint32_t,Result& out){
  ++calls;auto& present=l==Link::Primary?haveP:haveB;
  if(!present)return false;
  out=l==Link::Primary?p:b;present=false;return true;
 }
};
int main(){
 using K=Tx::Kind;using L=Tx::Link;
 Tx tx;unsigned hs=0,control=0,ordinary=0,mission=0;
 auto H=[&](const Tx::Result& r,uint32_t n){assert(r.kind==K::Handshake&&n==12);++hs;return true;};
 auto C=[&](const Tx::Result& r,uint32_t n){assert((r.kind==K::Stop||r.kind==K::Heartbeat)&&n==12);++control;return true;};
 auto O=[&](const Tx::Result& r,uint32_t n){assert(r.kind==K::Ordinary&&n==12);++ordinary;return true;};
 auto M=[&](const Tx::Result& r,uint32_t n){assert(r.kind==K::Mission&&n==12);++mission;return true;};
 auto route=[&](){return routeRemoteTx(tx,12,H,C,O,M);};
 auto fill=[&](K p,K b){tx.p={p,L::Primary,1};tx.b={b,L::Backup,2};tx.haveP=tx.haveB=true;};
 fill(K::Handshake,K::Stop);assert(route());assert(hs==1&&control==1&&tx.calls==2);
 fill(K::Ordinary,K::Mission);assert(route());assert(ordinary==1&&mission==1&&tx.calls==4);
 fill(K::Unsupported,K::Heartbeat);assert(!route());assert(control==2&&tx.calls==6);
 fill(K::Ordinary,K::Mission);tx.p.link=L::Backup;assert(!route());assert(ordinary==1&&mission==2);
 assert(route()&&tx.calls==10);
 fill(K::Ordinary,K::Stop);
 auto deny=[](const Tx::Result&,uint32_t){return false;};
 assert(!routeRemoteTx(tx,12,H,C,deny,M));assert(control==3&&ordinary==1&&tx.calls==12);
}
