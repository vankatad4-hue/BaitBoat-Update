#ifndef BOAT_RADIO_H
#define BOAT_RADIO_H
#include <Arduino.h>
#include "boat_commands.h"
#include "dual_radio_manager.h"
#include "radio_ack_tracker.h"
#include "r30h3_stop_trace.h"
#include "r30h6_utc_marker_policy.h"
#include "mission_upload.h"
#include "mission_control_wire.h"

enum BoatRadioLink : uint8_t {
    RADIO_LINK_NONE = 0,
    RADIO_LINK_NRF24_PRIMARY,
    RADIO_LINK_LORA_BACKUP
};
enum BoatRadioState : uint8_t {
    RADIO_STATE_DISCONNECTED = 0,
    RADIO_STATE_PRIMARY,
    RADIO_STATE_DEGRADED,
    RADIO_STATE_BACKUP,
    RADIO_STATE_NO_LINK,
    RADIO_STATE_RECOVERY
};

// Remote dual-radio adapter: nRF and E22 transports, authenticated frame
// validation, link supervision and heartbeat scaffolds. Pin map is provisional;
// key provisioning is fail-closed and commands are disabled without a key.
bool boatRadio_begin();
// Preparation only, never starts motors or release mechanisms.
bool boatRadio_prepareMission(MissionPlan plan);
MissionUploadState boatRadio_missionUploadState();
void boatRadio_cancelMissionPreparation();
bool boatRadio_missionControl(MissionControlOp op);
MissionControlPhase boatRadio_missionPhase();
bool boatRadio_missionFeedbackFresh();
bool boatRadio_missionControlPending();
bool boatRadio_missionControlUnknown();
MissionControlResult boatRadio_missionControlResult();
bool boatRadio_missionLocked();
bool boatRadio_manualModeConfirmed();
bool boatRadio_backupStopConfirmed(); // authenticated backup STOP evidence, not movement authorization
bool boatRadio_stopConfirmationPending();
bool boatRadio_sendMissionSticks(int16_t left,int16_t right,bool neutral);
bool boatRadio_sendCommand(const BoatCommand &command);
void boatRadio_update();
// Experimental owner-task lifecycle callbacks. Never start hardware.
void boatRadio_workerFault(bool primaryFault,bool backupFault);
void boatRadio_retireWorker();
// R30H-5: estimated UTC from authenticated Boat GPS, only for UI/RTC.
// This is NOT PPS, verified E2E STOP timing, radio lease or navigation safety.
bool boatRadio_utcNow(uint64_t &unixMs);
// Informational only. Coarse GPS UTC is NOT a shared physical stop-latency clock.
flytec_r30h6::ClockReport boatRadio_utcQuality();
bool boatRadio_isConnected();
BoatRadioLink boatRadio_getActiveLink();
BoatRadioState boatRadio_getState();
bool boatRadio_primaryConnected();
bool boatRadio_backupConnected();
bool boatRadio_getCommandAck(uint16_t commandId,RadioCommandAckRecord &record);
uint8_t boatRadio_pendingCommandAcks();
// Read only the Remote-LOCAL STOP intent/queue/driver-buffer timeline.
// false/0 when R30H-3 trace is disabled; cannot authorize control/ACK.
bool boatRadio_takeR30H3StopTrace(remote_r30h3::Record &out);
uint32_t boatRadio_r30h3TraceOverflows();

// Hardware-driver integration hooks. A valid RX must pass frame validation and
// HMAC authentication before reporting authenticated=true.
void boatRadio_setTransport(RadioSendFn transport);
void boatRadio_setReady(ManagedLink link, bool ready);
void boatRadio_reportValidPacket(ManagedLink link, bool authenticated, uint32_t receivedAtMs);
void boatRadio_reportFailure(ManagedLink link);
#endif
