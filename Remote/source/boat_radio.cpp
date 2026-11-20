#include "boat_radio.h"
#include "radio_payload_codec.h"
#include "radio_ack_tracker.h"
#include "boat_telemetry.h"
#include "boat_state.h"
#include "radio_protocol.h"
#include "radio_command_codec.h"
#include "radio_key_store.h"
#include "sc16is750_bridge.h"
#include "e22_uart_link.h"
#include "nrf24_link.h"
#include "radio_hw_config.h"
#include <Wire.h>
#include <SPI.h>
#include <esp_system.h>
#include <string.h>

static DualRadioManager g_linkManager;
static RadioAckTracker g_ackTracker;
static SC16IS750Bridge g_uartBridge(Wire,0x48);
static E22UartLink g_lora(g_uartBridge,
    [](bool m0,bool m1,void*){ g_uartBridge.setE22Mode(m0,m1); },
    [](void*){ return g_uartBridge.e22AuxReady(); },
    [](bool released,void*){ g_uartBridge.setE22Reset(released); },
    nullptr);
static NRF24Link g_nrf(REMOTE_NRF_CE_PIN,REMOTE_NRF_CSN_PIN,&SPI,REMOTE_NRF_CHANNEL,REMOTE_NRF_SCK_PIN,REMOTE_NRF_MISO_PIN,REMOTE_NRF_MOSI_PIN);
static bool g_loraConfigured=false;
static bool g_nrfReady=false;
static bool g_begun=false;
static constexpr uint8_t REMOTE_DEVICE_ID=0x01;
static constexpr uint8_t BOAT_DEVICE_ID=0x02;
static uint8_t g_sessionEpoch=1;
static uint16_t g_txSequence=0;
static uint32_t g_lastPrimaryHeartbeat=0;
static uint32_t g_lastBackupTxMs=0,g_lastBackupTxAttemptMs=0;
static bool g_backupTxSent=false,g_loraPending=false;
static uint8_t g_loraPendingPriority=0;
static RadioFrame g_loraPendingFrame;
static uint8_t g_authKey[32];
static RadioSequenceGuard g_boatSequenceGuard; // global cross-radio replay/freshness guard
static RadioSequenceGuard g_boatPrimarySequenceGuard;
static RadioSequenceGuard g_boatBackupSequenceGuard;
static bool g_sessionStartSeenPrimary=false,g_sessionStartSeenBackup=false;
static bool g_boatSessionStarted=false;
static uint8_t g_boatSessionEpoch=0;
static uint32_t g_lastSessionStartPrimaryMs=0,g_lastSessionStartBackupMs=0;
static constexpr uint32_t SESSION_START_PRIMARY_RETRY_MS=500;
static constexpr uint32_t SESSION_START_BACKUP_RETRY_MS=3000;

static bool sendSessionStartMarker(ManagedLink link,uint32_t now)
{
    if(!radioKey_get(g_authKey)) return false;
    RadioFrame start;
    radio_frameInit(start,RADIO_MSG_SESSION_START,REMOTE_DEVICE_ID,BOAT_DEVICE_ID,0,0,now,g_sessionEpoch);
    start.payload[0]=g_sessionEpoch;
    if(!radio_frameSeal(start,g_authKey)) return false;
    if(link==LINK_NRF_PRIMARY && g_nrfReady) return g_nrf.sendFrame(start);
    if(link==LINK_LORA_BACKUP && g_loraConfigured) return g_lora.sendFrame(start,now);
    return false;
}

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
    BoatCommand wireCommand=command;
    wireCommand.sequence=++g_txSequence;
    RadioFrame frame;
    if(!radio_encodeCommand(wireCommand,REMOTE_DEVICE_ID,BOAT_DEVICE_ID,g_sessionEpoch,frame,g_authKey))
        return false;
    if(link==LINK_NRF_PRIMARY && g_nrfReady) return g_nrf.sendFrame(frame);
    if(link==LINK_LORA_BACKUP && g_loraConfigured) {
        const uint32_t now=millis();
        const uint8_t priority=(command.type==CMD_EMERGENCY_STOP)?3U:
            ((command.type==CMD_SET_MODE && command.mode==MODE_RETURN_HOME)?2U:
             (command.type==CMD_MANUAL_MOTORS?1U:0U));
        if(priority==0U)return false;
        // Keep only the highest-priority pending backup action; latest manual
        // control replaces older manual frames. STOP cannot be overwritten.
        if(g_loraPending) {
            if(priority>=g_loraPendingPriority){g_loraPendingFrame=frame;g_loraPendingPriority=priority;}
            g_loraPending=true;
            if(g_backupTxSent && (uint32_t)(now-g_lastBackupTxMs)<3000U) return true;
            if(g_lora.sendFrame(g_loraPendingFrame,now)){g_lastBackupTxMs=now;g_backupTxSent=true;g_loraPending=false;g_loraPendingPriority=0;}
            return true;
        }
        if(g_backupTxSent && (uint32_t)(now-g_lastBackupTxMs)<3000U) {
            g_loraPendingFrame=frame;g_loraPending=true;g_loraPendingPriority=priority;return true;
        }
        if(g_lora.sendFrame(frame,now)){g_lastBackupTxMs=now;g_backupTxSent=true;return true;}
        g_loraPendingFrame=frame;g_loraPending=true;g_loraPendingPriority=priority;return true;
    }
    return false;
}

