#include "nrf24_link.h"
#include <string.h>

static const uint8_t REMOTE_PIPE[5]={'R','M','T','E','1'};
static const uint8_t BOAT_PIPE[5]={'B','O','A','T','1'};

NRF24Link::NRF24Link(int ce,int csn,SPIClass *spi,uint8_t ch,int sck,int miso,int mosi)
 : radio_(ce,csn,1000000),spiBus_(spi),cePin_(ce),csnPin_(csn),channel_(ch),spiSck_(sck),spiMiso_(miso),spiMosi_(mosi),
   ready_(false),txPending_(false),lastTxSuccess_(false),txCount_(0),rxCount_(0),
   txFailCount_(0),rxHead_(0),rxTail_(0),rxQueueCount_(0),txAddress_(nullptr),rxAddress_(nullptr) {
 memset(rxQueue_,0,sizeof(rxQueue_));
}
bool NRF24Link::begin(bool remoteRole) {
 ready_=false;txPending_=false;lastTxSuccess_=false;
 rxHead_=rxTail_=rxQueueCount_=0;txStartedAtMs_=0;
 if(cePin_<0 || csnPin_<0 || !spiBus_) return false;
 txAddress_=remoteRole?BOAT_PIPE:REMOTE_PIPE;
 rxAddress_=remoteRole?REMOTE_PIPE:BOAT_PIPE;
 if(spiSck_>=0 && spiMiso_>=0 && spiMosi_>=0) spiBus_->begin(spiSck_,spiMiso_,spiMosi_,csnPin_);
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
   if(txOk||txFail){
     txPending_=false;lastTxSuccess_=txOk&&!txFail;
     if(txFail){++txFailCount_;radio_.flush_tx();}
     radio_.startListening();
   }
   if(txPending_ && (uint32_t)(millis()-txStartedAtMs_)>=TX_PENDING_TIMEOUT_MS){
     // Fail closed: uncertain TX completion may otherwise hold STOP/heartbeat.
     ++txFailCount_;txPending_=false;lastTxSuccess_=false;ready_=false;
     rxHead_=rxTail_=rxQueueCount_=0;
     radio_.flush_tx();radio_.startListening();
     return;
   }
 }
 // One physical RF24 FIFO contains at most three 32-byte frames.
 // Drain in a single scheduled poll and never re-enter hardware from
 // receiveFrame(), which is now a pure software-queue operation.
 if(txPending_)return;
 for(unsigned i=0;i<3U && rxQueueCount_<4U && radio_.available();++i){
   RadioFrame f{};
   radio_.read(&f,sizeof(f));
   if(f.magic[0]!=RADIO_FRAME_MAGIC0||f.magic[1]!=RADIO_FRAME_MAGIC1||
      radio_crc16((const uint8_t*)&f,offsetof(RadioFrame,crc16))!=f.crc16)
     continue;
   if(rxQueueCount_<4U){
     rxQueue_[rxTail_]=f;
     rxTail_=(uint8_t)((rxTail_+1U)%4U);
     ++rxQueueCount_;++rxCount_;
   }
 }
 if(rxQueueCount_==4U && rxFullPollCount_!=UINT32_MAX) ++rxFullPollCount_;
}
bool NRF24Link::sendFrame(const RadioFrame &frame) {
 if(!ready_ || txPending_ || frame.magic[0]!=RADIO_FRAME_MAGIC0 || frame.magic[1]!=RADIO_FRAME_MAGIC1) return false;
 radio_.stopListening();
 radio_.openWritingPipe(txAddress_);
 if(!radio_.startWrite(&frame,sizeof(frame),false)) {
   radio_.startListening();return false;
 }
 txStartedAtMs_=millis();
 txPending_=true;lastTxSuccess_=false;++txCount_;
 return true; // TX started; application ACK/boat state is still required
}
bool NRF24Link::receiveFrame(RadioFrame &frame) {
 // Main Boat radio scheduler calls update() once before draining the queue.
 // Never perform SPI transactions just because an application dequeued data.
 if(!ready_ || rxQueueCount_==0) return false;
 frame=rxQueue_[rxHead_];rxHead_=(uint8_t)((rxHead_+1U)%4U);--rxQueueCount_;return true;
}
