// R30H-22: deterministic two-endpoint logical STOP / ACK / STATUS scheduler.
// Reuses production Remote retry, Remote ACK tracker, Boat ACK/STATUS outboxes,
// and each endpoint's REAL 32-byte HMAC/CRC and command/payload codecs.
// This simulator is NOT the ESP32 scheduler, the RF PHY, an actuator, nor proof
// of a measured physical STOP/current-zero response.
#include "radio_protocol.h"
#include "radio_command_codec.h"
#include "radio_payload_codec.h"
#include "radio_payload_encoder.h"
#include "radio_session_v2.h"
#include "../Boat/source/boat_stop_delivery.h"
#include "../Remote/source/remote_stop_delivery.h"
#include "../Remote/source/radio_ack_tracker.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>

struct Faults {
    uint32_t remoteBusyBefore=0, boatBusyBefore=0, physicalStoppedAt=0;
    unsigned dropStops=0, dropAcks=0, dropStatuses=0, failAckTx=0;
    bool dropAllStops=false, dropAllAcks=false, dropAllStatuses=false;
    bool feedbackUnknownForever=false;
    bool periodicBusy=false;
};

struct Sim {
    uint8_t epoch=7;
    uint8_t key[32]{};
    Faults faults{};
    flytec_r30h15::PrimaryStopRetry<BoatCommand> retry{};
    flytec_r30h17::StopConfirmationGate gate{};
    RadioAckTracker tracker{};
    boat_r30h18::StopAckOutbox ack{};
    boat_r30h19::StatusOutbox status{};
    RadioSessionCounter remoteSequence{},boatReceiveSequence{};
    RadioSessionCounter boatSequence{},remoteReceiveSequence{};
    bool physicalStopObserved=false,confirmed=false;
    unsigned stopAdmissions=0,stopReceipts=0,ackAdmissions=0,ackReceipts=0;
    unsigned statusAdmissions=0,statusReceipts=0,ordinaryBoatHeartbeats=0;
    unsigned ordinaryRemoteHeartbeats=0,rejectedFrames=0;
    RadioFrame capturedAck{};
    bool capturedAckValid=false;
    uint32_t lastNow=0;

