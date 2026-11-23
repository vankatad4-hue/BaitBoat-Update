// Portable protocol and fail-closed regression; no ESP, RF or physical GPIO.
#include "sc18is606_port.h"
#include "sc18is606_nrf_link.h"
#include <assert.h>
#include <array>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vector>

static uint32_t nowMs=0;
uint32_t millis(){return nowMs;}
void delay(unsigned long n){nowMs+=(uint32_t)n;}
void delayMicroseconds(unsigned int){}

uint16_t radio_crc16(const uint8_t*data,size_t length){
    uint16_t crc=0xffffU;
    for(size_t i=0;i<length;i++){
        crc^=(uint16_t)data[i]<<8;
        for(unsigned n=0;n<8;n++)
            crc=(crc&0x8000U)?(uint16_t)((crc<<1)^0x1021U):(uint16_t)(crc<<1);
    }
    return crc;
}

class FakeBridge final:public Sc18is606Io{
public:
    bool wrongId=false,failSpi=false,failAll=false,allowAck=true;
    unsigned spiRequests=0,ceWriteHighs=0;
    uint8_t ceState=0,config=0,status=0x0e;
    std::array<uint8_t,32> registers{};
    std::vector<uint8_t> reply;
    bool loaded=false,rxPending=false;
    std::array<uint8_t,RADIO_FRAME_SIZE> txPayload{},rxPayload{};
    void pauseMs(unsigned duration)override{nowMs+=(uint32_t)duration;}
    bool write(uint8_t addr,const uint8_t*data,size_t count)override{
        if(failAll||addr!=0x28||!data||count==0)return false;
        const uint8_t fn=data[0];
        reply.clear();
        if(fn==0xfe){
            const char *version=wrongId?"WRONG_IS606 1.0":"SC18IS606 1.0.0";
            reply.resize(16U,0);
            memcpy(reply.data(),version,strlen(version)>15U?15U:strlen(version));
        }else if(fn==0xf4&&count==2){
            const uint8_t previous=ceState;
            ceState=(uint8_t)(data[1]&0x04U);
            if(ceState)++ceWriteHighs;
            // nRF autonomous TX completes after a CE pulse; no software IRQ.
            if(previous && !ceState && config==0x0e && loaded && allowAck){
                status|=0x20;loaded=false;
            }
        }else if(fn==0xf5){
            reply.push_back(ceState);
        }else if(fn==0x01){
            ++spiRequests;
            if(failSpi)return false;
            if(count<2)return false;
            const uint8_t cmd=data[1];
            reply.assign(count-1U,0);
            reply[0]=status;
            if((cmd&0xe0U)==0x20U&&count>=3){
                const uint8_t reg=(uint8_t)(cmd&0x1fU);
                if(reg==0x07){
                    status=(uint8_t)(status&~data[2]);
                }else if(reg==0x00){
                    config=data[2];
                    registers[0]=config;
                }else{
                    registers[reg]=data[2];
                }
            }else if((cmd&0xe0U)==0x00U&&count>=3){
                const uint8_t reg=(uint8_t)(cmd&0x1fU);
                reply[1]=(reg==0x17)?(uint8_t)(rxPending?0x00:0x01):registers[reg];
            }else if(cmd==0xa0&&count==RADIO_FRAME_SIZE+2){
                memcpy(txPayload.data(),data+2,RADIO_FRAME_SIZE);
                loaded=true;
            }else if(cmd==0x61&&count==RADIO_FRAME_SIZE+2){
                if(rxPending){memcpy(reply.data()+1,rxPayload.data(),RADIO_FRAME_SIZE);rxPending=false;}
            }else if(cmd==0xe1){
                loaded=false;
            }else if(cmd==0xe2){
                rxPending=false;
            }
        }else if((fn==0xf0||fn==0xf6||fn==0xf7)&&count==2){
            // Bridge configuration commands; guarded by chip identity.
        }else{return false;}
        return true;
    }
    bool read(uint8_t address,uint8_t*dst,size_t count)override{
        if(failAll||address!=0x28||!dst||reply.size()!=count)return false;
        memcpy(dst,reply.data(),count);
        reply.clear();
        return true;
    }
    void inject(const RadioFrame&frame){
        memcpy(rxPayload.data(),&frame,RADIO_FRAME_SIZE);
        rxPending=true;status|=0x40;
    }
};

