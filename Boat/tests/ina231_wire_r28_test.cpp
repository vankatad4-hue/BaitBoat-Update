#include "../source/ina231_arduino_wire_r28.h"
#include <cassert>
#include <iostream>
int main(){
 TwoWire w;
 Ina231ArduinoWireR28 bus(w);
 assert(w.begins==0);
 assert(bus.write16(0x40,0,0x4122));
 assert(w.begins==1&&w.address==0x40);
 assert(w.tx.size()==3&&w.tx[0]==0&&w.tx[1]==0x41&&w.tx[2]==0x22);
 w.rx={0x27,0x60};
 uint16_t v=0;
 assert(bus.read16(0x40,2,v)&&v==0x2760);
 assert(w.tx.size()==1 && w.tx[0]==2);
 w.failRequest=true;v=0xffff;
 assert(!bus.read16(0x40,6,v)&&v==0);
 w.failRequest=false;w.rx={0x41};
 assert(!bus.read16(0x40,0,v)&&v==0);
 w.rx={0x41,0x22};w.failRead=true;
 assert(!bus.read16(0x40,0,v));
 w.failRead=false;w.failEnd=true;
 assert(!bus.write16(0x40,0,0x4122));
 assert(!bus.read16(0x40,0,v));
 std::cout<<"R28 TwoWire register framing and fault tests PASS\n";
}
