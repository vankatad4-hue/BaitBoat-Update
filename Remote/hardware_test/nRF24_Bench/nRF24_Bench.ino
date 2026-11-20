// nRF24 bench link test only. No authenticated commands, motors or solenoids.
#include <Arduino.h>
#include <SPI.h>
#include "../../source/nrf24_link.h"
#include "../../source/radio_protocol.h"
#include "../../source/radio_protocol.cpp"
#include "../../source/nrf24_link.cpp"

NRF24Link nrf(6,15,&SPI,76,12,13,11); // CE=GPIO6, CSN=GPIO15; isolate RS485 and Sensor AD
uint16_t sequence=0;uint32_t lastTx=0;
void setup(){
 Serial.begin(115200);
 // GPIO11 MOSI,12 SCK,13 MISO; shared with TF card. Test SD bus arbitration separately.
 SPI.begin(12,13,11,15);
 if(!nrf.begin(true)){Serial.println("nRF init failed; verify module, GPIO and shared SPI");return;}
 Serial.println("nRF primary bench: channel76, 250kbps, PA_LOW, fixed32, no actuators.");
}
void loop(){
 nrf.update();RadioFrame rx;
 while(nrf.receiveFrame(rx))Serial.printf("RX src=%u dst=%u seq=%u crc-ok payload=%c%c%c%c\n",rx.sourceId,rx.destinationId,rx.sequence,rx.payload[0],rx.payload[1],rx.payload[2],rx.payload[3]);
 if(nrf.ready() && millis()-lastTx>=1000){
  lastTx=millis();RadioFrame tx;
  radio_frameInit(tx,RADIO_MSG_LINK_PROBE,0x01,0x02,++sequence,0,millis(),0);
  tx.payload[0]='N';tx.payload[1]='R';tx.payload[2]='F';tx.payload[3]='T';
  tx.crc16=radio_crc16((const uint8_t*)&tx,offsetof(RadioFrame,crc16));
  if(nrf.sendFrame(tx))Serial.printf("TX started #%u\n",sequence);
 }
 delay(1);
}
