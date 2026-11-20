#include "boat_state.h"
#include "physical_button_scanner.h"
#include "release_button_guard.h"
#include <Wire.h>
#include <cassert>
TwoWire Wire;
static int released=-1,uiOk=0,stops=0,actions=0;
bool boatCommands_releaseHopperLeft(){released=0;return true;}
bool boatCommands_releaseHopperRight(){released=1;return true;}
bool boatCommands_releaseHookLeft(){released=2;return true;}
bool boatCommands_releaseHookRight(){released=3;return true;}
static void event(uint8_t id,bool pressed){if(!pressed)return;if(id==15)++uiOk;else if(id==0)++stops;else ++actions;}
static void sample(uint32_t time){physicalButtons_update(time);physicalButtons_update(time+26);}
int main(){
 initialiseBoatState();boatState.radioConnected=true;boatState.radioActiveLink=RADIO_UI_NRF_PRIMARY;boatState.emergencyStopConfirmed=true;boatState.emergencyStop=false;boatState.actuatorSafetyReady=true;
 physicalButtons_setCallback(event);assert(physicalButtons_begin());
 Wire.c=239;sample(100);Wire.c=255;sample(200);assert(uiOk==1&&released==-1);
 Wire.b=254;sample(1000);Wire.b=255;sample(2300);assert(releaseGuard_state()==RELEASE_GUARD_WAIT_CONFIRM);
 Wire.c=239;sample(2500);Wire.c=255;sample(3400);assert(released==0&&uiOk==1);
 released=-1;Wire.b=254;sample(4000);Wire.b=252;sample(4100);Wire.b=255;sample(5500);Wire.c=239;sample(5600);Wire.c=255;sample(6500);assert(released==-1);
 Wire.a=254;sample(7000);assert(stops==1);Wire.a=255;sample(7100);
 Wire.fault=true;sample(8000);assert(stops==2);Wire.a=247;Wire.fault=false;sample(8100);assert(actions==0);Wire.a=255;sample(8200);Wire.a=247;sample(8300);assert(actions==1);
 // Held button and OK at restart must not produce actions until neutral.
 Wire.a=254;Wire.c=239;assert(physicalButtons_begin());assert(stops==3);
 sample(9000);Wire.a=255;sample(9100);assert(uiOk==1);
 Wire.c=255;sample(9200);assert(uiOk==1);
 Wire.a=247;sample(9300);assert(actions==2);
}
