#include "../source/boat_local_stop_trace.h"
#include "../source/boat_radio_safety.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

int main(){
  using namespace boat_r30h2;
  LocalTrace trace;
  Record rec{};
  assert(!trace.take(rec));
  // Simulate accepted, verified radio events; this is not over-the-air evidence.
  trace.push(SESSION_ESTABLISHED,500,0,1,0);
  trace.push(AUTH_PRIMARY_SAMPLE,1000,1000,1,1);
  trace.push(AUTH_BACKUP_SAMPLE,1200,1200,2,3);
  trace.push(STOP_AUTH_RECEIVED,2000,2000,1,0,17);
  trace.push(STOP_LOGICAL_INTENT,2002,2000,1,0,17);
  assert(trace.size()==5);
  assert(trace.take(rec) && rec.event==SESSION_ESTABLISHED);
  assert(trace.take(rec) && rec.event==AUTH_PRIMARY_SAMPLE && rec.link==1);
  assert(trace.take(rec) && rec.event==AUTH_BACKUP_SAMPLE && rec.lastAcceptedRxMs==1200);
  assert(trace.take(rec) && rec.event==STOP_AUTH_RECEIVED && rec.commandId==17);
  assert(trace.take(rec) && rec.event==STOP_LOGICAL_INTENT && rec.timestampMs==2002);
  assert(!trace.take(rec));

  // Capacity is bounded; oldest safety records are never overwritten.
  for(unsigned i=0;i<LocalTrace::CAPACITY;++i)
    trace.push(AUTH_PRIMARY_SAMPLE,i,i,1,1,(uint16_t)i);
  trace.push(RTH_LOCAL_ENTRY,100,100,0,0);
  assert(trace.size()==LocalTrace::CAPACITY);
  assert(trace.overflows()==1);
  for(unsigned i=0;i<LocalTrace::CAPACITY;++i){
    assert(trace.take(rec));
    assert(rec.timestampMs==i && rec.commandId==i);
  }
  assert(!trace.take(rec));
  trace.reset();
  assert(trace.size()==0 && trace.overflows()==0);
  assert(strcmp(label(STOP_LOGICAL_INTENT),"STOP_LOGICAL_INTENT")==0);

  // R30H2 software timing scenario; 250ms nRF and 6000ms E22 freshness
  // are measured from accepted packets, NOT the exact moment RF is blocked.
  const uint32_t lastAccepted=1200U;
  assert((uint32_t)(lastAccepted+250U-lastAccepted)<=250U);
  assert((uint32_t)(lastAccepted+251U-lastAccepted)>250U);
  assert((uint32_t)(lastAccepted+6000U-lastAccepted)<=6000U);
  assert((uint32_t)(lastAccepted+6001U-lastAccepted)>6000U);
  assert(!boatRadioLossRthDue(true,true,true,true,true,lastAccepted,lastAccepted+9999U));
  assert( boatRadioLossRthDue(true,true,true,true,true,lastAccepted,lastAccepted+10000U));
  assert(!boatRadioLossRthDue(true,true,false,true,true,lastAccepted,lastAccepted+10000U));
  assert(!boatRadioLossRthDue(true,true,true,false,true,lastAccepted,lastAccepted+10000U));
  assert(!boatRadioLossRthDue(false,true,true,true,true,lastAccepted,lastAccepted+12000U));
  // Fresh backup before threshold resets the timer.
  assert(!boatRadioLossRthDue(true,true,true,true,true,lastAccepted+9000U,lastAccepted+10000U));
  puts("R30H-2 bounded Boat-local trace and 250/6000/10000ms policy simulation passed (NO_RF)");
  return 0;
}
