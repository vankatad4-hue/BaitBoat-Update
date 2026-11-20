// E22 + SC16IS750 bench-only RF test. No motor/solenoid command protocol.
#include <Arduino.h>
#include <Wire.h>
#include "../../source/sc16is750_bridge.h"
#include "../../source/e22_uart_link.h"
#include "../../source/radio_protocol.h"
#include "../../source/radio_protocol.cpp"
#include "../../source/sc16is750_bridge.cpp"
#include "../../source/e22_uart_link.cpp"

SC16IS750Bridge bridge(Wire,0x48);
E22UartLink lora(bridge,
 [](bool m0,bool m1,void*){bridge.setE22Mode(m0,m1);},
 [](void*){return bridge.e22AuxReady();},
 [](bool released,void*){bridge.setE22Reset(released);},nullptr);
uint16_t seq=0; uint32_t lastTx=0;
void setup(){
 Serial.begin(115200);
 Wire.begin(8,9);
 Serial.println("Remote E22 bench test: no actuators; test frame is unauthenticated.");
 if(!bridge.begin(9600)){Serial.println("SC16IS750 init failed");return;}
 if(!lora.configureEU868TestProfile()){Serial.println("E22 config/readback failed");return;}
 Serial.println("E22 configured/read back. Output 10dBm, channel 18, 32-byte packets.");
}
void loop(){
 lora.update();
 RadioFrame rx;
 while(lora.receiveFrame(rx)){
  Serial.printf("RX test frame src=%u dst=%u seq=%u crc-ok; payload=%02X %02X %02X %02X\n",
   rx.sourceId,rx.destinationId,rx.sequence,rx.payload[0],rx.payload[1],rx.payload[2],rx.payload[3]);
 }
 if(lora.ready() && millis()-lastTx>=5000){
  lastTx=millis();
  RadioFrame tx;
  radio_frameInit(tx,RADIO_MSG_LINK_PROBE,0x01,0x02,++seq,0,millis(),0);
  tx.payload[0]='T';tx.payload[1]='E';tx.payload[2]='S';tx.payload[3]='T';
  tx.crc16=radio_crc16((const uint8_t*)&tx,offsetof(RadioFrame,crc16));
  if(lora.sendFrame(tx,millis())) Serial.printf("TX test frame #%u\n",seq);
  else Serial.println("TX not ready/duty budget; skipped");
 }
 delay(1);
}
