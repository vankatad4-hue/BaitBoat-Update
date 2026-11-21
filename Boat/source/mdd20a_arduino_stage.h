#pragma once
#include <Arduino.h>
#include "mdd20a_direct_safety.h"
// Arduino-ESP32 v3 LEDC implementation. Not attached to BoatApplicationPort
// until physical STOP, rail, watchdog, gate and motor feedback exist.
// No GPIO pins are selected in this repository.
class Mdd20aArduinoStage {
public:
 bool begin(const Mdd20aDirectProfile &profile,uint64_t auditedOutputs,
            bool reversedLeft,bool reversedRight,uint16_t maxDuty,
            uint16_t reverseDeadtimeMs);
 bool write(const BoatActuatorOutput &request,const Mdd20aDirectFeedback &feedback,uint32_t now);
 void disable();
 bool initialized() const{return ready_;}
private:
 Mdd20aDirectProfile pins_=mdd20aUnassignedProfile();
 Mdd20aDirectSafety safety_;
 bool ready_=false,pwmLeftAttached_=false,pwmRightAttached_=false;
 void outputOff();
};
