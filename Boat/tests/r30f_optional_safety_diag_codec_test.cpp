#include "../source/radio_payload_codec.h"
#include "../source/radio_payload_encoder.h"
#include "../source/radio_safety_diagnostic_v1.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static RadioDecodedPayload diag(uint8_t reason,uint16_t flags,uint16_t mv,uint8_t seq){
 RadioDecodedPayload d{};d.kind=RADIO_PAYLOAD_SAFETY_DIAGNOSTIC;
 d.diagnosticReason=reason;d.diagnosticFlags=flags;
 d.diagnosticMillivolts=mv;d.diagnosticSequence=seq;return d;
}
static void invalid(uint8_t p[8]){
 RadioDecodedPayload decoded{};
 assert(!radio_decodeTelemetryPayload(p,decoded));
 assert(decoded.kind==RADIO_PAYLOAD_INVALID);
}
int main(){
 uint8_t wire[8]{};
 // An uninstalled Boat is diagnostic-only and voltage is explicitly unknown.
 auto d=diag(1,0x0A02U,0xffffU,7);
 assert(radio_encodeTelemetryPayload(d,wire));
 const uint8_t expected[8]={8,1,1,2,10,255,255,7};
 assert(memcmp(wire,expected,8)==0);
 RadioDecodedPayload back{};
 assert(radio_decodeTelemetryPayload(wire,back));
 assert(back.kind==RADIO_PAYLOAD_SAFETY_DIAGNOSTIC);
 assert(back.diagnosticReason==1&&back.diagnosticFlags==0x0A02U);
 assert(back.diagnosticMillivolts==0xffffU&&back.diagnosticSequence==7);
 // A synthetic observation can have a valid mV, but NEVER be commissioned.
 d=diag(14,0x0803U,12000U,8);
 assert(radio_encodeTelemetryPayload(d,wire));
 assert(radio_decodeTelemetryPayload(wire,back));
 assert(back.diagnosticReason==14&&back.diagnosticFlags==0x0803U);
 assert(back.diagnosticMillivolts==12000U&&back.diagnosticSequence==8);
 const uint8_t healthyVector[8]={8,1,14,3,8,0xe0,0x2e,8};
 assert(memcmp(wire,healthyVector,8)==0);
 // R30E emitted a fault from a fresh observation. All faults are retained.
 d=diag(8,0x081fU,9900U,255);
 assert(radio_encodeTelemetryPayload(d,wire));
 assert(radio_decodeTelemetryPayload(wire,back));
 assert(back.diagnosticFlags==0x081fU && back.diagnosticSequence==255);
 // A new receiver MUST reject a forged 'commissioned' or old version frame.
 uint8_t bad[8]{};
 memcpy(bad,healthyVector,8);bad[0]=7;invalid(bad);
 memcpy(bad,healthyVector,8);bad[1]=2;invalid(bad);
 memcpy(bad,healthyVector,8);bad[2]=0;invalid(bad);
 memcpy(bad,healthyVector,8);bad[2]=15;invalid(bad);
 memcpy(bad,healthyVector,8);bad[3]=1;invalid(bad); // not UNVERIFIED
 memcpy(bad,healthyVector,8);bad[4]=0;invalid(bad); // commissioning lock absent
 memcpy(bad,healthyVector,8);bad[4]=0x18;invalid(bad); // unsupported flags
 memcpy(bad,healthyVector,8);bad[3]=2;invalid(bad); // observed fault without observed bit
 memcpy(bad,healthyVector,8);bad[5]=0xff;bad[6]=0xff;invalid(bad);
 memcpy(bad,expected,8);bad[3]=3;invalid(bad); // NO_READER cannot be observed
 memcpy(bad,expected,8);bad[5]=0xe0;bad[6]=0x2e;invalid(bad);
 d=diag(14,0x0003,12000,2);memset(wire,0x55,8);
 assert(!radio_encodeTelemetryPayload(d,wire));
 for(unsigned i=0;i<8;i++)assert(wire[i]==0); // invalid encoder clears
 d=diag(14,0x0803,8999,2);assert(!radio_encodeTelemetryPayload(d,wire));
 d=diag(14,0x0803,13001,2);assert(!radio_encodeTelemetryPayload(d,wire));
 d=diag(14,0x1803,12000,2);assert(!radio_encodeTelemetryPayload(d,wire));
 // Previous STATUS and BATTERY frames preserve their published wire formats.
 RadioDecodedPayload legacy{};legacy.kind=RADIO_PAYLOAD_STATUS;
 legacy.boatMode=0;legacy.stopState=1;legacy.batteryPercent=255;
 assert(radio_encodeTelemetryPayload(legacy,wire));
 assert(wire[0]==1);
 assert(radio_decodeTelemetryPayload(wire,back)&&back.kind==RADIO_PAYLOAD_STATUS);
 legacy={};legacy.kind=RADIO_PAYLOAD_BATTERY_SPEED;
 legacy.batteryMillivolts=12000;legacy.speedCmPerSecond=100;
 assert(radio_encodeTelemetryPayload(legacy,wire));
 assert(wire[0]==2);
 assert(radio_decodeTelemetryPayload(wire,back)&&back.kind==RADIO_PAYLOAD_BATTERY_SPEED);
 printf("R30F shared strict subtype-8 vectors: 080101020affff07 08010e0308e02e08 PASS\n");
}