bool boatRadio_begin() {
    g_linkManager.begin(millis());
    g_loraConfigured=false;g_nrfReady=false;g_begun=false;
    memset(g_authKey,0,sizeof(g_authKey));
    g_sessionEpoch=(uint8_t)esp_random(); if(g_sessionEpoch==0)g_sessionEpoch=1;
    g_txSequence=0;g_lastPrimaryHeartbeat=0;g_lastBackupTxMs=0;g_lastBackupTxAttemptMs=0;g_backupTxSent=false;g_loraPending=false;g_loraPendingPriority=0;g_boatSequenceGuard.reset();g_boatPrimarySequenceGuard.reset();g_boatBackupSequenceGuard.reset();g_boatSessionStarted=false;g_boatSessionEpoch=0;g_sessionStartSeenPrimary=false;g_sessionStartSeenBackup=false;g_ackTracker.reset(0);telemetryInit();

    // Display/SD setup initializes shared SPI/I2C. nRF CE/CSN pins are provisional.
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
    // Authenticated boot/session marker; TX remains disabled until key is provisioned.
    if(g_begun && radioKey_get(g_authKey)) {
        const uint32_t startNow=millis();
        if(g_nrfReady && sendSessionStartMarker(LINK_NRF_PRIMARY,startNow)) g_lastSessionStartPrimaryMs=startNow;
        if(g_loraConfigured && sendSessionStartMarker(LINK_LORA_BACKUP,startNow)) {
            g_lastSessionStartBackupMs=startNow;g_lastBackupTxMs=startNow;g_backupTxSent=true;
        }
    }
    return g_begun; // configured hardware, not a connected/authenticated boat link
}

bool boatRadio_sendCommand(const BoatCommand &command) {
    if(!g_begun) return false;
    const uint32_t now=millis();
    const bool queued=g_linkManager.send(command,radioSendTransport,now);
    if(queued && command.type!=CMD_MANUAL_MOTORS) {
        const uint16_t id=(uint16_t)(command.commandId&0xFFFFU);
        const uint8_t mode=(uint8_t)command.mode;
        const uint16_t timeout=(command.type>=CMD_RELEASE_HOPPER_LEFT && command.type<=CMD_RELEASE_HOOK_RIGHT)?3500U:2500U;
        if(id!=0) g_ackTracker.begin(id,(uint8_t)command.type,mode,now,timeout);
    }
    return queued;
}

