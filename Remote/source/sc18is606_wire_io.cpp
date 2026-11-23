#include "sc18is606_wire_io.h"
bool Sc18is606WireIo::write(uint8_t addr,const uint8_t*bytes,size_t count){
    if(!bytes||count==0||count>34U)return false; // fixed 32-byte nRF limit
    wire_.beginTransmission(addr);
    if(wire_.write(bytes,count)!=count){(void)wire_.endTransmission();return false;}
    return wire_.endTransmission()==0;
}
bool Sc18is606WireIo::read(uint8_t addr,uint8_t*bytes,size_t count){
    if(!bytes||count==0||count>Sc18is606Port::MAX_TRANSFER)return false;
    if(wire_.requestFrom((int)addr,(int)count)!=(int)count)return false;
    for(size_t i=0;i<count;i++){
        const int v=wire_.read();
        if(v<0)return false;
        bytes[i]=(uint8_t)v;
    }
    return true;
}
