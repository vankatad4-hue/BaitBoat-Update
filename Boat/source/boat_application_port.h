#pragma once
#include "boat_hardware_supervisor.h"
#include "boat_dual_radio_port.h"
// R15: hardware port uses ONLY ready-made Heltec, MDD20A, EKM014 and
// independently reviewed commercial safety modules with removable harnesses.
// No custom carrier PCB or live output pins exist in the default build.
// Driver contract: the installed module-specific port must obtain real local
// samples and enforce an independent electrical watchdog. apply() must reject
// output if its hardware interlocks/lease have failed, including when the MCU
// cannot service the loop. There is deliberately no default working driver.
struct BoatPhysicalDriver {
 bool (*read)(BoatHardwareSample &);
 bool (*apply)(const BoatActuatorOutput &,uint16_t maxLeaseMs);
 void (*disable)();
};
class BoatApplicationPort {
public:
 explicit BoatApplicationPort(BoatDualRadioPort &radio):radio_(radio){}
 bool attach(const BoatPhysicalDriver &driver,const BoatActuatorConfig &config);
 void tick();
private:
 static BoatApplicationPort *active_;
 static bool readMission(ExecInputs &in);
 static bool prepareGate();
 BoatDualRadioPort &radio_;
 BoatPhysicalDriver driver_{};
 BoatActuatorSupervisor executor_;
 BoatHardwareSample sample_{};
 bool attached_=false,sampleOK_=false;
 uint16_t releaseTransaction_=0;
 uint32_t lastStatusAt_=0,lastDataAt_=0,generation_=0;
 uint8_t sampleSequence_=0;
 bool manualPending_=false;
 BoatCommand manualRelease_{};
 void finishManual(bool success);
 void publishData(uint32_t now);
 void fault();
};
