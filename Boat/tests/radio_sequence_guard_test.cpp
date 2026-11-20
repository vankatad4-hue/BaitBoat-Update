#include "../source/radio_command_codec.h"
#include <assert.h>

bool radio_frameVerify(const RadioFrame &, const uint8_t[32]) { return true; }
bool radio_tickFresh(uint16_t, uint16_t, uint16_t) { return true; }
void radio_frameInit(RadioFrame &f, RadioMessageType type, uint8_t src, uint8_t dst, uint16_t seq, uint16_t cmdId, uint32_t now, uint8_t epoch) {\n  f={}; f.type=(uint8_t)type; f.sourceId=src; f.destinationId=dst; f.sequence=seq; f.commandId=cmdId; f.tick16=(uint16_t)now; f.sessionEpoch=epoch;\n}\nbool radio_frameSeal(RadioFrame &, const uint8_t[32]) { return true; }

int main() {
  RadioSequenceGuard g;
  assert(g.accept(7,65534,true));
  assert(g.accept(7,65535));
  assert(g.accept(7,0)); // uint16 wrap
  assert(!g.accept(7,0)); // duplicate
  assert(!g.accept(7,65535)); // old sequence
  assert(!g.accept(8,1)); // epoch changes not permitted implicitly
  assert(g.accept(8,1,true)); // explicit session reset
  assert(!g.accept(7,2)); // old epoch rejected
  g.reset();
  assert(g.accept(3,10));
  assert(g.accept(3,12)); // forward gap permitted
  assert(!g.accept(3,11)); // out-of-order packet rejected
  return 0;
}
