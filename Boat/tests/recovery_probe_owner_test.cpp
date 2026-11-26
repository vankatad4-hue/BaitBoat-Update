#include "radio_protocol.h"
#include "boat_radio_services.h"
#include <cassert>

using namespace flytec_r30h118;
using namespace flytec_r30h120;
using namespace flytec_r30h121;
using namespace flytec_r30h123;
using namespace flytec_rtos;

int main() {
  Transport<RadioFrame> bus;
  PhyEvents<RadioFrame> events;
  AppRadio<RadioFrame> app(bus, events);
  HandshakeLane<RadioFrame> handshake;
  SharedPriority priority;
  OwnerLink<RadioFrame> owner(app, handshake, events, priority);
  assert(owner.setSession(1));

  auto verify = [](const RadioFrame &) { return true; };
  auto makeEcho = [&](RadioFrame &frame) {
    frame.type = RADIO_MSG_LINK_PROBE;
    frame.sourceId = 2;
    frame.destinationId = 1;
    frame.sequence = 1;
    frame.sessionEpoch = 1;
    return recoveryProbeEncode(RecoveryProbeKind::Echo, 77, frame.payload);
  };

  RadioFrame frame{};
  assert(makeEcho(frame));

  RadioFrame malformed = frame;
  malformed.flags = 1;
  assert(!owner.submitFeedback(malformed, 10, TxPriority::Heartbeat, 100,
                               verify));

  malformed = frame;
  malformed.commandId = 9;
  assert(!owner.submitFeedback(malformed, 10, TxPriority::Heartbeat, 100,
                               verify));

  malformed = frame;
  assert(recoveryProbeEncode(RecoveryProbeKind::Challenge, 77,
                             malformed.payload));
  assert(!owner.submitFeedback(malformed, 10, TxPriority::Heartbeat, 100,
                               verify));

  assert(owner.submitFeedback(frame, 10, TxPriority::Heartbeat, 100, verify));
}
