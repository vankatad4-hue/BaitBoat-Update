#include "remote_i2c_dispatcher.h"
#include <cassert>
#include <thread>
using D=flytec_rtos::RemoteI2cDispatcher;
static bool execute(void* ctx,D::Operation op){
 ++*static_cast<unsigned*>(ctx);return op==D::Operation::PollPrimary;
}
int main(){
 D d;unsigned calls=0;assert(!d.begin());assert(flytec_rtos::bindRemoteI2cOwner());assert(d.begin());
 assert(!d.submit({0,0,D::Operation::PollPrimary}));
 std::thread producer([&]{
  assert(d.submit({1,100,D::Operation::PollPrimary}));
  assert(d.submit({2,100,D::Operation::PollBackup}));
  assert(!d.service(101,execute,&calls));
 });
 producer.join();assert(calls==0);
 assert(d.service(101,execute,&calls)&&calls==1);
 D::Result r{};assert(d.take(r)&&r.ticket==1&&r.outcome==D::Outcome::Completed);
 assert(d.service(102,execute,&calls)&&calls==2);
 assert(d.take(r)&&r.ticket==2&&r.outcome==D::Outcome::Failed);
 assert(d.submit({3,100,D::Operation::PollPrimary}));
 assert(d.service(200,execute,&calls)&&calls==2);
 assert(d.take(r)&&r.ticket==3&&r.outcome==D::Outcome::Expired);
 assert(d.submit({4,UINT32_MAX-10,D::Operation::PollPrimary}));
 assert(d.service(5,execute,&calls)&&calls==3);
 assert(d.take(r)&&r.ticket==4&&r.outcome==D::Outcome::Completed);
 for(unsigned i=0;i<4;++i)assert(d.submit({i+5,200,D::Operation::PollPrimary}));
 assert(!d.submit({9,200,D::Operation::PollBackup})&&d.fault());
 assert(!d.service(201,execute,&calls)&&calls==3);
}
