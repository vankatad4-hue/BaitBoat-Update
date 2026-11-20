#include "nrf24_link.h"
#include <string.h>

static const uint8_t REMOTE_PIPE[5]={'R','M','T','E','1'};
static const uint8_t BOAT_PIPE[5]={'B','O','A','T','1'};

NRF24Link::NRF24Link(int ce,int csn,SPIClass *spi,uint8_t ch)
 : radio_(ce,csn,1000000),spiBus_(spi),cePin_(ce),csnPin_(csn),channel_(ch),
   ready_(false),txPending_(false),lastTxSuccess_(false),txCount_(0),rxCount_(0),
   txFailCount_(0),rxAvailable_(false),txAddress_(nullptr),rxAddress_(nullptr) {
 memset(&rxFrame_,0,sizeof(rxFrame_));
}
bool NRF24Link::begin(bool remoteRole) {
 ready_=false;
 if(cePin_<0 || csnPin_<0 || !spiBus_) return false;
 txAddress_=remoteRole?BOAT_PIPE:REMOTE_PIPE;
 rxAddress_=remoteRole?REMOTE_PIPE:BOAT_PIPE;
 if(!radio_.begin(spiBus_)) return false;
 radio_.setChannel(channel_);
 radio_.setDataRate(RF24_250KBPS);
 radio_.setPALevel(RF24_PA_LOW); // initial bench-safe level; range/power must be measured
 radio_.setAutoAck(true);
 radio_.setRetries(5,15);
 radio_.setCRCLength(RF24_CRC_16);
 radio_.disableDynamicPayloads();
 radio_.setPayloadSize(RADIO_FRAME_SIZE);
 radio_.openWritingPipe(txAddress_);
 radio_.openReadingPipe(1,rxAddress_);
 radio_.startListening();
 ready_=radio_.isValid();
 return ready_;
}
void NRF24Link::update() {
 if(!ready_) return;
 if(txPending_) {
   bool txOk=false,txFail=false,rxReady=false;
   radio_.whatHappened(txOk,txFail,rxReady);
   if(txOk || txFail) {
     txPending_=false;lastTxSuccess_=txOk;
     if(txFail) { ++txFailCount_; radio_.flush_tx(); }
     radio_.startListening();
   }
   if(rxReady && radio_.available()) {
     RadioFrame f; radio_.read(&f,sizeof(f));
     if(f.magic[0]==RADIO_FRAME_MAGIC0 && f.magic[1]==RADIO_FRAME_MAGIC1 &&
       radio_crc16((const uint8_t*)&f,offsetof(RadioFrame,crc16))==f.crc16) {
       rxFrame_=f;rxAvailable_=true;++rxCount_;
     }
   }
 }
 if(!txPending_ && radio_.available()) {
   RadioFrame f; radio_.read(&f,sizeof(f));
   if(f.magic[0]==RADIO_FRAME_MAGIC0 && f.magic[1]==RADIO_FRAME_MAGIC1 &&
     radio_crc16((const uint8_t*)&f,offsetof(RadioFrame,crc16))==f.crc16) {
     rxFrame_=f;rxAvailable_=true;++rxCount_;
   }
 }
}
bool NRF24Link::sendFrame(const RadioFrame &frame) {
 if(!ready_ || txPending_ || frame.magic[0]!=RADIO_FRAME_MAGIC0 || frame.magic[1]!=RADIO_FRAME_MAGIC1) return false;
 radio_.stopListening();
 radio_.openWritingPipe(txAddress_);
 if(!radio_.startWrite(&frame,sizeof(frame),false)) {
   radio_.startListening();return false;
 }
 txPending_=true;lastTxSuccess_=false;++txCount_;
 return true; // TX started; application ACK/boat state is still required
}
bool NRF24Link::receiveFrame(RadioFrame &frame) {
 update();
 if(!rxAvailable_) return false;
 frame=rxFrame_;rxAvailable_=false;return true;
}
