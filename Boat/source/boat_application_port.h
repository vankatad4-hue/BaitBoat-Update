#pragma once
#include "boat_hardware_supervisor.h"
#include "boat_dual_radio_port.h"
// Driver contract: the board-specific implementation must obtain real local
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
 uint32_t lastStatusAt_=0;
 void fault();
};
