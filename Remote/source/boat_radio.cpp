#include "boat_radio.h"

static DualRadioManager g_linkManager;
static RadioSendFn g_transport = nullptr;
static bool g_begun = false;

static BoatRadioLink toPublicLink(ManagedLink link) {
    if (link == LINK_NRF_PRIMARY) return RADIO_LINK_NRF24_PRIMARY;
    if (link == LINK_LORA_BACKUP) return RADIO_LINK_LORA_BACKUP;
    return RADIO_LINK_NONE;
}
static BoatRadioState toPublicState(ManagedState state) {
    switch (state) {
    case LINK_PRIMARY: return RADIO_STATE_PRIMARY;
    case LINK_DEGRADED: return RADIO_STATE_DEGRADED;
    case LINK_BACKUP: return RADIO_STATE_BACKUP;
    case LINK_NO_LINK: return RADIO_STATE_NO_LINK;
    case LINK_RECOVERY: return RADIO_STATE_RECOVERY;
    default: return RADIO_STATE_DISCONNECTED;
    }
}

bool boatRadio_begin() {
    g_linkManager.begin(millis());
    g_transport = nullptr; // fail closed: no radio driver attached
    g_begun = true;
    return false; // radios remain offline until actual drivers are registered
}

bool boatRadio_sendCommand(const BoatCommand &command) {
    if (!g_begun) return false;
    return g_linkManager.send(command, g_transport, millis());
}

void boatRadio_update() {
    if (!g_begun) return;
    // Transport drivers must poll/read without blocking and report valid RX.
    // This function only updates health/failover state.
    g_linkManager.tick(millis());
}

bool boatRadio_isConnected() { return g_linkManager.connected(millis()); }
BoatRadioLink boatRadio_getActiveLink() { return toPublicLink(g_linkManager.active()); }
BoatRadioState boatRadio_getState() { return toPublicState(g_linkManager.state()); }
bool boatRadio_primaryConnected() { return g_linkManager.primaryHealthy(millis()); }
bool boatRadio_backupConnected() { return g_linkManager.backupHealthy(millis()); }

void boatRadio_setTransport(RadioSendFn transport) { g_transport = transport; }
void boatRadio_setReady(ManagedLink link, bool ready) { g_linkManager.setTransportReady(link, ready); }
void boatRadio_reportValidPacket(ManagedLink link, bool authenticated, uint32_t receivedAtMs) {
    g_linkManager.reportValidRx(link, authenticated, receivedAtMs);
}
void boatRadio_reportFailure(ManagedLink link) { g_linkManager.reportFailure(link); }
