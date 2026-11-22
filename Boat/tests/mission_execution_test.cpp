#include "../source/mission_execution.h"
#include <cassert>
#include <iostream>
int main(){
 MissionPlan p={42,0,0,5,300,0,1000,0,0};
 ExecInputs in={{0,0,1,90,true,true,true,true},false,true,true,true,true,true};
 MissionExecution m;
 assert(!m.start(p,in,0,false));assert(m.start(p,in,0,true));
 assert(m.tick(in,1).intent==INTENT_NAV_TARGET);
 assert(m.pause(true));assert(m.tick(in,2).intent==INTENT_STOP);
 assert(m.resume(in,true));in.fix.lonE7=1000;
 assert(m.tick(in,3).intent==INTENT_STOP&&m.phase()==EXEC_AT_TARGET);
 in.stationary=false;assert(m.tick(in,4).intent==INTENT_STOP);
 in.stationary=true;auto r=m.tick(in,5);assert(r.intent==INTENT_RELEASE&&r.channel==0);
 assert(m.tick(in,6).intent==INTENT_STOP);
 assert(!m.releaseComplete(43,r.token,true,true));
 assert(m.releaseComplete(42,r.token,true,true));
 r=m.tick(in,7);assert(r.intent==INTENT_RELEASE&&r.channel==2);
 assert(!m.releaseComplete(42,1,true,true));assert(m.releaseComplete(42,r.token,true,true));
 assert(m.tick(in,8).intent==INTENT_STOP&&m.phase()==EXEC_RETURNING);
 assert(m.tick(in,9).intent==INTENT_NAV_HOME);
 in.fix.lonE7=0;assert(m.tick(in,10).intent==INTENT_STOP&&m.phase()==EXEC_COMPLETE);
 assert(!m.start(p,in,11,true));
 ++p.transaction;assert(m.start(p,in,12,true));
 in.primaryFresh=false;in.backupFresh=true;assert(m.tick(in,13).intent==INTENT_STOP&&m.phase()==EXEC_RETURNING);
 assert(m.tick(in,14).intent==INTENT_STOP&&m.phase()==EXEC_COMPLETE);
 ++p.transaction;in.primaryFresh=true;in.backupFresh=true;in.fix.lonE7=1000;
 assert(m.start(p,in,20,true));m.tick(in,21);r=m.tick(in,22);assert(r.intent==INTENT_RELEASE);
 in.primaryFresh=false;assert(m.tick(in,23).intent==INTENT_STOP&&m.phase()==EXEC_ABORTED);
 ++p.transaction;in.primaryFresh=true;assert(m.start(p,in,30,true));m.tick(in,31);r=m.tick(in,32);assert(r.intent==INTENT_RELEASE);
 assert(m.tick(in,5032).intent==INTENT_STOP&&m.phase()==EXEC_ABORTED);
 ++p.transaction;assert(m.start(p,in,6000,true));in.stopActive=true;
 assert(m.tick(in,6001).intent==INTENT_STOP&&m.phase()==EXEC_ABORTED);
 std::cout<<"Mission execution supervisor simulation passed\n";
}
