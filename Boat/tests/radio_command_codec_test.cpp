#include "../source/radio_command_codec.h"
#include <assert.h>

// Host stubs: test the command codec policy without cryptography or transport.
void radio_frameInit(RadioFrame &f, RadioMessageType type, uint8_t src, uint8_t dst, uint16_t seq, uint16_t cmdId, uint32_t now, uint8_t epoch) {\n  f={}; f.type=(uint8_t)type; f.sourceId=src; f.destinationId=dst; f.sequence=seq; f.commandId=cmdId; f.tick16=(uint16_t)now; f.sessionEpoch=epoch;\n}\nbool radio_frameSeal(RadioFrame &, const uint8_t[32]) { return true; }
bool radio_frameVerify(const RadioFrame &, const uint8_t[32]) { return true; }
bool radio_tickFresh(uint16_t, uint16_t, uint16_t) { return true; }

int main() {
  uint8_t key[32]={}; RadioFrame f={}; BoatCommand out={};
  BoatCommand c={}; c.commandId=77; c.sequence=12; c.createdAtMs=100;
  c.type=CMD_MANUAL_MOTORS; c.mode=MODE_MANUAL; c.leftMotor=900; c.rightMotor=-900;
  assert(radio_encodeCommand(c,1,2,9,f,key));
  assert(radio_decodeCommand(f,1,2,200,2500,true,out,key));
  assert(out.leftMotor==250 && out.rightMotor==-250); // backup throttle clamp
  assert(radio_decodeCommand(f,1,2,200,2500,false,out,key)); // primary unrestricted by backup clamp

  c.type=CMD_CLEAR_STOP; c.activate=false;
  assert(radio_encodeCommand(c,1,2,9,f,key));
  assert(!radio_decodeCommand(f,1,2,200,2500,true,out,key));
  assert(radio_decodeCommand(f,1,2,200,2500,false,out,key));

  c.type=CMD_SET_MODE; c.mode=MODE_AUTOPILOT;
  assert(radio_encodeCommand(c,1,2,9,f,key));
  assert(!radio_decodeCommand(f,1,2,200,2500,true,out,key));
  c.mode=MODE_RETURN_HOME;
  assert(radio_encodeCommand(c,1,2,9,f,key));
  assert(radio_decodeCommand(f,1,2,200,2500,true,out,key));

  c.type=CMD_RELEASE_HOOK_LEFT; c.activate=true;
  assert(radio_encodeCommand(c,1,2,9,f,key));
  assert(!radio_decodeCommand(f,1,2,200,2500,true,out,key));
  assert(radio_decodeCommand(f,1,2,200,2500,false,out,key));

  c.type=CMD_HOOK_RELEASE; c.activate=true;
  assert(radio_encodeCommand(c,1,2,9,f,key));
  assert(!radio_decodeCommand(f,1,2,200,2500,false,out,key)); // legacy combined release forbidden
  return 0;
}
