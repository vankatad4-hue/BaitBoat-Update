#include "sc18is606_nrf_link.h"
#include <string.h>

namespace {
const uint8_t REMOTE_PIPE[5]={'R','M','T','E','1'};
const uint8_t BOAT_PIPE[5]={'B','O','A','T','1'};
constexpr uint8_t REG_CONFIG=0x00,REG_EN_AA=0x01,REG_EN_RXADDR=0x02;
constexpr uint8_t REG_SETUP_AW=0x03,REG_SETUP_RETR=0x04,REG_RF_CH=0x05;
constexpr uint8_t REG_RF_SETUP=0x06,REG_STATUS=0x07,REG_FIFO_STATUS=0x17;
constexpr uint8_t REG_RX_ADDR_P0=0x0a,REG_RX_ADDR_P1=0x0b,REG_TX_ADDR=0x10;
constexpr uint8_t REG_RX_PW_P0=0x11,REG_RX_PW_P1=0x12;
constexpr uint8_t REG_DYNPD=0x1c,REG_FEATURE=0x1d;
constexpr uint8_t CMD_W_TX=0xa0,CMD_R_RX=0x61,CMD_FLUSH_TX=0xe1,CMD_FLUSH_RX=0xe2;
constexpr uint8_t STAT_RX_DR=0x40,STAT_TX_DS=0x20,STAT_MAX_RT=0x10;
constexpr uint8_t CONFIG_RX=0x0f,CONFIG_TX=0x0e; // 16-bit CRC, PWR_UP
constexpr uint32_t TX_STUCK_MS=90;
}
Sc18is606NrfLink::Sc18is606NrfLink(Sc18is606Io&io,Mcp23017CePort&ce,uint8_t addr,uint8_t channel)
    :port_(io,ce,addr),channel_(channel){}