    explicit Sim(Faults f={}):faults(f) {
        for(unsigned i=0;i<32;++i)key[i]=static_cast<uint8_t>(i+1);
        tracker.reset(epoch);
    }
    void press(uint16_t id,uint32_t now){
        assert(id!=0);
        BoatCommand cmd{};cmd.commandId=id;cmd.type=CMD_EMERGENCY_STOP;
        cmd.mode=MODE_MANUAL;cmd.createdAtMs=now;
        retry.queue(cmd,now);gate.request(id);confirmed=false;
        assert(tracker.begin(id,CMD_EMERGENCY_STOP,MODE_MANUAL,now,2500U));
    }
    void renewSession(uint8_t newEpoch){
        assert(newEpoch!=epoch);
        epoch=newEpoch;
        for(unsigned i=0;i<32;++i)key[i]^=static_cast<uint8_t>(newEpoch+i);
        remoteSequence.resetAfterHandshake();boatReceiveSequence.resetAfterHandshake();
        boatSequence.resetAfterHandshake();remoteReceiveSequence.resetAfterHandshake();
        retry.reset();ack.reset();status.reset();gate.reset();tracker.reset(epoch);
        confirmed=false;physicalStopObserved=false;
    }
    bool acceptBoat(const RadioFrame& f,uint32_t now){
        if(!radio_frameVerify(f,key)||f.sourceId!=1||f.destinationId!=2||
           f.sessionEpoch!=epoch||!radioLinkFlagMatches(f.flags,false)||
           !boatReceiveSequence.accept(f.sequence)){++rejectedFrames;return false;}
        BoatCommand cmd{};
        if(!radio_decodeCommand(f,1,2,now,2500,false,cmd,key)||
           cmd.type!=CMD_EMERGENCY_STOP||cmd.commandId!=f.commandId){
            ++rejectedFrames;return false;
        }
        physicalStopObserved=true;
        // Real production STOP receiver discards pre-STOP status and offers
        // a STOP command-ID ACK. NEVER synthesize measured motor feedback.
        status.reset();
        assert(ack.offer((uint16_t)cmd.commandId,epoch,RADIO_ACK_ACCEPTED,2,0,now));
        ++stopReceipts;
        return true;
    }
    bool acceptRemote(const RadioFrame& f,uint32_t now){
        if(!radio_frameVerify(f,key)||f.sourceId!=2||f.destinationId!=1||
           f.sessionEpoch!=epoch||!radioLinkFlagMatches(f.flags,false)||
           !remoteReceiveSequence.accept(f.sequence)){++rejectedFrames;return false;}
        RadioDecodedPayload decoded{};
        if(f.type==RADIO_MSG_ACK){
            if(!radio_decodeAckPayload(f.payload,decoded)||
               decoded.commandId!=f.commandId||decoded.commandType!=CMD_EMERGENCY_STOP){
                ++rejectedFrames;return false;
            }
            if(!tracker.observe(decoded,RADIO_ACK_LINK_PRIMARY,epoch,now))return false;
            ++ackReceipts;
            const bool matched=gate.acceptedAck(decoded.commandId,
                decoded.ackResult==RADIO_ACK_ACCEPTED||
                decoded.ackResult==RADIO_ACK_EXECUTED);
            return matched;
        }
        if(f.type==RADIO_MSG_TELEMETRY){
            if(!radio_decodeTelemetryPayload(f.payload,decoded)||
               decoded.kind!=RADIO_PAYLOAD_STATUS){++rejectedFrames;return false;}
            ++statusReceipts;
            if(gate.armed()&&gate.allowsStopStatus(true,decoded.stopState==1)){
                confirmed=true;retry.reset();gate.reset();
            }
            return true;
        }
        if(f.type==RADIO_MSG_HEARTBEAT)
            return radio_decodeHeartbeatPayload(f.payload,decoded);
        return false;
    }
    bool remoteStopTx(uint32_t now,bool busy){
        if(!retry.mayAttempt(true,true,busy,now))return false;
        BoatCommand cmd=retry.command();
        uint16_t seq=0;assert(remoteSequence.next(seq));
        cmd.sequence=seq;cmd.createdAtMs=now;
        RadioFrame f{};assert(radio_encodeCommand(cmd,1,2,epoch,f,key));
        assert(sizeof(f)==RADIO_FRAME_SIZE&&radio_frameVerify(f,key));
        retry.noteAdmitted(now);
        ++stopAdmissions;
        if(faults.dropAllStops||faults.dropStops){
            if(faults.dropStops)--faults.dropStops;
            return true; // admitted, lost before Boat
        }
        assert(acceptBoat(f,now));
        return true;
    }
    bool boatAckTx(uint32_t now,bool busy){
        if(!ack.due(now,true,true,busy))return false;
        const auto ticket=ack.ticket();
        uint16_t seq=0;assert(boatSequence.next(seq));
        RadioFrame f{};
        radio_frameInit(f,RADIO_MSG_ACK,2,1,seq,ticket.commandId,now,epoch);
        assert(radio_encodeAckPayload(ticket.result,CMD_EMERGENCY_STOP,
                ticket.commandId,ticket.stopState,ticket.interlocks,f.payload));
        assert(radio_frameSeal(f,key));
        ack.admitted(now);++ackAdmissions;
        if(faults.failAckTx){
            --faults.failAckTx;
            ack.completed(false); // never delivered; bounded retry in Boat
            return true;
        }
        ack.completed(true); // RF24 TX completion ≠ application delivery
        capturedAck=f;capturedAckValid=true;
        if(faults.dropAllAcks||faults.dropAcks){
            if(faults.dropAcks)--faults.dropAcks;
            return true;
        }
        (void)acceptRemote(f,now);
        return true;
    }
    void samplePhysical(uint32_t now){
        if(!physicalStopObserved||now==0||now%200U!=0U)return;
        RadioDecodedPayload s{};
        s.kind=RADIO_PAYLOAD_STATUS;s.boatMode=0;s.batteryPercent=70;
        s.stopState=(faults.feedbackUnknownForever||now<faults.physicalStoppedAt)?2:1;
        assert(status.offer(s,now));
    }
    bool boatStatusTx(uint32_t now,bool busy){
        if(!status.due(now,true,true,busy,ack.pending()))return false;
        uint16_t seq=0;assert(boatSequence.next(seq));
        RadioFrame f{};radio_frameInit(f,RADIO_MSG_TELEMETRY,2,1,seq,0,now,epoch);
        // Production encoder comes from the compiled endpoint; not test bytes.
        assert(radio_encodeTelemetryPayload(status.status(),f.payload));
        assert(radio_frameSeal(f,key));status.sent();++statusAdmissions;
        if(faults.dropAllStatuses||faults.dropStatuses){
            if(faults.dropStatuses)--faults.dropStatuses;
            return true;
        }
        (void)acceptRemote(f,now);
        return true;
    }
    void boatHeartbeat(uint32_t now,bool busy){
        if(now%100U!=0U ||
           !boat_r30h21::ordinarySlotFree({true,true,busy,ack.pending(),status.pending()}))
            return;
        uint16_t seq=0;assert(boatSequence.next(seq));
        RadioFrame f{};radio_frameInit(f,RADIO_MSG_HEARTBEAT,2,1,seq,0,now,epoch);
        assert(radio_encodeHeartbeatPayload(1,1,f.payload));
        assert(radio_frameSeal(f,key));++ordinaryBoatHeartbeats;
        (void)acceptRemote(f,now);
    }
    void remoteHeartbeat(uint32_t now,bool busy){
        if(now%100U!=0U||!retry.maySendPeriodic(true,busy))return;
        ++ordinaryRemoteHeartbeats; // no control/actuator authority in harness
    }
    void tick(uint32_t now){
        lastNow=now;
        if(retry.expired(now))retry.reset();
        const bool remoteBusy=(now<faults.remoteBusyBefore)||
             (faults.periodicBusy && now%100U==0U);
        const bool boatBusy=(now<faults.boatBusyBefore)||
             (faults.periodicBusy && now%125U==0U);
        // Actual priorities: Remote STOP retry before its heartbeat;
        // Boat RX/STOP ACK -> newest sampled STATUS -> ordinary heartbeat.
        const bool remoteStopUsed=remoteStopTx(now,remoteBusy);
        remoteHeartbeat(now,remoteBusy||remoteStopUsed);
        samplePhysical(now);
        bool boatSlotBusy=boatBusy;
        if(boatAckTx(now,boatSlotBusy))boatSlotBusy=true;
        if(boatStatusTx(now,boatSlotBusy))boatSlotBusy=true;
        boatHeartbeat(now,boatSlotBusy);
        tracker.update(now);
    }
    void run(uint32_t start,uint32_t end,uint32_t increment=25U){
        for(uint32_t now=start;now<=end;now+=increment)tick(now);
    }
};
static void success(const char *label,Faults f,uint32_t minTime,uint32_t maxTime){
    Sim s(f);s.press(501,0);
    bool before=false;uint32_t confirmedAt=0;
    for(uint32_t t=0;t<=3200;t+=25U){
        before=s.confirmed;s.tick(t);
        if(!before&&s.confirmed){confirmedAt=t;break;}
    }
    assert(s.confirmed);
    assert(confirmedAt>=minTime&&confirmedAt<=maxTime);
    assert(s.stopAdmissions>0&&s.stopReceipts>0);
    assert(s.ackAdmissions>0&&s.ackReceipts>0);
    assert(s.statusAdmissions>0&&s.statusReceipts>0);
    assert(s.retry.transmissions()==0&&!s.retry.pending());
    assert(s.gate.armed()==false);
    std::cout<<"  PASS "<<label<<" (logical tick "<<confirmedAt<<")\n";
}
static void failure(const char *label,Faults f,bool expectBoatSawStop){
    Sim s(f);s.press(601,0);s.run(0,3500);
    assert(!s.confirmed);
    assert(!s.retry.pending()); // 2.5-s admission queue expired/bounded
    assert(s.gate.armed()); // STOP latch unresolved, not silently released
    assert(s.physicalStopObserved==expectBoatSawStop);
    assert(s.stopAdmissions<=8);
    assert(s.ackAdmissions<=8U*8U);
    if(f.remoteBusyBefore<=3500U)
        assert(s.ordinaryRemoteHeartbeats>0); // local retry budget exhausted
    else assert(s.ordinaryRemoteHeartbeats==0); // PHY remains occupied
    RadioCommandAckRecord record{};
    assert(s.tracker.get(601,record));
    assert(record.state==RADIO_CMD_ACK_TIMEOUT);
    std::cout<<"  PASS "<<label<<" (STOP unresolved; fail-closed)\n";
}
int main(){
    static_assert(RADIO_FRAME_SIZE==32,"radio frame must remain 32 bytes");
    success("normal physical feedback",Faults{},200,400);
    Faults busy{};busy.remoteBusyBefore=225;busy.boatBusyBefore=425;
    busy.physicalStoppedAt=450;busy.periodicBusy=true;
    success("RF busy slots and latest STATUS",busy,600,1100);
    Faults dropFirst{};dropFirst.dropStops=1;dropFirst.dropAcks=1;
    dropFirst.dropStatuses=1;dropFirst.physicalStoppedAt=200;
    success("lost first STOP, ACK and STATUS",dropFirst,400,1200);
    Faults txFail{};txFail.failAckTx=1;txFail.physicalStoppedAt=400;
    success("Boat ACK RF TX failure and bounded retry",txFail,400,1000);
    Faults unknownThenReady{};unknownThenReady.physicalStoppedAt=800;
    success("UNKNOWN feedback until later real stopped sample",unknownThenReady,800,1300);

    Faults neverStop{};neverStop.dropAllStops=true;
    failure("all command frames lost",neverStop,false);
    Faults neverAck{};neverAck.dropAllAcks=true;
    failure("no authenticated STOP ACK despite stopped feedback",neverAck,true);
    Faults neverStatus{};neverStatus.dropAllStatuses=true;
    failure("ACK received but no stopped STATUS",neverStatus,true);
    Faults noFeedback{};noFeedback.feedbackUnknownForever=true;
    failure("feedback unknown / physical STOP not attested",noFeedback,true);
    Faults permanentlyBusy{};permanentlyBusy.remoteBusyBefore=999999U;
    failure("no radio TX slot until STOP queue expires",permanentlyBusy,false);

    // No previously authenticated ACK or stopped STATUS may be reused after
    // a new STOP request, nor across a new epoch/key.
    Sim replay;replay.press(710,0);replay.run(0,225);
    assert(replay.confirmed&&replay.capturedAckValid);
    const RadioFrame previousAck=replay.capturedAck;
    replay.press(711,250);
    assert(!replay.acceptRemote(previousAck,250));
    assert(!replay.confirmed&&!replay.gate.ackAccepted());
    replay.run(250,650);
    assert(replay.confirmed);
    replay.renewSession(8);
    replay.press(712,700);
    assert(!replay.acceptRemote(previousAck,700));
    assert(!replay.confirmed&&!replay.gate.ackAccepted());
    replay.run(700,1300);
    assert(replay.confirmed);
    std::cout<<"  PASS duplicate ACK / new command / epoch reset and recovery\n";
    std::cout<<"R30H-22 two-endpoint STOP scheduler fault matrix PASS (HOST ONLY)\n";
}