static bool dispatchValidatedPayload(ManagedLink link,const RadioFrame &frame,uint32_t now) {
    RadioDecodedPayload decoded{};
    switch(frame.type) {
        case RADIO_MSG_HEARTBEAT:
            return radio_decodeHeartbeatPayload(frame.payload,decoded);
        case RADIO_MSG_ACK:
            if(!radio_decodeAckPayload(frame.payload,decoded)) return false;
            // Authenticated, session-bound ACKs may update only a matching command record.
            g_ackTracker.observe(decoded,link==LINK_NRF_PRIMARY?RADIO_ACK_LINK_PRIMARY:RADIO_ACK_LINK_BACKUP,
                                 frame.sessionEpoch,now);
            return true;
        case RADIO_MSG_TELEMETRY:
            if(!radio_decodeTelemetryPayload(frame.payload,decoded)) return false;
            return telemetryApplyRadioPayload(decoded,link==LINK_NRF_PRIMARY,now);
        case RADIO_MSG_LINK_PROBE:
            return true;
        default:
            return false;
    }
}
static bool payloadBytesZero(const uint8_t *p,uint8_t start,uint8_t end) {
    for(uint8_t i=start;i<=end;i++)if(p[i]!=0)return false;
    return true;
}
static void validateInboundFrame(ManagedLink link,const RadioFrame &frame) {
    const uint32_t now=millis();
    if(!radioKey_get(g_authKey)){g_linkManager.reportFailure(link);return;}
    const bool sessionPayloadValid=frame.type!=RADIO_MSG_SESSION_START||
        (frame.sequence==0&&frame.payload[0]==frame.sessionEpoch&&frame.sessionEpoch==g_sessionEpoch&&payloadBytesZero(frame.payload,1,7));
    const bool probePayloadValid=frame.type!=RADIO_MSG_LINK_PROBE||payloadBytesZero(frame.payload,0,7);
    const bool envelopeValid=radio_frameVerify(frame,g_authKey)&&frame.flags==0&&
        frame.sourceId==BOAT_DEVICE_ID&&frame.destinationId==REMOTE_DEVICE_ID&&
        (frame.type==RADIO_MSG_SESSION_START||frame.type==RADIO_MSG_HEARTBEAT||
         frame.type==RADIO_MSG_TELEMETRY||frame.type==RADIO_MSG_ACK||frame.type==RADIO_MSG_LINK_PROBE)&&
        sessionPayloadValid&&probePayloadValid;
    if(!envelopeValid){g_linkManager.reportFailure(link);return;}
    RadioSequenceGuard &linkGuard=(link==LINK_NRF_PRIMARY)?g_boatPrimarySequenceGuard:g_boatBackupSequenceGuard;
    bool &startSeen=(link==LINK_NRF_PRIMARY)?g_sessionStartSeenPrimary:g_sessionStartSeenBackup;
    if(frame.type==RADIO_MSG_SESSION_START) {
        if(g_boatSessionStarted&&frame.sessionEpoch==g_boatSessionEpoch) {
            // Same-link replay is not allowed to reset sequence state. A first
            // session marker on the second radio seeds only that link's guard.
            if(!startSeen){linkGuard.accept(frame.sessionEpoch,frame.sequence,true);startSeen=true;}
            g_linkManager.reportValidRx(link,true,now);return;
        }
        g_boatSequenceGuard.reset();g_boatPrimarySequenceGuard.reset();g_boatBackupSequenceGuard.reset();
        g_sessionStartSeenPrimary=false;g_sessionStartSeenBackup=false;
        if(!g_boatSequenceGuard.accept(frame.sessionEpoch,frame.sequence,true)||
           !linkGuard.accept(frame.sessionEpoch,frame.sequence,true)){g_linkManager.reportFailure(link);return;}
        g_boatSessionStarted=true;g_boatSessionEpoch=frame.sessionEpoch;startSeen=true;
        g_ackTracker.reset(frame.sessionEpoch);telemetryInit();
        g_linkManager.reportValidRx(link,true,now);return;
    }
    if(!g_boatSessionStarted||frame.sessionEpoch!=g_boatSessionEpoch||
       !linkGuard.accept(frame.sessionEpoch,frame.sequence,false)){
        g_linkManager.reportFailure(link);return;
    }
    // Cross-radio duplicate/stale frames are authenticated but must not refresh
    // application telemetry or ACK state a second time.
    if(!g_boatSequenceGuard.accept(frame.sessionEpoch,frame.sequence,false)){
        g_linkManager.reportValidRx(link,true,now);return;
    }
    if(!dispatchValidatedPayload(link,frame,now)){g_linkManager.reportFailure(link);return;}
    g_linkManager.reportValidRx(link,true,now);
}

