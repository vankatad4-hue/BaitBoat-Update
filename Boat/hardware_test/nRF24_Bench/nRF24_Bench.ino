// nRF24 bench link test only. No actuator commands or motor outputs.
#include <Arduino.h>
#include <SPI.h>
#include "../../source/nrf24_link.h"
#include "../../source/radio_protocol.h"
#include "../../source/radio_protocol.cpp"
#include "../../source/nrf24_link.cpp"

#ifndef TEST_NRF_CE_PIN
#define TEST_NRF_CE_PIN -1
#endif
#ifndef TEST_NRF_CSN_PIN
#define TEST_NRF_CSN_PIN -1
#endif
NRF24Link nrf(TEST_NRF_CE_PIN,TEST_NRF_CSN_PIN,&SPI,76);
uint16_t sequence=0;uint32_t lastTx=0;
void setup(){
 Serial.begin(115200);
 if(TEST_NRF_CE_PIN<0||TEST_NRF_CSN_PIN<0){Serial.println("Set CE/CSN after Tracker pin audit; disabled.");return;}
 SPI.begin();
 if(!nrf.begin(false)){Serial.println("nRF init failed");return;}
 Serial.println("nRF primary bench channel76,250kbps,PA_LOW; no actuators.");
}
void loop(){
 nrf.update();RadioFrame rx;
 while(nrf.receiveFrame(rx))Serial.printf("RX src=%u dst=%u seq=%u crc-ok payload=%c%c%c%c\n",rx.sourceId,rx.destinationId,rx.sequence,rx.payload[0],rx.payload[1],rx.payload[2],rx.payload[3]);
 if(nrf.ready()&&millis()-lastTx>=1500){
  lastTx=millis();RadioFrame tx;
  radio_frameInit(tx,RADIO_MSG_LINK_PROBE,0x02,0x01,++sequence,0,millis(),0);
  tx.payload[0]='B';tx.payload[1]='O';tx.payload[2]='A';tx.payload[3]='T';
  tx.crc16=radio_crc16((const uint8_t*)&tx,offsetof(RadioFrame,crc16));
  if(nrf.sendFrame(tx))Serial.printf("TX started #%u\n",sequence);
 }
 delay(1);
}
