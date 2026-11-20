// Boat E22 bench-only RF test. No motors, solenoids, or actuator commands.
#include <Arduino.h>
#include <HardwareSerial.h>
#include "../../source/e22_uart_link.h"
#include "../../source/radio_protocol.h"
#include "../../source/radio_protocol.cpp"
#include "../../source/e22_uart_link.cpp"

#ifndef TEST_E22_RX_PIN
#define TEST_E22_RX_PIN -1
#endif
#ifndef TEST_E22_TX_PIN
#define TEST_E22_TX_PIN -1
#endif
#ifndef TEST_E22_M0_PIN
#define TEST_E22_M0_PIN -1
#endif
#ifndef TEST_E22_M1_PIN
#define TEST_E22_M1_PIN -1
#endif
#ifndef TEST_E22_AUX_PIN
#define TEST_E22_AUX_PIN -1
#endif
#ifndef TEST_E22_RESET_PIN
#define TEST_E22_RESET_PIN -1
#endif
HardwareSerial e22Serial(1);
E22UartLink lora(e22Serial,
 [](bool m0,bool m1,void*){if(TEST_E22_M0_PIN>=0)digitalWrite(TEST_E22_M0_PIN,m0?HIGH:LOW);if(TEST_E22_M1_PIN>=0)digitalWrite(TEST_E22_M1_PIN,m1?HIGH:LOW);},
 [] (void*){return TEST_E22_AUX_PIN>=0 && digitalRead(TEST_E22_AUX_PIN)==HIGH;},
 [](bool released,void*){if(TEST_E22_RESET_PIN>=0)digitalWrite(TEST_E22_RESET_PIN,released?HIGH:LOW);},nullptr);
uint16_t seq=0;uint32_t lastTx=0;
void setup(){
 Serial.begin(115200);
 if(TEST_E22_RX_PIN<0 || TEST_E22_TX_PIN<0 || TEST_E22_M0_PIN<0 || TEST_E22_M1_PIN<0 || TEST_E22_AUX_PIN<0){
  Serial.println("Set test pins from exact Tracker board schematic first; radio disabled.");return;
 }
 pinMode(TEST_E22_M0_PIN,OUTPUT);pinMode(TEST_E22_M1_PIN,OUTPUT);pinMode(TEST_E22_AUX_PIN,INPUT);
 if(TEST_E22_RESET_PIN>=0){pinMode(TEST_E22_RESET_PIN,OUTPUT);digitalWrite(TEST_E22_RESET_PIN,HIGH);}
 e22Serial.begin(9600,SERIAL_8N1,TEST_E22_RX_PIN,TEST_E22_TX_PIN);
 if(!lora.configureEU868TestProfile()){Serial.println("E22 configuration/readback failed");return;}
 Serial.println("E22 configured. No actuators connected.");
}
void loop(){
 lora.update();RadioFrame rx;
 while(lora.receiveFrame(rx)){
  Serial.printf("RX test frame src=%u dst=%u seq=%u crc-ok; payload=%02X %02X %02X %02X\n",
   rx.sourceId,rx.destinationId,rx.sequence,rx.payload[0],rx.payload[1],rx.payload[2],rx.payload[3]);
 }
 if(lora.ready() && millis()-lastTx>=7500){
  lastTx=millis();RadioFrame tx;
  radio_frameInit(tx,RADIO_MSG_LINK_PROBE,0x02,0x01,++seq,0,millis(),0);
  tx.payload[0]='B';tx.payload[1]='O';tx.payload[2]='A';tx.payload[3]='T';
  tx.crc16=radio_crc16((const uint8_t*)&tx,offsetof(RadioFrame,crc16));
  if(lora.sendFrame(tx,millis()))Serial.printf("TX test frame #%u\n",seq);
  else Serial.println("TX skipped (AUX or duty budget)");
 }
 delay(1);
}
