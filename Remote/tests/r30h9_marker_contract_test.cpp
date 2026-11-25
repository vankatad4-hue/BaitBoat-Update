#include "../source/diagnostic_marker_contract.h"
#include "../source/remote_diagnostic_marker_config.h"
#include <assert.h>
#include <stdio.h>
#include <initializer_list>

int main() {
 using namespace flytec_r30h9;
 static_assert(REMOTE_R30H9_MARKER_GPIO == -1, "No Remote GPIO assigned");
 static_assert(REMOTE_R30H9_GPIO_OUTPUT_SIGNED == 0, "No physical marker approval");
 LocalRecorder recorder;
 assert(recorder.push(Endpoint::REMOTE,Event::REMOTE_STOP_REQUEST,100,3));
 assert(recorder.push(Endpoint::REMOTE,Event::REMOTE_NRF_TX_ADMITTED,101,3));
 assert(recorder.push(Endpoint::REMOTE,Event::REMOTE_E22_UART_ADMITTED,103,3));
 Record r{};
 assert(recorder.pop(r)&&r.event==Event::REMOTE_STOP_REQUEST&&r.endpoint==Endpoint::REMOTE);
 assert(r.evidence==Evidence::SOFTWARE_LOCAL_UNCERTIFIED&&r.commandId==3);
 assert(recorder.pop(r)&&r.event==Event::REMOTE_NRF_TX_ADMITTED);
 assert(recorder.pop(r)&&r.event==Event::REMOTE_E22_UART_ADMITTED);
 assert(!recorder.pop(r));
 for(int pin : {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,35,36,37,38,39,40,41,42,43,44,45,46,47,48})
  assert(isRemoteKnownOccupiedOrRestricted(pin) &&
         !currentlyApprovedOutput(Endpoint::REMOTE,pin));
 for(int pin : {22,23,24,25,26,27,28,29,30,31,32,33,34})
  assert(!currentlyApprovedOutput(Endpoint::REMOTE,pin));
 PhysicalGate signedInHost{};
 assert(!prerequisitesDocumented(signedInHost));
 printf("R30H-9 Remote NO_GPIO marker recorder and full Waveshare pin denylist PASS (host only)\n");
}