int main(){
    static_assert(RADIO_FRAME_SIZE==32U,"nRF payload fixed at 32");
    {
        FakeBridge fake;Sc18is606Port bridge(fake,0x28);
        assert(bridge.begin());
        assert(fake.ceState==0);
        uint8_t tx[3]={0xff,0xff,0xff},rx[3]={};
        assert(bridge.transfer(tx,rx,3));
        assert(fake.spiRequests==1);
        assert(bridge.ce(true));
        assert(fake.ceState==4); // SS2/GPIO2, not EEPROM SS1
        assert(bridge.ce(false));
        assert(fake.ceState==0);
        uint8_t tooLong[34]={};
        assert(!bridge.transfer(tooLong,tooLong,sizeof(tooLong)));
        fake.failSpi=true;
        const unsigned before=fake.spiRequests;
        assert(!bridge.transfer(tx,rx,3));
        assert(fake.spiRequests==before+1); // no duplicate nonidempotent SPI write
        assert(!bridge.ready());
    }
    {
        FakeBridge fake;fake.wrongId=true;
        Sc18is606Port bridge(fake,0x28);
        assert(!bridge.begin());
        assert(fake.ceWriteHighs==0); // identify silicon before any CE operations
    }
    {
        FakeBridge fake;Sc18is606Port bridge(fake,0x48);
        assert(!bridge.begin()); // no accidental SC16IS750 address use
    }
    {
        nowMs=0;FakeBridge fake;
        Sc18is606NrfLink radio(fake,0x28,76);
        assert(radio.begin(true));
        assert(radio.ready());
        assert(fake.ceState==4); // CE high in RX mode; SS1 EEPROM untouched
        assert(fake.registers[0x05]==76);
        assert(fake.registers[0x06]==0x22); // 250kbps PA_LOW
        RadioFrame frame{};
        frame.magic[0]=RADIO_FRAME_MAGIC0;
        frame.magic[1]=RADIO_FRAME_MAGIC1;
        frame.type=RADIO_MSG_HEARTBEAT;
        frame.crc16=radio_crc16((const uint8_t*)&frame,offsetof(RadioFrame,crc16));
        assert(radio.sendFrame(frame));
        assert(radio.txPending());
        assert(memcmp(fake.txPayload.data(),&frame,RADIO_FRAME_SIZE)==0);
        radio.update(); // ACK status, radio re-enters RX
        assert(!radio.txPending());
        assert(radio.lastTxSuccess());
        fake.inject(frame);
        radio.update();
        RadioFrame received{};
        assert(radio.receiveFrame(received));
        assert(memcmp(&received,&frame,RADIO_FRAME_SIZE)==0);
        assert(!radio.receiveFrame(received));
        assert(radio.txCount()==1);
        assert(radio.rxCount()==1);
        // Distinct CRC failure must not appear in accepted RX queue.
        frame.crc16=(uint16_t)(frame.crc16^0x1234U);
        fake.inject(frame);
        radio.update();
        assert(!radio.receiveFrame(received));
        fake.failAll=true;
        radio.update();
        assert(!radio.ready()); // revoke primary on bus/bridge errors
        assert(!radio.sendFrame(frame));
    }
    {
        nowMs=0;FakeBridge fake;fake.allowAck=false;
        Sc18is606NrfLink radio(fake,0x28,76);
        assert(radio.begin(true));
        RadioFrame frame{};
        frame.magic[0]=RADIO_FRAME_MAGIC0;frame.magic[1]=RADIO_FRAME_MAGIC1;
        assert(radio.sendFrame(frame));
        fake.status|=0x10; // MAX_RT: radio has exhausted its auto-ACK retries
        radio.update();
        assert(!radio.txPending());
        assert(!radio.lastTxSuccess());
        assert(radio.txFailCount()==1);
        assert(radio.ready()); // recover RX, never misreport TX ACK
    }
    {
        nowMs=0;FakeBridge fake;fake.allowAck=false;
        Sc18is606NrfLink radio(fake,0x28,76);
        assert(radio.begin(true));
        RadioFrame frame{};
        frame.magic[0]=RADIO_FRAME_MAGIC0;frame.magic[1]=RADIO_FRAME_MAGIC1;
        assert(radio.sendFrame(frame));
        nowMs+=91;
        radio.update();
        assert(!radio.ready()); // stale TX is never treated as delivered
        assert(radio.txFailCount()==1);
    }
    return 0;
}
