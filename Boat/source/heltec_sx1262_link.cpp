#include "heltec_sx1262_link.h"
HeltecSx1262Link::HeltecSx1262Link()
 :module_(BOAT_SX1262_NSS,BOAT_SX1262_DIO1,BOAT_SX1262_RST,
          BOAT_SX1262_BUSY,SPI,SPISettings(2000000,MSBFIRST,SPI_MODE0)),
  radio_(&module_){}
bool HeltecSx1262Link::begin(){
 ready_=false;txPending_=false;head_=tail_=count_=0;
#if BOAT_SX1262_BOARD_AUDIT_SIGNED && BOAT_SX1262_PEER_PHY_VERIFIED && BOAT_SX1262_EU868_APPROVED
 if(BOAT_SX1262_NSS<0||BOAT_SX1262_SCK<0||BOAT_SX1262_MISO<0||
    BOAT_SX1262_MOSI<0||BOAT_SX1262_RST<0||BOAT_SX1262_BUSY<0||
    BOAT_SX1262_DIO1<0)return false;
 // Provisional test PHY, not an approved RF power/band profile.
 if(radio_.begin(868.1,125.0,9,5,0x12,10,8,1.8,false)!=RADIOLIB_ERR_NONE)
  return false;
 if(radio_.setDio2AsRfSwitch(true)!=RADIOLIB_ERR_NONE)return false;
 ready_=radio_.startReceive()==RADIOLIB_ERR_NONE;
#endif
 return ready_;
}
void HeltecSx1262Link::update(){
 if(!ready_)return;
 const uint32_t now=millis();
 if(txPending_ && (uint32_t)(now-started_)>3000U){
  ready_=false;txPending_=false;return;
 }
 if(digitalRead(BOAT_SX1262_DIO1)!=HIGH)return;
 if(txPending_){
  txPending_=false;
  const bool finished=radio_.finishTransmit()==RADIOLIB_ERR_NONE;
  ready_=finished && (radio_.startReceive()==RADIOLIB_ERR_NONE);
  return;
 }
 const size_t n=radio_.getPacketLength();
 if(n==RADIO_FRAME_SIZE){
  RadioFrame f{};
  if(radio_.readData((uint8_t*)&f,sizeof(f))==RADIOLIB_ERR_NONE &&
     f.magic[0]==RADIO_FRAME_MAGIC0&&f.magic[1]==RADIO_FRAME_MAGIC1&&
     radio_crc16((const uint8_t*)&f,offsetof(RadioFrame,crc16))==f.crc16 &&
     count_<4){
   queue_[tail_]=f;tail_=(uint8_t)((tail_+1U)%4);++count_;
  }
 }else radio_.finishReceive();
 ready_=radio_.startReceive()==RADIOLIB_ERR_NONE;
}
bool HeltecSx1262Link::sendFrame(const RadioFrame &f,uint32_t now){
 update();
 if(!ready_||txPending_||f.magic[0]!=RADIO_FRAME_MAGIC0||
    f.magic[1]!=RADIO_FRAME_MAGIC1)return false;
 const uint32_t air=sx1262AirtimeMs(RADIO_FRAME_SIZE,9,125000,1);
 if(!limiter_.allows(now,air))return false;
 if(radio_.startTransmit((const uint8_t*)&f,sizeof(f))!=RADIOLIB_ERR_NONE)
  return false;
 limiter_.reserve(now,air);
 started_=now;txPending_=true;return true;
}
bool HeltecSx1262Link::receiveFrame(RadioFrame &f){
 update();
 if(!count_)return false;
 f=queue_[head_];head_=(uint8_t)((head_+1U)%4);--count_;return true;
}