void boatRadio_update() {
    if(!g_begun) return;
    const uint32_t now=millis();
    if(radioKey_get(g_authKey) && g_begun) {
        if(g_nrfReady && !g_sessionStartSeenPrimary &&
           (uint32_t)(now-g_lastSessionStartPrimaryMs)>=SESSION_START_PRIMARY_RETRY_MS) {
            if(sendSessionStartMarker(LINK_NRF_PRIMARY,now)) g_lastSessionStartPrimaryMs=now;
        }
        if(g_loraConfigured && !g_sessionStartSeenBackup &&
           (uint32_t)(now-g_lastSessionStartBackupMs)>=SESSION_START_BACKUP_RETRY_MS &&
           (uint32_t)(now-g_lastBackupTxAttemptMs)>=250U) {
            g_lastBackupTxAttemptMs=now;
            if(sendSessionStartMarker(LINK_LORA_BACKUP,now)) {
                g_lastSessionStartBackupMs=now;g_lastBackupTxMs=now;g_backupTxSent=true;
            }
        }
    }
    if(radioKey_get(g_authKey)) {
        if(g_nrfReady && (uint32_t)(now-g_lastPrimaryHeartbeat)>=100U) {
            g_lastPrimaryHeartbeat=now;RadioFrame hb;radio_frameInit(hb,RADIO_MSG_HEARTBEAT,REMOTE_DEVICE_ID,BOAT_DEVICE_ID,++g_txSequence,0,now,g_sessionEpoch);
            hb.payload[0]=(uint8_t)(1U|(g_nrfReady?2U:0U)|(g_loraConfigured?4U:0U)|(boatState.emergencyStopConfirmed&&boatState.emergencyStop?8U:0U));hb.payload[1]=1;radio_frameSeal(hb,g_authKey);g_nrf.sendFrame(hb);
        }
        if(g_loraConfigured && g_loraPending && (!g_backupTxSent || (uint32_t)(now-g_lastBackupTxMs)>=3000U) && (uint32_t)(now-g_lastBackupTxAttemptMs)>=250U) {
            g_lastBackupTxAttemptMs=now;
            if(g_lora.sendFrame(g_loraPendingFrame,now)){g_lastBackupTxMs=now;g_backupTxSent=true;g_loraPending=false;g_loraPendingPriority=0;}
        }
        if(g_loraConfigured && !g_loraPending && (!g_backupTxSent || (uint32_t)(now-g_lastBackupTxMs)>=3000U) && (uint32_t)(now-g_lastBackupTxAttemptMs)>=250U) {
            g_lastBackupTxAttemptMs=now;RadioFrame hb;radio_frameInit(hb,RADIO_MSG_HEARTBEAT,REMOTE_DEVICE_ID,BOAT_DEVICE_ID,++g_txSequence,0,now,g_sessionEpoch);
            hb.payload[0]=(uint8_t)(1U|(g_nrfReady?2U:0U)|(g_loraConfigured?4U:0U)|(boatState.emergencyStopConfirmed&&boatState.emergencyStop?8U:0U));hb.payload[1]=1;
            if(radio_frameSeal(hb,g_authKey) && g_lora.sendFrame(hb,now)){g_lastBackupTxMs=now;g_backupTxSent=true;}
        }
    }
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
    g_ackTracker.update(now);
    g_linkManager.tick(millis());
}

bool boatRadio_isConnected(){return g_linkManager.connected(millis());}
BoatRadioLink boatRadio_getActiveLink(){return toPublicLink(g_linkManager.active());}
BoatRadioState boatRadio_getState(){return toPublicState(g_linkManager.state());}
bool boatRadio_primaryConnected(){return g_linkManager.primaryHealthy(millis());}
bool boatRadio_backupConnected(){return g_linkManager.backupHealthy(millis());}
bool boatRadio_getCommandAck(uint16_t commandId,RadioCommandAckRecord &record){return g_ackTracker.get(commandId,record);}
uint8_t boatRadio_pendingCommandAcks(){return g_ackTracker.pendingCount();}
void boatRadio_setTransport(RadioSendFn transport){(void)transport;} // fixed in this adapter
void boatRadio_setReady(ManagedLink link,bool ready){g_linkManager.setTransportReady(link,ready);}
void boatRadio_reportValidPacket(ManagedLink link,bool authenticated,uint32_t receivedAtMs){g_linkManager.reportValidRx(link,authenticated,receivedAtMs);}
void boatRadio_reportFailure(ManagedLink link){g_linkManager.reportFailure(link);}
