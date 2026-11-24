#include "remote_i2c_phy_service.h"
#include <cassert>
#include <thread>
using D=flytec_rtos::RemoteI2cDispatcher;
constexpr auto P=D::Operation::PollPrimary;
constexpr auto B=D::Operation::PollBackup;
static bool execute(void* ctx,D::Operation op){
 ++*static_cast<unsigned*>(ctx);return op==P;
}
struct FakePhy{
 unsigned calls=0;uint32_t last=0;
 bool tick(flytec_rtos::RadioFrameHandoff&,flytec_rtos::TxGenerationGate&,uint32_t now){
  assert(flytec_rtos::remoteI2cAllowed());++calls;last=now;return true;
 }
};
int main(){
 D d;unsigned calls=0;assert(!d.begin());assert(flytec_rtos::bindRemoteI2cOwner());assert(d.begin());
 assert(!d.submit({0,0,P}));
 std::thread producer([&]{
  assert(d.submit({1,100,P}));assert(d.submit({2,100,P}));assert(d.submit({3,100,B}));
  assert(!d.service(101,execute,&calls));
 });
 producer.join();assert(calls==0);
 D::Result r{};
 assert(d.service(101,execute,&calls)&&calls==1);
 assert(d.take(P,r)&&r.ticket==1&&r.outcome==D::Outcome::Completed);
 assert(d.service(102,execute,&calls)&&calls==2);
 assert(d.take(B,r)&&r.ticket==3&&r.outcome==D::Outcome::Failed);
 assert(d.service(103,execute,&calls)&&calls==3);
 assert(d.take(P,r)&&r.ticket==2);
 assert(d.submit({4,100,P}));assert(d.service(200,execute,&calls)&&calls==3);
 assert(d.take(P,r)&&r.ticket==4&&r.outcome==D::Outcome::Expired);
 assert(d.submit({5,UINT32_MAX-10,P}));assert(d.service(5,execute,&calls)&&calls==4);
 assert(d.take(P,r)&&r.ticket==5&&r.outcome==D::Outcome::Completed);
 for(unsigned i=0;i<4;++i)assert(d.submit({i+6,200,P}));
 assert(!d.submit({10,200,P})&&d.fault(P)&&!d.fault(B));
 assert(d.submit({11,200,B}));assert(d.service(201,execute,&calls)&&calls==5);
 assert(d.take(B,r)&&r.ticket==11);
 assert(d.service(202,execute,&calls)&&calls==5);
 // Overflow the result lane, not the request lane, in the reverse direction.
 D e;assert(e.begin());
 for(unsigned i=1;i<=4;++i){assert(e.submit({i,300,B}));assert(e.service(301,execute,&calls));}
 assert(e.submit({5,300,B}));assert(!e.service(301,execute,&calls));
 assert(e.fault(B)&&!e.fault(P));assert(e.submit({6,300,P}));
 assert(e.service(301,execute,&calls));assert(e.take(P,r)&&r.ticket==6);
 D f;assert(f.begin());FakePhy p,b;flytec_rtos::RadioFrameHandoff h;
 assert(h.begin());flytec_rtos::TxGenerationGate g;assert(g.transition(1));
 flytec_rtos::RemoteI2cPhyService<FakePhy,FakePhy> svc(f,p,b,h,g);
 std::thread worker([&]{assert(f.submit({12,400,P}));assert(f.submit({13,400,B}));assert(!svc.service(400));});
 worker.join();assert(p.calls==0&&b.calls==0);
 assert(svc.service(401)&&p.calls==1&&b.calls==0);
 assert(svc.service(402)&&b.calls==1&&b.last==402);
 assert(f.take(P,r)&&r.ticket==12&&r.outcome==D::Outcome::Completed);
 assert(f.take(B,r)&&r.ticket==13&&r.outcome==D::Outcome::Completed);
}
