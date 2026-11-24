#ifdef FLYTEC_R30H120_PHYSICAL_COMPILE
#include "r30h120_physical_task.h"
#include "boat_radio_hw_config.h"
static_assert(!BOAT_NRF_PIN_AUDIT_SIGNED && !BOAT_E22_UART_PIN_AUDIT_SIGNED && !BOAT_E22_RF_AIRTIME_AUDIT_SIGNED,"Compile-only target requires unsigned hardware");
using namespace flytec_r30h118;
static Transport<RadioFrame> nb,eb;
static PhyEvents<RadioFrame> ne,ee;
static flytec_r30h120::PhysicalTask::Config cfg{
 BOAT_NRF_CE_PIN,BOAT_NRF_CSN_PIN,&SPI,BOAT_NRF_CHANNEL,
 BOAT_NRF_SCK_PIN,BOAT_NRF_MISO_PIN,BOAT_NRF_MOSI_PIN,
 {BOAT_E22_UART_PIN_TX,BOAT_E22_UART_PIN_RX,BOAT_E22_UART_PIN_M0,BOAT_E22_UART_PIN_M1,BOAT_E22_UART_PIN_AUX,BOAT_E22_UART_PIN_NRST,false,false,false},false,false};
static flytec_r30h120::HandshakeLane<RadioFrame> nh,eh;
static flytec_r30h120::SharedPriority np,ep;
static flytec_r30h120::PhysicalTask task(cfg,nb,eb,ne,ee);
// No hardware initialization or task launch.
void setup(){
 Serial.begin(115200);
 const bool aliasRejected=!task.bindShared(nh,nh,np,ep,1,2);
 const bool bound=task.bindShared(nh,eh,np,ep,1,2);
 const bool duplicateRejected=!task.bindShared(nh,eh,np,ep,1,2);
 const bool locked=!task.start(false,false);
 task.stop();
 Serial.println(aliasRejected&&bound&&duplicateRejected&&locked?"SHARED PHYSICAL WORKER LOCKED":"BINDING CONTRACT FAILED");
}
void loop(){delay(1000);}
#endif
