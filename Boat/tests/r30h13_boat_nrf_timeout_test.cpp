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
    return 0;
}
