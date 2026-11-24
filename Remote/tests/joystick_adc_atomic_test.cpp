#include "remote_i2c_owner.h"
#include "joystick_adc.h"
#include <Wire.h>
#include <cassert>
TwoWire Wire;
uint32_t millis(){static uint32_t now=0;return ++now;}
static int scaled(uint16_t v){return (int)((long)v*4095L/32767L);}
int main(){
 assert(flytec_rtos::bindRemoteI2cOwner());
    int lx=11,ly=22,rx=33,ry=44;
    assert(!joystickAdc_isReady());
    assert(!joystickAdc_read(lx,ly,rx,ry));
    assert(lx==11&&ly==22&&rx==33&&ry==44);
    assert(!joystickAdc_begin({0x47,0,1,2,3}));
    assert(!joystickAdc_begin({0x4A,0,1,1,3}));
    assert(joystickAdc_begin({0x4A,0,1,2,3}));
    assert(joystickAdc_isReady());
    assert(joystickAdc_read(lx,ly,rx,ry));
    assert(lx==scaled(4095)&&ly==scaled(8192)&&rx==scaled(16384)&&ry==scaled(24576));
    const int a=lx,b=ly,c=rx,d=ry;
    Wire.failChannel=2;
    assert(!joystickAdc_read(lx,ly,rx,ry));
    assert(lx==a&&ly==b&&rx==c&&ry==d);
    Wire.failChannel=-1;
    Wire.channels[3]=0xFFFFU;
    assert(!joystickAdc_read(lx,ly,rx,ry));
    assert(lx==a&&ly==b&&rx==c&&ry==d);
    Wire.channels[3]=24576;
    assert(joystickAdc_read(lx,ly,rx,ry));
    assert(!joystickAdc_begin({0x4A,0,0,2,3}));
    assert(!joystickAdc_isReady());
    assert(!joystickAdc_read(lx,ly,rx,ry));
}
