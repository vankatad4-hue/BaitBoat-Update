#include "remote_exclusive_runtime.h"
#include "remote_async_tx.h"
#include "e22_phy_adapter.h"
#include <cassert>
using namespace flytec_rtos;
struct Worker{
 bool launch=true,done=false;unsigned starts=0,stops=0;
 RadioWatchdog::State health=RadioWatchdog::State::Healthy;
 bool begin(bool a){++starts;return launch&&a;}
 void requestStop(){++stops;}
 bool exited()const{return done;}
 bool permit()const{return true;}
 RadioWatchdog::State supervise(){return health;}
};
struct Service{bool service(uint32_t){return true;}};
struct App{
 RemoteRadioOwner& owner;RemoteAsyncTx& tx;unsigned inits=0,revokes=0;bool ok=true,ownerWasWorker=false;
 static bool init(void* p){auto& a=*static_cast<App*>(p);++a.inits;return a.ok&&a.tx.beginSession(1);}
 static void revoke(void* p){auto& a=*static_cast<App*>(p);a.ownerWasWorker=a.owner.worker();++a.revokes;assert(a.tx.retireSession());}
};
struct Driver{
 enum class Admission{Accepted,Busy,DutyLimited,Invalid,Unavailable,Uncertain};
 E22TxState state;unsigned starts=0;
 bool configured()const{return !state.fault();}
 bool transportFault()const{return state.fault();}
 bool txPending()const{return state.pending();}
 E22TxState::Outcome txOutcome()const{return state.outcome();}
 void update(){}
 bool receiveFrame(RadioFrame&){return false;}
 Admission trySendFrame(const RadioFrame&,uint32_t now){++starts;state.accepted(now);return Admission::Accepted;}
};
int main(){
 assert(bindRemoteI2cOwner());RemoteRadioOwner owner;RadioFrameHandoff h;TxGenerationGate gate;
 RemoteAsyncTx tx(owner,h,gate);Worker w;Service s;RemoteWorkerRuntime<Worker,Service> rt(w,s);
 RemoteExclusiveRuntime<decltype(rt)> ex(owner,h,rt);App app{owner,tx};
 assert(!ex.begin(false,&App::init,&App::revoke,&app));
 assert(ex.begin(true,&App::init,&App::revoke,&app)&&app.inits==1);
 assert(!owner.claimLegacy()&&!ex.begin(true,&App::init,&App::revoke,&app));
 assert(ex.tick(0,true,false)&&ex.permit()); // degraded link is not global fault
 assert(!ex.tick(1,true,true)&&app.revokes==1&&app.ownerWasWorker&&w.stops==1&&gate.stopped());
 assert(!owner.worker()&&!owner.claimLegacy()&&!ex.permit());
 ex.requestStop();assert(app.revokes==1&&w.stops==1);w.done=true;assert(ex.exited());
 RemoteRadioOwner failedOwner;RadioFrameHandoff fh;TxGenerationGate fg;RemoteAsyncTx ft(failedOwner,fh,fg);
 Worker fw;Service fs;RemoteWorkerRuntime<Worker,Service> fr(fw,fs);
 RemoteExclusiveRuntime<decltype(fr)> fe(failedOwner,fh,fr);App fa{failedOwner,ft};fa.ok=false;
 assert(!fe.begin(true,&App::init,&App::revoke,&fa)&&fa.revokes==1&&fw.starts==0);
 assert(!failedOwner.claimLegacy()&&!fe.begin(true,&App::init,&App::revoke,&fa));
 // A pre-started runtime cannot be adopted by the exclusive wrapper. Even
 // though callback binding fails, fail closed by retiring the owner and
 // requesting stop on the already-live worker.
 RemoteRadioOwner adoptedOwner;RadioFrameHandoff ah;TxGenerationGate ag;
 RemoteAsyncTx at(adoptedOwner,ah,ag);Worker aw;Service as;
 RemoteWorkerRuntime<Worker,Service> ar(aw,as);assert(ar.begin(true,true));
 RemoteExclusiveRuntime<decltype(ar)> ae(adoptedOwner,ah,ar);App aa{adoptedOwner,at};
 assert(!ae.begin(true,&App::init,&App::revoke,&aa));
 assert(aa.inits==0&&aa.revokes==1&&aw.starts==1&&aw.stops==1);
 assert(ar.state()==decltype(ar)::State::Retired&&!adoptedOwner.worker());
 // External application retirement must not strand a live Core 0 worker.
 RemoteRadioOwner eo;RadioFrameHandoff eh;TxGenerationGate eg;RemoteAsyncTx et(eo,eh,eg);
 Worker ew;Service es;RemoteWorkerRuntime<Worker,Service> er(ew,es);
 RemoteExclusiveRuntime<decltype(er)> ee(eo,eh,er);App ea{eo,et};
 assert(ee.begin(true,&App::init,&App::revoke,&ea));assert(ee.tick(0));
 eo.retire();assert(!ee.tick(1)&&ew.stops==1);
 assert(!ee.permit()&&ea.revokes==1&&!ea.ownerWasWorker&&eg.stopped());ee.requestStop();assert(ew.stops==1);
 // Actual mock driver admissions: wait from terminal local completion even
 // when the application generation changes. Deadline is still checked first.
 RadioFrameHandoff ph;assert(ph.begin());TxGenerationGate pg;assert(pg.transition(1));
 Driver d;E22PhyAdapter<Driver> a(d,3000);
 TxRequest req{};req.link=PhyLink::Backup;req.ticket=1;req.generation=1;
 assert(ph.backupCritical.offer(req)&&a.tick(ph,pg,100));assert(d.starts==1);
 d.state.poll(134,true);assert(a.tick(ph,pg,134));
 assert(pg.latchStop(2));req.ticket=2;req.generation=2;req.createdMs=135;req.lifetimeMs=10000;
 assert(ph.backupCritical.offer(req)&&a.tick(ph,pg,135));assert(d.starts==1);
 assert(a.tick(ph,pg,3133)&&d.starts==1);
 assert(a.tick(ph,pg,3134)&&d.starts==2);
}

