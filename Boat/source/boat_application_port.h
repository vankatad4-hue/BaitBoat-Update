#pragma once
#include "boat_hardware_supervisor.h"
#include "boat_output_off_feedback_guard.h"
#include "boat_dual_radio_port.h"
// Current modular Boat uses ready-made ESP32-S3-DevKitC-1-N32R16V,
// MDD20A, EKM014 and
// existing ready-made power and actuator modules with removable harnesses.
// No custom carrier PCB or live output pins exist in the default build.
// Driver contract: the installed module-specific port must obtain real local
// samples and enforce the software command lease and output constraints.
// The ESP32 internal watchdog provides software recovery, not an independent
// power disconnect. There is deliberately no default working driver.
struct BoatPhysicalDriver {
 bool (*read)(BoatHardwareSample &);
 bool (*apply)(const BoatActuatorOutput &,uint16_t maxLeaseMs);
 void (*disable)();
};
class BoatApplicationPort {
public:
 explicit BoatApplicationPort(BoatDualRadioPort &radio):radio_(radio){}
 bool attach(const BoatPhysicalDriver &driver,const BoatActuatorConfig &config);
 // Copy optional sensor values from the existing application/I2C owner.
 // These telemetry-only fields never enter actuator permission decisions.
 void updateSensorTelemetry(const BoatHardwareSample &sample);
 void tick();
private:
 static BoatApplicationPort *active_;
 static bool readMission(ExecInputs &in);
 static bool prepareGate();
 BoatDualRadioPort &radio_;
 BoatPhysicalDriver driver_{};
 BoatActuatorSupervisor executor_;
 BoatHardwareSample sample_{};
 BoatHardwareSample sensorTelemetry_{};
 BoatBatteryMonitor batteryMonitor_{};
 bool attached_=false,sampleOK_=false;
 // A physical driver apply() failure has uncertain actuator state. Do not
 // permit clearing STOP within this application instance after such a fault.
 // An authorized cold commissioning reset and measured driver recovery are
 // required, never radio-only recovery.
 bool driverFaultLatched_=false;
 boat_r30h26::OutputOffFeedbackGuard outputOffFeedback_{};
 uint16_t releaseTransaction_=0;
 uint32_t lastStatusAt_=0,lastDataAt_=0,generation_=0;
 uint8_t sampleSequence_=0;
 bool manualPending_=false;
 BoatCommand manualRelease_{};
 void finishManual(bool success);
 void publishData(uint32_t now);
 void fault();
};
