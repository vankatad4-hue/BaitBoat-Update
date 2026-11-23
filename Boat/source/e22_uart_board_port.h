#pragma once
#include <Arduino.h>
#include "e22_uart_link.h"
// External UART E22 adapter. No automatic GPIO selection or actuator control.
struct E22BoardPins { int8_t tx,rx,m0,m1,aux,nrst; bool audited,rfVerified,levelsVerified; };
inline bool e22BoardPinsValid(const E22BoardPins &p) {
 if(!p.audited||!p.rfVerified||!p.levelsVerified)return false;
 int v[]={p.tx,p.rx,p.m0,p.m1,p.aux,p.nrst};
 for(int i=0;i<6;i++){if(v[i]<0||v[i]>48)return false;
  for(int j=0;j<i;j++)if(v[i]==v[j])return false;}
 return true;
}
class E22UartBoardPort {
public:
 explicit E22UartBoardPort(HardwareSerial &serial):serial_(serial),
  link_(serial_,setMode,auxReady,setReset,this){}
 bool begin(const E22BoardPins &p){
  active_=false;
  if(!e22BoardPinsValid(p))return false;
  pins_=p;
  pinMode(p.m0,OUTPUT);digitalWrite(p.m0,LOW);
  pinMode(p.m1,OUTPUT);digitalWrite(p.m1,LOW);
  pinMode(p.aux,INPUT);
  // Drive active-low reset released before E22 configuration begins.
  digitalWrite(p.nrst,HIGH);pinMode(p.nrst,OUTPUT);
  serial_.begin(9600,SERIAL_8N1,p.rx,p.tx);
  active_=link_.configureEU868TestProfile();
  return active_;
 }
 void update(){if(active_)link_.update();}
 bool sendFrame(const RadioFrame &f,uint32_t ms){return active_&&link_.sendFrame(f,ms);}
 bool receiveFrame(RadioFrame &f){return active_&&link_.receiveFrame(f);}
 bool ready()const{return active_&&link_.ready();}
private:
 static void setMode(bool m0,bool m1,void *ctx){
  auto *p=static_cast<E22UartBoardPort*>(ctx);
  digitalWrite(p->pins_.m0,m0?HIGH:LOW);
  digitalWrite(p->pins_.m1,m1?HIGH:LOW);
 }
 static void setReset(bool released,void *ctx){
  auto *p=static_cast<E22UartBoardPort*>(ctx);
  digitalWrite(p->pins_.nrst,released?HIGH:LOW);
 }
 static bool auxReady(void *ctx){
  auto *p=static_cast<E22UartBoardPort*>(ctx);
  return digitalRead(p->pins_.aux)==HIGH;
 }
 HardwareSerial &serial_;
 E22BoardPins pins_={-1,-1,-1,-1,-1,-1,false,false,false};
 E22UartLink link_;
 bool active_=false;
};
