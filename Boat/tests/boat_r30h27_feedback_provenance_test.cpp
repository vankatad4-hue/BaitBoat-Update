// R30H-27 pure host evidence classification: real R30D diagnostic capture,
// command echoes, MCP23017 readback, spoofed source types, old values,
// simulated physical-current-zero and status-wire semantics. NO HARDWARE.
#include "../source/boat_r30h27_feedback_provenance_audit.h"
#include <cassert>
#include <cstdint>
#include <iostream>
using namespace boat_r30h27;
struct Reader final : boat_r30d::ReadOnlySource {
 boat_r30d::RawFrame input{};
 bool ok=true;
 bool read(boat_r30d::RawFrame &out) override {
  if(!ok)return false;
  out=input;return true;
 }
};
static boat_r30d::RawFrame nominal(uint32_t t){
 boat_r30d::RawFrame x{};
 x.stop={true,t,1};x.watchdog={true,t,2};
 x.commonDisconnectAux={true,t,3};x.coilPulseLimiter={true,t,4};
 x.coilCurrentZero={true,t,5};x.motorCurrentZero={true,t,6};
 x.battery={true,12200U,t,7};
 return x;
}
static Profile hypotheticalClaims(){
 Profile p{};
 for(unsigned i=0;i<7U;++i){
  auto &s=p.channels[i];
  s.origin=requiredOrigin(i);s.rawSourceId=(uint8_t)(i+1U);
  s.claimedPolarityChecked=true;s.claimedWiringReviewed=true;
  s.claimedIndependentMeasurementChecked=true;
 }
 return p;
}
int main(){
 static_assert(!PHYSICAL_FEEDBACK_SIGNOFF_COMPLETE,
               "Independent output-current-zero evidence NOT signed");
 static_assert(!boat_r30d::ACTUAL_READER_COMMISSIONED,
               "No physically connected common-bus diagnostic reader");
 static_assert(!boat_r30c::COMMON_BUS_HARDWARE_COMMISSIONED,
               "No commissioned independent cutoff on shared 3S bus");
 Reader reader{};boat_r30d::ReadOnlyCommonBusPort port;
 auto before=port.snapshot(1000U);
 assert(inspect(before,hypotheticalClaims(),1000U).verdict==
        Verdict::NO_VALID_DIAGNOSTIC);
 reader.input=nominal(1000U);
 auto c=port.sample(reader,1000U);
 assert(c.observed&&!c.canAuthorizeActuators);
 assert(c.result==boat_r30d::Result::OK_DIAGNOSTIC_ONLY);
 Profile p{};
 assert(inspect(c,p,1000U).verdict==Verdict::SOURCE_ID_MISMATCH);
 p=hypotheticalClaims();
 assert(inspect(c,p,1000U).verdict==Verdict::PHYSICAL_EVIDENCE_NOT_SIGNED);
 assert(!mayCertifyCurrentZero(inspect(c,p,1000U)));
 for(unsigned i=0;i<7U;++i){
  auto changed=p;
  changed.channels[i].rawSourceId=0;
  assert(inspect(c,changed,1000U).verdict==Verdict::SOURCE_ID_MISMATCH);
  changed=p;
  changed.channels[i].origin=Origin::SOFTWARE_COMMAND_ECHO;
  assert(inspect(c,changed,1000U).verdict==Verdict::COMMAND_OR_DERIVED_ECHO);
  changed.channels[i].origin=Origin::MCP_OUTPUT_LATCH_READBACK;
  assert(inspect(c,changed,1000U).verdict==Verdict::COMMAND_OR_DERIVED_ECHO);
  changed.channels[i].origin=Origin::MCU_DERIVED_ESTIMATE;
  assert(inspect(c,changed,1000U).verdict==Verdict::COMMAND_OR_DERIVED_ECHO);
  changed=p;changed.channels[i].origin=Origin::EXTERNAL_BUS_VOLTAGE_MEASUREMENT;
  if(i!=6U)assert(inspect(c,changed,1000U).verdict==
                 Verdict::ORIGIN_TYPE_INCOMPATIBLE);
  changed=p;changed.channels[i].claimedWiringReviewed=false;
  assert(inspect(c,changed,1000U).verdict==Verdict::SOURCE_REVIEW_MISSING);
  changed=p;changed.channels[i].claimedPolarityChecked=false;
  assert(inspect(c,changed,1000U).verdict==Verdict::SOURCE_REVIEW_MISSING);
  changed=p;changed.channels[i].claimedIndependentMeasurementChecked=false;
  assert(inspect(c,changed,1000U).verdict==Verdict::SOURCE_REVIEW_MISSING);
 }
 assert(inspect(c,p,1050U).verdict==Verdict::STALE_DIAGNOSTIC);
 // Simulated physical STOP/status telemetry may say STOP=1 but no independent
 // electrical current-zero verification has been obtained.
 BoatHardwareSample f{};
 f.safetyValid=true;f.outputFeedbackValid=true;f.watchdogReady=true;
 f.stationary=true;f.releaseIdle=true;f.safetyAt=1000U;
 f.physicalStopReleased=false;f.batteryBand=BOAT_BATTERY_GREEN;
 const auto status=boatHardware_status(f,1000U,true,MC_ABORTED,false,true);
 assert(status.stopState==1U); // protocol-level reported idle, NOT proof!
 assert(!mayCertifyCurrentZero(inspect(c,p,1000U)));
 f.leftMotorActive=true;f.stationary=false;
 assert(boatHardware_status(f,1000U,true,MC_ABORTED,false,true).stopState==2U);
 // A fault report is *observed* but must not certify safety or current-zero.
 reader.input=nominal(1100U);reader.input.stop.asserted=false;
 c=port.sample(reader,1100U);
 assert(inspect(c,p,1100U).verdict==Verdict::REPORTED_SAFETY_FAULT);
 reader.input=nominal(1120U);reader.input.motorCurrentZero.asserted=false;
 c=port.sample(reader,1120U);
 assert(inspect(c,p,1120U).verdict==Verdict::REPORTED_CURRENT_NOT_ZERO);
 reader.input=nominal(1140U);reader.input.coilCurrentZero.asserted=false;
 c=port.sample(reader,1140U);
 assert(inspect(c,p,1140U).verdict==Verdict::REPORTED_CURRENT_NOT_ZERO);
 reader.input=nominal(1200U);reader.input.watchdog.sourceId=1;
 c=port.sample(reader,1200U);
 assert(inspect(c,p,1200U).verdict==Verdict::NO_VALID_DIAGNOSTIC);
 reader.ok=false;
 c=port.sample(reader,1250U);
 assert(inspect(c,p,1250U).verdict==Verdict::NO_VALID_DIAGNOSTIC);
 reader.ok=true;reader.input=nominal(0xfffffffeU);
 c=port.sample(reader,1U);
 assert(inspect(c,p,1U).verdict==Verdict::PHYSICAL_EVIDENCE_NOT_SIGNED);
 assert(inspect(c,p,48U).verdict==Verdict::STALE_DIAGNOSTIC);
 std::cout<<"R30H-27 read-only feedback provenance and STOP current-zero separation PASS; physical commissioning remains denied\n";
}