void Sc18is606NrfLink::trip(){
    ready_=false;txPending_=false;lastTxSuccess_=false;
    port_.disable(); // never authorize new traffic on an uncertain bridge
}
bool Sc18is606NrfLink::regWrite(uint8_t reg,uint8_t value){
    const uint8_t tx[2]={(uint8_t)(0x20|(reg&0x1f)),value};
    uint8_t rx[2]={};
    return port_.transfer(tx,rx,2);
}
bool Sc18is606NrfLink::regRead(uint8_t reg,uint8_t&value){
    const uint8_t tx[2]={(uint8_t)(reg&0x1f),0xff};
    uint8_t rx[2]={};
    if(!port_.transfer(tx,rx,2))return false;
    value=rx[1];return true;
}
bool Sc18is606NrfLink::regWriteBytes(uint8_t reg,const uint8_t*data,size_t count){
    if(!data||count==0||count>5)return false;
    uint8_t tx[6]={(uint8_t)(0x20|(reg&0x1f))},rx[6]={};
    memcpy(tx+1,data,count);
    return port_.transfer(tx,rx,count+1);
}
bool Sc18is606NrfLink::command(uint8_t code){
    uint8_t rx=0;
    return port_.transfer(&code,&rx,1);
}
bool Sc18is606NrfLink::pollStatus(uint8_t&status){
    const uint8_t nop=0xff;
    return port_.transfer(&nop,&status,1) && (status&0x80U)==0U;
}
bool Sc18is606NrfLink::listen(){
    if(!port_.ce(false)||!regWrite(REG_CONFIG,CONFIG_RX)){trip();return false;}
    delayMicroseconds(150);
    if(!port_.ce(true)){trip();return false;}
    return true;
}
bool Sc18is606NrfLink::begin(bool remoteRole){
    ready_=false;txPending_=false;lastTxSuccess_=false;
    rxHead_=rxTail_=rxCountQueued_=0;txCount_=rxCount_=txFailCount_=0;
    if(channel_>125||!port_.begin())return false;
    txAddress_=remoteRole?BOAT_PIPE:REMOTE_PIPE;
    rxAddress_=remoteRole?REMOTE_PIPE:BOAT_PIPE;
    // Mirror the existing RF24 wire profile used on Boat.
    // Local RF_PA_LOW / 250 kbit/sec / fixed 32 bytes / 16-bit CRC.
    if(!port_.ce(false)||
       !regWrite(REG_CONFIG,0x0c)||
       !regWrite(REG_EN_AA,0x03)||
       !regWrite(REG_EN_RXADDR,0x03)||
       !regWrite(REG_SETUP_AW,0x03)||
       !regWrite(REG_SETUP_RETR,0x5f)||
       !regWrite(REG_RF_CH,channel_)||
       !regWrite(REG_RF_SETUP,0x22)||
       !regWrite(REG_RX_PW_P0,(uint8_t)RADIO_FRAME_SIZE)||
       !regWrite(REG_RX_PW_P1,(uint8_t)RADIO_FRAME_SIZE)||
       !regWrite(REG_DYNPD,0)||
       !regWrite(REG_FEATURE,0)||
       !regWriteBytes(REG_TX_ADDR,txAddress_,5)||
       !regWriteBytes(REG_RX_ADDR_P0,txAddress_,5)||
       !regWriteBytes(REG_RX_ADDR_P1,rxAddress_,5)||
       !regWrite(REG_STATUS,0x70)||
       !command(CMD_FLUSH_RX)||!command(CMD_FLUSH_TX)){
        trip();return false;
    }
    uint8_t ch=0,rf=0;
    if(!regRead(REG_RF_CH,ch)||ch!=channel_||
       !regRead(REG_RF_SETUP,rf)||rf!=0x22||
       !regWrite(REG_CONFIG,CONFIG_RX)){
        trip();return false;
    }
    delay(5);
    if(!port_.ce(true)){trip();return false;}
    ready_=true;return true;
}
bool Sc18is606NrfLink::sendFrame(const RadioFrame&frame){
    if(!ready()||txPending_||
       frame.magic[0]!=RADIO_FRAME_MAGIC0||
       frame.magic[1]!=RADIO_FRAME_MAGIC1)return false;
    uint8_t tx[RADIO_FRAME_SIZE+1]={CMD_W_TX},rx[RADIO_FRAME_SIZE+1]={};
    memcpy(tx+1,&frame,sizeof(frame));
    if(!port_.ce(false)||
       !regWrite(REG_CONFIG,CONFIG_TX)||
       !regWrite(REG_STATUS,0x70)||
       !command(CMD_FLUSH_TX)||
       !port_.transfer(tx,rx,sizeof(tx))){
        trip();return false;
    }
    delayMicroseconds(150);
    if(!port_.ce(true)){trip();return false;}
    delayMicroseconds(20); // nRF needs CE pulse >=10us
    if(!port_.ce(false)){trip();return false;}
    txAt_=millis();txPending_=true;lastTxSuccess_=false;
    ++txCount_;
    return true; // buffer admission; authenticated Boat ACK still required
}
bool Sc18is606NrfLink::queueRx(){
    uint8_t tx[RADIO_FRAME_SIZE+1]={CMD_R_RX},rx[RADIO_FRAME_SIZE+1]={};
    for(size_t i=1;i<sizeof(tx);i++)tx[i]=0xff;
    if(!port_.transfer(tx,rx,sizeof(tx)))return false;
    RadioFrame frame{};
    memcpy(&frame,rx+1,RADIO_FRAME_SIZE);
    if(frame.magic[0]==RADIO_FRAME_MAGIC0&&
       frame.magic[1]==RADIO_FRAME_MAGIC1&&
       radio_crc16((const uint8_t*)&frame,offsetof(RadioFrame,crc16))==frame.crc16){
        if(rxCountQueued_<4){
            rxQueue_[rxTail_]=frame;
            rxTail_=(uint8_t)((rxTail_+1U)%4U);
            ++rxCountQueued_;++rxCount_;
        }
    }
    return true;
}
void Sc18is606NrfLink::update(){
    if(!ready())return;
    uint8_t status=0;
    if(!pollStatus(status)){trip();return;}
    if(txPending_){
        if((status&(STAT_TX_DS|STAT_MAX_RT))!=0U){
            const bool ok=(status&STAT_TX_DS)!=0U &&
                          (status&STAT_MAX_RT)==0U;
            if(!regWrite(REG_STATUS,(uint8_t)(STAT_TX_DS|STAT_MAX_RT))||
               (!ok&&!command(CMD_FLUSH_TX))){
                trip();return;
            }
            txPending_=false;lastTxSuccess_=ok;
            if(!ok)++txFailCount_;
            if(!listen())return;
        }else if((uint32_t)(millis()-txAt_)>=TX_STUCK_MS){
            ++txFailCount_;trip();return; // no indefinite stalled STOP queue
        }
    }
    if(txPending_||!ready())return;
    // Poll FIFO instead of using an occupied Waveshare ESP interrupt pin.
    uint8_t fifo=0;
    if(!regRead(REG_FIFO_STATUS,fifo)){trip();return;}
    // At most two RF frames per pass; don't starve buttons/joysticks/LVGL.
    for(unsigned n=0;n<2U && rxCountQueued_<4U && (fifo&0x01U)==0U;n++){
        if(!queueRx()||!regRead(REG_FIFO_STATUS,fifo)){trip();return;}
    }
    if(rxCountQueued_==4U && rxFullPollCount_!=UINT32_MAX) ++rxFullPollCount_;
    if((status&STAT_RX_DR)!=0U && !regWrite(REG_STATUS,STAT_RX_DR))trip();
}
bool Sc18is606NrfLink::receiveFrame(RadioFrame&frame){
    if(!ready()||rxCountQueued_==0)return false;
    frame=rxQueue_[rxHead_];
    rxHead_=(uint8_t)((rxHead_+1U)%4U);
    --rxCountQueued_;return true;
}
