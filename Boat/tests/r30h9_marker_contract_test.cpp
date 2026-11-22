#include "../source/r30h9_marker_contract.h"
#include "../source/boat_r30h9_marker_config.h"
#include <assert.h>
#include <stdio.h>

int main() {
 using namespace flytec_r30h9;
 static_assert(BOAT_R30H9_MARKER_GPIO == -1, "No Boat GPIO allocated");
 static_assert(BOAT_R30H9_GPIO_OUTPUT_SIGNED == 0, "No physical marker approval");
 LocalRecorder trace;
 assert(trace.size()==0 && trace.dropped()==0);
 assert(trace.push(Endpoint::BOAT,Event::BOAT_AUTH_FRAME,4294967294U,17));
 assert(trace.push(Endpoint::BOAT,Event::BOAT_AUTH_STOP,4294967295U,17));
 assert(trace.push(Endpoint::BOAT,Event::BOAT_STOP_INTENT,0U,17));
 assert(trace.push(Endpoint::BOAT,Event::BOAT_RTH_ENTRY,1U));
 Record r{};
 assert(trace.pop(r) && r.localMonotonicMs==4294967294U && r.commandId==17);
 assert(r.event==Event::BOAT_AUTH_FRAME && r.endpoint==Endpoint::BOAT);
 assert(r.evidence==Evidence::SOFTWARE_LOCAL_UNCERTIFIED);
 assert(trace.pop(r) && r.localMonotonicMs==4294967295U);
 assert(trace.pop(r) && r.localMonotonicMs==0U);
 assert(trace.pop(r) && r.localMonotonicMs==1U);
 assert(!trace.pop(r));
 for(int i=0;i<LocalRecorder::CAPACITY;++i)assert(trace.push(Endpoint::BOAT,Event::BOAT_RTH_ENTRY,(uint32_t)i));
 assert(!trace.push(Endpoint::BOAT,Event::BOAT_RTH_ENTRY,99) && trace.dropped()==1);
 assert(trace.size()==LocalRecorder::CAPACITY);
 trace.reset();assert(trace.size()==0 && trace.dropped()==0);

 for(int pin : {1,2,41,42}) {
  assert(isBoatReservedForSafety(pin));
  assert(!currentlyApprovedOutput(Endpoint::BOAT,pin));
 }
 for(int pin : {4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,21,39,40}) {
  assert(isBoatAllocated(pin));
  assert(!currentlyApprovedOutput(Endpoint::BOAT,pin));
 }
 for(int pin : {0,3,19,20,35,36,37,38,43,44,45,46,47,48}) {
  assert(isBoatFlashOrDangerous(pin));
  assert(!currentlyApprovedOutput(Endpoint::BOAT,pin));
 }
 PhysicalGate gates{};
 assert(!prerequisitesDocumented(gates));
 gates.boardRevisionInspected=true;
 gates.actualHeaderContinuityMeasured=true;
 gates.pinConflictAuditSigned=true;
 gates.ioVoltageMeasured=true;
 gates.actuatorSupplyVmVhDisconnected=true;
 gates.radioAntennaAndBandApproved=true;
 gates.trueExternalCommonClockPresent=true;
 gates.eventToEdgeLatencyCharacterized=true;
 assert(!prerequisitesDocumented(gates));
 gates.independentReviewerSigned=true;
 assert(prerequisitesDocumented(gates));
 assert(!currentlyApprovedOutput(Endpoint::BOAT,1)); // tests NEVER sign a GPIO
 printf("R30H-9 Boat NO_GPIO marker recorder, safety reservations and unsigned nine-gate policy PASS (host only)\n");
}
