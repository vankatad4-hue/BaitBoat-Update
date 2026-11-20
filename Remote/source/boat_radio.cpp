#include "boat_radio.h"
#include "radio_protocol.h"
#include "radio_key_store.h"
#include "sc16is750_bridge.h"
#include "e22_uart_link.h"
#include <Wire.h>

static DualRadioManager g_linkManager;
static SC16IS750Bridge g_uartBridge(Wire,0x48);
static E22UartLink g_lora(g_uartBridge,
    [](bool m0,bool m1,void*){ g_uartBridge.setE22Mode(m0,m1); },
    [](void*){ return g_uartBridge.e22AuxReady(); },
    [](bool released,void*){ g_uartBridge.setE22Reset(released); },
    nullptr);
static bool g_loraConfigured=false;
static bool g_begun=false;
static constexpr uint8_t REMOTE_DEVICE_ID=0x01;
static constexpr uint8_t BOAT_DEVICE_ID=0x02;
static constexpr uint8_t SESSION_EPOCH=1;
static uint8_t g_authKey[32];

static BoatRadioLink toPublicLink(ManagedLink link) {
    if(link==LINK_NRF_PRIMARY) return RADIO_LINK_NRF24_PRIMARY;
    if(link==LINK_LORA_BACKUP) return RADIO_LINK_LORA_BACKUP;
    return RADIO_LINK_NONE;
}
static BoatRadioState toPublicState(ManagedState state) {
    switch(state) {
        case LINK_PRIMARY:return RADIO_STATE_PRIMARY;
        case LINK_DEGRADED:return RADIO_STATE_DEGRADED;
        case LINK_BACKUP:return RADIO_STATE_BACKUP;
        case LINK_NO_LINK:return RADIO_STATE_NO_LINK;
        case LINK_RECOVERY:return RADIO_STATE_RECOVERY;
        default:return RADIO_STATE_DISCONNECTED;
    }
}

static bool radioSendTransport(ManagedLink link,const BoatCommand &command) {
    if(link!=LINK_LORA_BACKUP || !g_loraConfigured) return false;
    if(!radioKey_get(g_authKey)) return false; // fail closed; no key means no command TX
    RadioFrame frame;
    if(!radio_encodeCommand(command,REMOTE_DEVICE_ID,BOAT_DEVICE_ID,SESSION_EPOCH,frame,g_authKey))
        return false;
    return g_lora.sendFrame(frame,millis());
}

bool boatRadio_begin() {
    g_linkManager.begin(millis());
    g_loraConfigured=false;
    g_begun=false;
    memset(g_authKey,0,sizeof(g_authKey));

    // Board initialization must already have brought up the shared I2C bus.
    // SC16IS750 is on I2C 0x48 and provides the E22 9600-8N1 UART.
    if(!g_uartBridge.begin(9600)) return false;
    if(!g_lora.configureEU868TestProfile()) return false;

    g_loraConfigured=true;
    g_linkManager.setTransportReady(LINK_LORA_BACKUP,true);
    g_linkManager.setTransportReady(LINK_NRF_PRIMARY,false); // primary RF24 driver not yet installed
    g_begun=true;
    return true; // LoRa hardware is configured; no authenticated link is established yet
}

bool boatRadio_sendCommand(const BoatCommand &command) {
    if(!g_begun) return false;
    return g_linkManager.send(command,radioSendTransport,millis());
}

void boatRadio_update() {
    if(!g_begun) return;
    g_lora.update();
    RadioFrame frame;
    while(g_lora.receiveFrame(frame)) {
        const uint32_t now=millis();
        if(!radioKey_get(g_authKey)) {
            g_linkManager.reportFailure(LINK_LORA_BACKUP);
            continue;
        }
        const bool valid=radio_frameVerify(frame,g_authKey) &&
            frame.sourceId==BOAT_DEVICE_ID && frame.destinationId==REMOTE_DEVICE_ID &&
            radio_tickFresh(frame.tick16,(uint16_t)now,3000) &&
            (frame.type==RADIO_MSG_HEARTBEAT || frame.type==RADIO_MSG_TELEMETRY ||
             frame.type==RADIO_MSG_ACK || frame.type==RADIO_MSG_LINK_PROBE);
        if(valid) g_linkManager.reportValidRx(LINK_LORA_BACKUP,true,now);
        else g_linkManager.reportFailure(LINK_LORA_BACKUP);
    }
    g_linkManager.tick(millis());
}

bool boatRadio_isConnected(){return g_linkManager.connected(millis());}
BoatRadioLink boatRadio_getActiveLink(){return toPublicLink(g_linkManager.active());}
BoatRadioState boatRadio_getState(){return toPublicState(g_linkManager.state());}
bool boatRadio_primaryConnected(){return g_linkManager.primaryHealthy(millis());}
bool boatRadio_backupConnected(){return g_linkManager.backupHealthy(millis());}
void boatRadio_setTransport(RadioSendFn transport){(void)transport;} // driver selection fixed in R1 scaffold
void boatRadio_setReady(ManagedLink link,bool ready){g_linkManager.setTransportReady(link,ready);}
void boatRadio_reportValidPacket(ManagedLink link,bool authenticated,uint32_t receivedAtMs){g_linkManager.reportValidRx(link,authenticated,receivedAtMs);}
void boatRadio_reportFailure(ManagedLink link){g_linkManager.reportFailure(link);}
