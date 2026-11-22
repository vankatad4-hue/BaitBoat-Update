#include "../source/radio_command_codec.h"
#include <cassert>

// Host-only cryptography stubs. Frame authentication is covered by separate
// session/HMAC tests; this verifies the Remote command decoder's link policy.
void radio_frameInit(RadioFrame &f,RadioMessageType type,uint8_t src,uint8_t dst,
                     uint16_t seq,uint16_t commandId,uint32_t now,uint8_t epoch){
 f={};f.type=(uint8_t)type;f.sourceId=src;f.destinationId=dst;
 f.sequence=seq;f.commandId=commandId;f.tick16=(uint16_t)now;f.sessionEpoch=epoch;
}
bool radio_frameSeal(RadioFrame &,const uint8_t[32]){return true;}
bool radio_frameVerify(const RadioFrame &,const uint8_t[32]){return true;}
bool radio_tickFresh(uint16_t,uint16_t,uint16_t){return false;}
int main(){
 const uint8_t key[32]={};RadioFrame frame={};BoatCommand got={};
 BoatCommand cmd={};cmd.commandId=1;cmd.sequence=2;cmd.createdAtMs=10;
 cmd.type=CMD_MANUAL_MOTORS;cmd.mode=MODE_MANUAL;cmd.leftMotor=900;
 assert(radio_encodeCommand(cmd,1,2,3,frame,key));
 assert(!radio_decodeCommand(frame,1,2,20,2500,true,got,key));
 assert(radio_decodeCommand(frame,1,2,20,2500,false,got,key));
 assert(got.leftMotor==900);
 cmd.type=CMD_EMERGENCY_STOP;cmd.leftMotor=0;
 assert(radio_encodeCommand(cmd,1,2,3,frame,key));
 assert(radio_decodeCommand(frame,1,2,20,2500,true,got,key));
 cmd.type=CMD_SET_MODE;cmd.mode=MODE_RETURN_HOME;
 assert(radio_encodeCommand(cmd,1,2,3,frame,key));
 assert(radio_decodeCommand(frame,1,2,20,2500,true,got,key));
 cmd.mode=MODE_AUTOPILOT;
 assert(radio_encodeCommand(cmd,1,2,3,frame,key));
 assert(!radio_decodeCommand(frame,1,2,20,2500,true,got,key));
 cmd.type=CMD_RELEASE_HOOK_LEFT;cmd.mode=MODE_MANUAL;cmd.activate=true;
 assert(radio_encodeCommand(cmd,1,2,3,frame,key));
 assert(!radio_decodeCommand(frame,1,2,20,2500,true,got,key));
}
