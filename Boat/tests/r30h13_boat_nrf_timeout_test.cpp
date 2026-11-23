// Portable RF24 host mock: tests the stuck-TX failure boundary, NO RF output.
#include "nrf24_link.h"
#include <assert.h>
FakeRadioBehavior fakeRadio{};
static uint32_t fakeNow=0;
uint32_t millis(){return fakeNow;}
uint16_t radio_crc16(const uint8_t*,size_t){return 0;}
static RadioFrame validFrame(){
    RadioFrame f{};f.magic[0]=RADIO_FRAME_MAGIC0;
    f.magic[1]=RADIO_FRAME_MAGIC1;return f;
}
int main(){
    SPIClass spi;
    {
        NRF24Link radio(-1,-1,&spi);
        assert(!radio.begin(false));
        assert(!radio.ready());
    }
    { // No IRQ/status for >120ms: revoke radio, no stale output or queue.
        fakeRadio={};fakeNow=100;
        NRF24Link radio(10,9,&spi,76,12,13,11);
        assert(radio.begin(false));
        const RadioFrame f=validFrame();
        assert(radio.sendFrame(f));
        assert(radio.txPending());
        assert(!radio.sendFrame(f));
        fakeNow=219;radio.update();
        assert(radio.ready());
        assert(radio.txPending());
        fakeNow=220;radio.update();
        assert(!radio.ready());
        assert(!radio.txPending());
        assert(!radio.lastTxSuccess());
        assert(radio.txFailCount()==1);
        assert(fakeRadio.flushes==1);
        assert(!radio.sendFrame(f));
        RadioFrame out{};
        assert(!radio.receiveFrame(out));
    }
    { // Successful RF24 TX status clears pending without false watchdog.
        fakeRadio={};fakeNow=300;
        NRF24Link radio(10,9,&spi);
        assert(radio.begin(false));
        assert(radio.sendFrame(validFrame()));
        fakeNow=320;fakeRadio.ack=true;radio.update();
        assert(radio.ready());
        assert(!radio.txPending());
        assert(radio.lastTxSuccess());
        assert(radio.txFailCount()==0);
    }
    { // MAX_RT is an explicit RF failure, not a forced full transport lock.
        fakeRadio={};fakeNow=450;
        NRF24Link radio(10,9,&spi);
        assert(radio.begin(false));
        assert(radio.sendFrame(validFrame()));
        fakeRadio.maxRt=true;radio.update();
        assert(radio.ready());
        assert(!radio.txPending());
        assert(!radio.lastTxSuccess());
        assert(radio.txFailCount()==1);
        assert(fakeRadio.flushes==1);
    }
    { // millis() wraps; unsigned arithmetic still expires after 120ms.
        fakeRadio={};fakeNow=0xfffffffaU;
        NRF24Link radio(10,9,&spi);
        assert(radio.begin(false));
        assert(radio.sendFrame(validFrame()));
        fakeNow=(uint32_t)(0xfffffffaU+121U);
        radio.update();
        assert(!radio.ready());
        assert(radio.txFailCount()==1);
    }
    { // R30H-14: both ends must agree on 250 kbps, CRC16, 32 bytes,
      // channel, retry settings and directional pipe addresses.
        fakeRadio={};fakeNow=500;
        NRF24Link boat(10,9,&spi,76,12,13,11);
        assert(boat.begin(false));
        const uint8_t expectWrite[5]={'R','M','T','E','1'};
        const uint8_t expectRead[5]={'B','O','A','T','1'};
        assert(fakeRadio.channel==76);
        assert(fakeRadio.dataRate==RF24_250KBPS);
        assert(fakeRadio.paLevel==RF24_PA_LOW);
        assert(fakeRadio.autoAck);
        assert(fakeRadio.retryDelay==5&&fakeRadio.retryCount==15);
        assert(fakeRadio.crcLength==RF24_CRC_16);
        assert(fakeRadio.dynamicPayloadsDisabled);
        assert(fakeRadio.payloadSize==RADIO_FRAME_SIZE);
        assert(fakeRadio.readPipeIndex==1);
        assert(memcmp(fakeRadio.writePipe,expectWrite,5U)==0);
        assert(memcmp(fakeRadio.readPipe,expectRead,5U)==0);
    }
    { // R30H-14: one scheduled update drains the physical 3-deep RX FIFO.
      // The application dequeues without performing another physical SPI read.
        fakeRadio={};fakeNow=600;
        NRF24Link boat(10,9,&spi);
        assert(boat.begin(false));
        RadioFrame first=validFrame(),invalid=validFrame(),last=validFrame();
        first.commandId=11U;invalid.commandId=12U;last.commandId=13U;
        invalid.crc16=1U; // mock crc16() returns zero; reject this payload
        assert(fakeRadio.inject(&first,sizeof(first)));
        assert(fakeRadio.inject(&invalid,sizeof(invalid)));
        assert(fakeRadio.inject(&last,sizeof(last)));
        boat.update();
        assert(fakeRadio.readCalls==3U);
        assert(fakeRadio.rxCount==0U);
        const unsigned reads=fakeRadio.readCalls;
        const unsigned available=fakeRadio.availableCalls;
        RadioFrame out{};
        assert(boat.receiveFrame(out)&&out.commandId==11U);
        assert(boat.receiveFrame(out)&&out.commandId==13U);
        assert(!boat.receiveFrame(out));
        assert(boat.rxCount()==2U);
        assert(fakeRadio.readCalls==reads&&fakeRadio.availableCalls==available);
        assert(!boat.txPending());
    }
    { // During pending TX, defer RX FIFO handling until completion.
        fakeRadio={};fakeNow=700;
        NRF24Link boat(10,9,&spi);
        assert(boat.begin(false));
        assert(boat.sendFrame(validFrame()));
        RadioFrame incoming=validFrame();incoming.commandId=88U;
        assert(fakeRadio.inject(&incoming,sizeof(incoming)));
        boat.update();
        assert(fakeRadio.readCalls==0U);
        fakeRadio.ack=true;boat.update();
        assert(fakeRadio.readCalls==1U);
        assert(boat.lastTxSuccess());
        RadioFrame out{};
        assert(boat.receiveFrame(out)&&out.commandId==88U);
    }
    return 0;
}
