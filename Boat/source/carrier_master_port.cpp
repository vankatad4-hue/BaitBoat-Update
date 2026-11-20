#include "carrier_master_port.h"
bool CarrierMasterPort::send(carrier::Op op,int16_t left,int16_t right,int8_t coil){
 if(sequence_==UINT32_MAX)return false;
 const carrier::Command c{op,++sequence_,left,right,coil};uint8_t bytes[carrier::SIZE];
 if(!carrier::encode(c,bytes))return false;
 wire_.beginTransmission(carrier::ADDRESS);
 if(wire_.write(bytes,sizeof(bytes))!=sizeof(bytes)){wire_.endTransmission();return false;}
 if(wire_.endTransmission()!=0){haveStatus_=false;return false;}
 return true;
}
bool CarrierMasterPort::syncOff(){haveStatus_=false;return send(carrier::SYNC_OFF,0,0,-1);}
bool CarrierMasterPort::disarm(){haveStatus_=false;return send(carrier::DISARM,0,0,-1);}
bool CarrierMasterPort::command(int16_t left,int16_t right,int8_t coil){
 const uint8_t required=carrier::STOP_OK|carrier::WATCHDOG_OK|carrier::INTERLOCK_OK|carrier::GATE_ON|carrier::FEEDBACK_OK;
 if((left||right||coil>=0)&&(!healthy(millis())||lastState_!=carrier::ARMED||(lastFlags_&required)!=required))return false;
 return send(carrier::OUTPUT,left,right,coil);
}
bool CarrierMasterPort::readStatus(carrier::Status &status){
 uint8_t bytes[carrier::SIZE];
 if(wire_.requestFrom(carrier::ADDRESS,carrier::SIZE)!=(size_t)carrier::SIZE){haveStatus_=false;return false;}
 for(auto &b:bytes){const int v=wire_.read();if(v<0){haveStatus_=false;return false;}b=(uint8_t)v;}
 carrier::Status s{};
 if(!carrier::decodeStatus(bytes,sizeof(bytes),s)||s.echoed!=sequence_||(haveStatus_&&s.sample==lastSample_)||s.state==carrier::FAULT){haveStatus_=false;return false;}
 lastState_=s.state;lastFlags_=s.flags;lastSample_=s.sample;at_=millis();haveStatus_=true;status=s;return true;
}
