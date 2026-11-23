// Portable protocol and fail-closed regression; no ESP, RF or physical GPIO.
#include "sc18is606_port.h"
#include "sc18is606_nrf_link.h"
#include "mcp23017_ce_port.h"
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
    unsigned spiRequests=0,unexpectedCeCommands=0;
    uint8_t config=0,status=0x0e;
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
        }else if(fn==0xf4||fn==0xf5||fn==0xf6||fn==0xf7){
            ++unexpectedCeCommands;return false; // SS2 may NEVER drive nRF CE
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
        }else if(fn==0xf0&&count==2){
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


// MCP23017@0x21 GPA0 drives CE while GPA4 remains an OK-button INPUT.
// This fake models reset IODIRA=0xff, OLATA=0x00 and the output pin readback.
class FakeMcpCe final:public Mcp23017CeBus {
public:
    explicit FakeMcpCe(FakeBridge&bridge):bridge_(bridge){}
    bool fail=false;
    uint8_t dir=0xff,latch=0,pull=0x10; // GPA4 OK pull-up preserved
    unsigned highTransitions=0,writeCount=0;
    bool writeReg(uint8_t addr,uint8_t reg,uint8_t value)override{
        if(fail||addr!=0x21)return false;
        ++writeCount;
        if(reg==0x14){
            const bool wasHigh=(latch&1U)!=0U && (dir&1U)==0U;
            latch=value;
            const bool nowHigh=(latch&1U)!=0U && (dir&1U)==0U;
            if(nowHigh&&!wasHigh)++highTransitions;
            if(wasHigh&&!nowHigh && bridge_.config==0x0e && bridge_.loaded && bridge_.allowAck){
                bridge_.status|=0x20;bridge_.loaded=false;
            }
            return true;
        }
        if(reg==0x00){dir=value;return true;}
        if(reg==0x0c){pull=value;return true;}
        return false;
    }
    bool readReg(uint8_t addr,uint8_t reg,uint8_t&value)override{
        if(fail||addr!=0x21)return false;
        switch(reg){
          case 0x0a:value=0;return true; // IOCON BANK0
          case 0x14:value=latch;return true;
          case 0x00:value=dir;return true;
          case 0x0c:value=pull;return true;
          case 0x12:value=(uint8_t)((latch&~dir)|0x10U);return true;
          default:return false;
        }
    }
private:
    FakeBridge&bridge_;
};

int main(){
    static_assert(RADIO_FRAME_SIZE==32U,"nRF payload fixed at 32");
    {
        FakeBridge fake;FakeMcpCe io(fake);Mcp23017CePort ce(io);
        Sc18is606Port bridge(fake,ce,0x28);
        assert(bridge.begin());
        assert((io.latch&1U)==0U && (io.dir&1U)==0U);
        assert((io.pull&0x10U)!=0U); // retain OK button pull-up
        uint8_t tx[3]={0xff,0xff,0xff},rx[3]={};
        assert(bridge.transfer(tx,rx,3));
        assert(fake.spiRequests==1);
        assert(bridge.ce(true));
        assert((io.latch&1U)==1U); // GPA0, never SS2
        assert(bridge.ce(false));
        assert((io.latch&1U)==0U);
        assert(fake.unexpectedCeCommands==0);
        uint8_t tooLong[34]={};
        assert(!bridge.transfer(tooLong,tooLong,sizeof(tooLong)));
        fake.failSpi=true;
        const unsigned before=fake.spiRequests;
        assert(!bridge.transfer(tx,rx,3));
        assert(fake.spiRequests==before+1); // no duplicate nonidempotent SPI write
        assert(!bridge.ready());
    }
    {
        FakeBridge fake;fake.wrongId=true;FakeMcpCe io(fake);
        Mcp23017CePort ce(io);Sc18is606Port bridge(fake,ce,0x28);
        assert(!bridge.begin());
        assert(io.highTransitions==0); // CE stays low on wrong bridge ID
    }
    {
        FakeBridge fake;FakeMcpCe io(fake);
        Mcp23017CePort ce(io);Sc18is606Port bridge(fake,ce,0x48);
        assert(!bridge.begin()); // no accidental SC16IS750 address use
    }
    {
        nowMs=0;FakeBridge fake;FakeMcpCe io(fake);
        Mcp23017CePort ce(io);
        Sc18is606NrfLink radio(fake,ce,0x28,76);
        assert(radio.begin(true));
        assert(radio.ready());
        assert((io.latch&1U)==1U); // CE high in RX mode on MCP GPA0
        assert(fake.unexpectedCeCommands==0); // SS2 + EEPROM untouched
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
        FakeMcpCe io(fake);Mcp23017CePort ce(io);
        Sc18is606NrfLink radio(fake,ce,0x28,76);
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
        FakeMcpCe io(fake);Mcp23017CePort ce(io);
        Sc18is606NrfLink radio(fake,ce,0x28,76);
        assert(radio.begin(true));
        RadioFrame frame{};
        frame.magic[0]=RADIO_FRAME_MAGIC0;frame.magic[1]=RADIO_FRAME_MAGIC1;
        assert(radio.sendFrame(frame));
        nowMs+=91;
        radio.update();
        assert(!radio.ready()); // stale TX is never treated as delivered
        assert(radio.txFailCount()==1);
    }

    { // Reset default GPIOs are high-impedance. A failed I2C line must never
      // be treated as an accepted radio command; hardware pull-down needed.
      FakeBridge fake;FakeMcpCe io(fake);Mcp23017CePort ce(io);
      io.fail=true;
      assert(!ce.beginLow());
      assert(!ce.ready());
      assert(!ce.write(true));
      io.fail=false;
      assert(ce.beginLow());
      io.fail=true;
      assert(!ce.write(true));
      assert(!ce.ready());
    }
    return 0;
}
