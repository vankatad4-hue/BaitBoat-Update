#include "boat_radio.h"
#include "radio_protocol.h"
#include "radio_key_store.h"
#include "sc16is750_bridge.h"
#include "e22_uart_link.h"
#include "nrf24_link.h"
#include "radio_hw_config.h"
#include <Wire.h>
#include <SPI.h>

static DualRadioManager g_linkManager;
static SC16IS750Bridge g_uartBridge(Wire,0x48);
static E22UartLink g_lora(g_uartBridge,
    [](bool m0,bool m1,void*){ g_uartBridge.setE22Mode(m0,m1); },
    [](void*){ return g_uartBridge.e22AuxReady(); },
    [](bool released,void*){ g_uartBridge.setE22Reset(released); },
    nullptr);
static NRF24Link g_nrf(REMOTE_NRF_CE_PIN,REMOTE_NRF_CSN_PIN,&SPI,REMOTE_NRF_CHANNEL);
static bool g_loraConfigured=false;
static bool g_nrfReady=false;
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
    if(!radioKey_get(g_authKey)) return false; // fail closed; no key means no command TX
    RadioFrame frame;
    if(!radio_encodeCommand(command,REMOTE_DEVICE_ID,BOAT_DEVICE_ID,SESSION_EPOCH,frame,g_authKey))
        return false;
    if(link==LINK_NRF_PRIMARY && g_nrfReady) return g_nrf.sendFrame(frame);
    if(link==LINK_LORA_BACKUP && g_loraConfigured) return g_lora.sendFrame(frame,millis());
    return false;
}

bool boatRadio_begin() {
    g_linkManager.begin(millis());
    g_loraConfigured=false;g_nrfReady=false;g_begun=false;
    memset(g_authKey,0,sizeof(g_authKey));

    // Display/SD initialization should already have initialized the shared SPI
    // and I2C buses. The exact physical radio CE/CSN pins are still unassigned.
    g_nrfReady=g_nrf.begin(true);
    g_linkManager.setTransportReady(LINK_NRF_PRIMARY,g_nrfReady);

    // SC16IS750 I2C bridge at 0x48 provides the E22 UART without using GPIO15/16.
    if(g_uartBridge.begin(9600) && g_lora.configureEU868TestProfile()) {
        g_loraConfigured=true;
        g_linkManager.setTransportReady(LINK_LORA_BACKUP,true);
    } else {
        g_linkManager.setTransportReady(LINK_LORA_BACKUP,false);
    }
    g_begun=g_nrfReady || g_loraConfigured;
    return g_begun; // configured hardware, not a connected/authenticated boat link
}

bool boatRadio_sendCommand(const BoatCommand &command) {
    if(!g_begun) return false;
    return g_linkManager.send(command,radioSendTransport,millis());
}

static void validateInboundFrame(ManagedLink link,const RadioFrame &frame) {
    const uint32_t now=millis();
    if(!radioKey_get(g_authKey)) {g_linkManager.reportFailure(link);return;}
    const bool valid=radio_frameVerify(frame,g_authKey) &&
        frame.sourceId==BOAT_DEVICE_ID && frame.destinationId==REMOTE_DEVICE_ID &&
        radio_tickFresh(frame.tick16,(uint16_t)now,3000) &&
        (frame.type==RADIO_MSG_HEARTBEAT || frame.type==RADIO_MSG_TELEMETRY ||
         frame.type==RADIO_MSG_ACK || frame.type==RADIO_MSG_LINK_PROBE);
    if(valid) g_linkManager.reportValidRx(link,true,now);
    else g_linkManager.reportFailure(link);
}

void boatRadio_update() {
    if(!g_begun) return;
    if(g_nrfReady) {
        g_nrf.update();
        RadioFrame frame;
        while(g_nrf.receiveFrame(frame)) validateInboundFrame(LINK_NRF_PRIMARY,frame);
    }
    if(g_loraConfigured) {
        g_lora.update();
        RadioFrame frame;
        while(g_lora.receiveFrame(frame)) validateInboundFrame(LINK_LORA_BACKUP,frame);
    }
    g_linkManager.tick(millis());
}

bool boatRadio_isConnected(){return g_linkManager.connected(millis());}
BoatRadioLink boatRadio_getActiveLink(){return toPublicLink(g_linkManager.active());}
BoatRadioState boatRadio_getState(){return toPublicState(g_linkManager.state());}
bool boatRadio_primaryConnected(){return g_linkManager.primaryHealthy(millis());}
bool boatRadio_backupConnected(){return g_linkManager.backupHealthy(millis());}
void boatRadio_setTransport(RadioSendFn transport){(void)transport;} // fixed in this adapter
void boatRadio_setReady(ManagedLink link,bool ready){g_linkManager.setTransportReady(link,ready);}
void boatRadio_reportValidPacket(ManagedLink link,bool authenticated,uint32_t receivedAtMs){g_linkManager.reportValidRx(link,authenticated,receivedAtMs);}
void boatRadio_reportFailure(ManagedLink link){g_linkManager.reportFailure(link);}
