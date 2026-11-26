#include "recovery_probe_wire.h"
#include <cassert>
#include <cstdint>

int main() {
  using flytec_rtos::RecoveryProbeKind;
  uint8_t payload[8]{};
  uint16_t decoded = 0;

  assert(flytec_rtos::recoveryProbeEncode(RecoveryProbeKind::Challenge, 0x1234, payload));
  assert(payload[0] == 0x52 && payload[1] == 0x43 && payload[2] == 1);
  assert(payload[3] == uint8_t(RecoveryProbeKind::Challenge));
  assert(payload[4] == 0x34 && payload[5] == 0x12 && payload[6] == 0 && payload[7] == 0);
  assert(flytec_rtos::recoveryProbeDecode(payload, RecoveryProbeKind::Challenge, decoded));
  assert(decoded == 0x1234);
  assert(!flytec_rtos::recoveryProbeDecode(payload, RecoveryProbeKind::Echo, decoded));

  assert(flytec_rtos::recoveryProbeEncode(RecoveryProbeKind::Echo, 0xBEEF, payload));
  assert(flytec_rtos::recoveryProbeDecode(payload, RecoveryProbeKind::Echo, decoded));
  assert(decoded == 0xBEEF);
  assert(!flytec_rtos::recoveryProbeEncode(RecoveryProbeKind::Echo, 0, payload));
  assert(!flytec_rtos::recoveryProbeEncode(static_cast<RecoveryProbeKind>(0), 1, payload));

  assert(flytec_rtos::recoveryProbeEncode(RecoveryProbeKind::Echo, 1, payload));
  payload[0] ^= 1;
  assert(!flytec_rtos::recoveryProbeDecode(payload, RecoveryProbeKind::Echo, decoded));
  assert(flytec_rtos::recoveryProbeEncode(RecoveryProbeKind::Echo, 1, payload));
  payload[6] = 1;
  assert(!flytec_rtos::recoveryProbeDecode(payload, RecoveryProbeKind::Echo, decoded));
}
