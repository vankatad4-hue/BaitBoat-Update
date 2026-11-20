/*
 * Flytec Boat non-actuating target application.
 * GPIO assignments are deliberately disabled until the precise Tracker revision,
 * board wiring and external hardware interlocks are verified.
 * No motor/solenoid pins are configured. Do not connect actuator power.
 */
#include <Arduino.h>
#include "boat_dual_radio_port.h"
static BoatDualRadioPort radio;
void setup(){
 Serial.begin(115200);
 Serial.println("Flytec Boat: diagnostic-only firmware; actuator output disabled.");
 const bool ready=radio.begin();
 Serial.println(ready?"Radio transport configured; awaiting secure session.":"Radio pin configuration unavailable.");
 // No physical stationary/STOP sensor validation exists yet, so leave
 // the mission preparation gate unset (default deny).
}
void loop(){
 radio.update();
 BoatCommand command{};bool duplicate=false;ManagedLink link=LINK_NONE;
 // Consume diagnostics without executing any accepted command or issuing
 // execution acknowledgements. Queue receipt never represents actuation.
 while(radio.takeCommand(command,duplicate,link)){}
 delay(1);
}
