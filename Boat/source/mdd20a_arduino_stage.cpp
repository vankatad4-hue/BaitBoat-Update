#include "mdd20a_arduino_stage.h"
void Mdd20aArduinoStage::outputOff() {
 if(pins_.permit>=0)digitalWrite(pins_.permit,LOW);
 if(pwmLeftAttached_)ledcWrite(pins_.pwmLeft,0);
 if(pwmRightAttached_)ledcWrite(pins_.pwmRight,0);
 if(pins_.dirLeft>=0)digitalWrite(pins_.dirLeft,LOW);
 if(pins_.dirRight>=0)digitalWrite(pins_.dirRight,LOW);
 if(pins_.watchdogEdge>=0)digitalWrite(pins_.watchdogEdge,LOW);
}
void Mdd20aArduinoStage::disable() {
 outputOff();
 safety_.fault();ready_=false;
}
bool Mdd20aArduinoStage::begin(const Mdd20aDirectProfile &p,uint64_t allowed,
                               bool reverseL,bool reverseR,uint16_t pwmMax,uint16_t reverseMs) {
 // Call only before actuator supply is connected. Pin output selection is
 // hardware safety-critical; unsigned or duplicate assignments fail closed.
 if(ready_||pins_.permit>=0||!mdd20aDirectProfileValid(p,allowed)||
    !safety_.configure(p,allowed,reverseL,reverseR,pwmMax,reverseMs))return false;
 pins_=p;
 pinMode(pins_.permit,OUTPUT);digitalWrite(pins_.permit,LOW);
 pinMode(pins_.watchdogEdge,OUTPUT);digitalWrite(pins_.watchdogEdge,LOW);
 pinMode(pins_.dirLeft,OUTPUT);digitalWrite(pins_.dirLeft,LOW);
 pinMode(pins_.dirRight,OUTPUT);digitalWrite(pins_.dirRight,LOW);
 pinMode(pins_.pwmLeft,OUTPUT);digitalWrite(pins_.pwmLeft,LOW);
 pinMode(pins_.pwmRight,OUTPUT);digitalWrite(pins_.pwmRight,LOW);
 // Core 3.x pin-based LEDC; distinct channels prevent accidental sharing.
 pwmLeftAttached_=ledcAttachChannel(pins_.pwmLeft,20000,10,0);
 pwmRightAttached_=ledcAttachChannel(pins_.pwmRight,20000,10,1);
 if(!pwmLeftAttached_||!pwmRightAttached_) {disable();return false;}
 outputOff();ready_=true;return true;
}
bool Mdd20aArduinoStage::write(const BoatActuatorOutput &request,
                               const Mdd20aDirectFeedback &feedback,uint32_t now) {
 if(!ready_)return false;
 CarrierOutputWords out{};
 if(!safety_.prepare(request,feedback,now,out)) {disable();return false;}
 // HAL owns MDD20A PWM/DIR only. Solenoid pulse source must be separate.
 // Independent electrical gate is confirmed, not auto-armed, here.
 if(!ledcWrite(pins_.pwmLeft,0)||!ledcWrite(pins_.pwmRight,0)) {disable();return false;}
 digitalWrite(pins_.dirLeft,out.dirLeft?HIGH:LOW);
 digitalWrite(pins_.dirRight,out.dirRight?HIGH:LOW);
 if(!ledcWrite(pins_.pwmLeft,out.pwmLeft)||!ledcWrite(pins_.pwmRight,out.pwmRight)) {
  disable();return false;
 }
 // WDI intentionally not toggled here; it must be serviced only by a
 // separately verified completed-application-cycle path. Merely writing PWM
 // must never keep the external watchdog alive.
 return true;
}
