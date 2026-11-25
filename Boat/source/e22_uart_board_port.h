#pragma once
#include <Arduino.h>
#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
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
 explicit E22UartBoardPort(HardwareSerial &serial,uart_port_t uartPort):serial_(serial),uartPort_(uartPort),
  link_(serial_,setMode,auxReady,setReset,this,txCapacity){}
 bool begin(const E22BoardPins &p){
  if(commissioningLocked_)return false;
  applicationBeginAttempted_=true;active_=false;
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
 // Commissioning-only host operation, never called from application startup.
 // A successful lock is sticky until a fresh offline instance/reboot.
 bool lockForCommissioning(const flytec_rtos::E22CommissioningPermit& permit,
                           flytec_rtos::E22IsolationProbe isolation,void* context){
  if(commissioningLocked_||!active_||!permit.valid()||!isolation||!isolation(context)||
     link_.txPending()||link_.transportFault()||!e22BoardPinsValid(pins_))return false;
  commissioningLocked_=true;isolation_=isolation;isolationContext_=context;
  link_.disableAdmission();active_=false;
  return commissionCheck(commissioningDrain(500));
 }
 // Fresh fixture-prepared UART1 only; no serial.begin, pin writes, reset or
 // E22 configuration write here. A failed attachment remains sticky.
 bool attachOfflineRecovery(const E22BoardPins& p,const flytec_rtos::E22CommissioningPermit& permit,
                            flytec_rtos::E22IsolationProbe isolation,void* context){
  if(applicationBeginAttempted_||commissioningLocked_||active_||!permit.valid()||!isolation||
     !isolation(context)||uartPort_!=UART_NUM_1||!e22BoardPinsValid(p))return false;
  commissioningLocked_=true;isolation_=isolation;isolationContext_=context;pins_=p;
  link_.disableAdmission();
  return commissionCheck(uart_is_driver_installed(uartPort_)&&
    (commissioningModeMatches(false)||commissioningModeMatches(true))&&commissioningDrain(500));
 }
 bool commissioningBaudVerified(uint32_t baud,uint32_t limit){
  if(!commissioningAllowed()||limit<10U||limit>500U||!flytec_rtos::uartBaudWithinOnePercent(baud,baud))return false;
  const uint32_t started=millis();
  if(!commissioningDrain(limit)||!commissioningAllowed()||uart_set_baudrate(uartPort_,baud)!=ESP_OK)return commissionCheck(false);
  uint32_t actual=0;
  if(uart_get_baudrate(uartPort_,&actual)!=ESP_OK||!flytec_rtos::uartBaudWithinOnePercent(actual,baud))return commissionCheck(false);
  const uint32_t elapsed=millis()-started;
  return commissionCheck(elapsed<limit&&commissioningDrain(limit-elapsed)&&uint32_t(millis()-started)<limit);
 }
 // Owner-only shutdown. UART/module RF inactivity is not certified.
 void shutdownLocal(){link_.shutdownLocal();active_=false;}
 void update(){if(active_)link_.update();}
 bool sendFrame(const RadioFrame &f,uint32_t ms){return active_&&link_.sendFrame(f,ms);}
 bool receiveFrame(RadioFrame &f){return active_&&link_.receiveFrame(f);}
 bool ready()const{return active_&&link_.ready();}
 using Admission=E22UartLink::Admission;
 Admission trySendFrame(const RadioFrame& f,uint32_t ms){return active_?link_.trySendFrame(f,ms):Admission::Unavailable;}
 bool configured()const{return active_&&link_.configured();}
 bool transportFault()const{return link_.transportFault();}
 bool txPending()const{return link_.txPending();}
 flytec_rtos::E22TxState::Outcome txOutcome()const{return link_.txOutcome();}
private:
 friend class flytec_rtos::E22OfflinePort<E22UartBoardPort>;
 void commissioningLatchFault(){commissioningFault_=true;}
 uint32_t commissioningNow()const{return millis();}
 void commissioningWait(){delay(1);}
 bool commissioningHostBaud(uint32_t,uint32_t baud,uint32_t limit){return commissioningBaudVerified(baud,limit);}
 bool commissioningModeMatches(bool config){
  return commissioningAllowed()&&digitalRead(pins_.m0)==LOW&&digitalRead(pins_.m1)==(config?HIGH:LOW)&&digitalRead(pins_.nrst)==HIGH;
 }
 bool commissioningModeVerified(bool config,uint32_t limit){
  const uint32_t start=millis();
  if(!commissioningDrain(limit)||!commissioningAllowed())return commissionCheck(false);
  setMode(false,config,this);delay(3);
  while(commissioningAllowed()&&uint32_t(millis()-start)<limit){
   if(commissioningModeMatches(config)&&auxReady(this)){delay(3);return commissionCheck(uint32_t(millis()-start)<limit&&commissioningModeMatches(config)&&auxReady(this));}
   delay(1);
  }
  return commissionCheck(false);
 }
 bool commissioningRxCount(unsigned& count){
  size_t n=0;if(!commissioningAllowed()||uart_get_buffered_data_len(uartPort_,&n)!=ESP_OK||n>128U)return false;
  count=unsigned(n);return true;
 }
 bool commissioningReadByte(uint8_t& b){
  return commissioningAllowed()&&uart_read_bytes(uartPort_,&b,1,0)==1;
 }
 bool commissioningTxIdle(){return commissioningAllowed()&&uart_wait_tx_done(uartPort_,0)==ESP_OK&&auxReady(this);}
 bool commissioningConfigWrite(const uint8_t* cmd,unsigned n){
  uint32_t baud=0;
  if(!commissioningModeMatches(true)||!commissioningTxIdle()||uart_get_baudrate(uartPort_,&baud)!=ESP_OK||
     !flytec_rtos::uartBaudWithinOnePercent(baud,9600)||!flytec_rtos::e22CommissioningCommand(cmd,n,n==3?12:4))return false;
  // Direct FIFO operation: never waits for space or enables application TX.
  return uart_tx_chars(uartPort_,reinterpret_cast<const char*>(cmd),n)==int(n);
 }
 bool commissioningAllowed()const{return commissioningLocked_&&!commissioningFault_&&isolation_&&isolation_(isolationContext_);}
 bool commissionCheck(bool ok){if(!ok||!commissioningAllowed()){commissioningFault_=true;return false;}return true;}
 bool commissioningDrain(uint32_t limit){
  if(!commissioningAllowed()||limit<5U||limit>500U||uartPort_!=UART_NUM_1)return false;
  uint32_t baud=0;size_t buffered=0;uart_word_length_t bits;uart_parity_t parity;uart_stop_bits_t stop;
  const uint32_t started=millis();
  if(uart_get_baudrate(uartPort_,&baud)!=ESP_OK||
     (!flytec_rtos::uartBaudWithinOnePercent(baud,9600)&&!flytec_rtos::uartBaudWithinOnePercent(baud,115200))||
     uart_get_word_length(uartPort_,&bits)!=ESP_OK||bits!=UART_DATA_8_BITS||
     uart_get_parity(uartPort_,&parity)!=ESP_OK||parity!=UART_PARITY_DISABLE||
     uart_get_stop_bits(uartPort_,&stop)!=ESP_OK||stop!=UART_STOP_BITS_1)return false;
  if(uart_wait_tx_done(uartPort_,pdMS_TO_TICKS(limit-4U))!=ESP_OK)return false;
  // Extra full-character guard for the slower supported rate. Still not RF idle.
  delay(3);
  return commissioningAllowed()&&uint32_t(millis()-started)<limit&&
    uart_get_buffered_data_len(uartPort_,&buffered)==ESP_OK&&buffered==0&&serial_.available()==0&&auxReady(this);
 }
 static size_t txCapacity(void* ctx){
  auto& p=*static_cast<E22UartBoardPort*>(ctx);
  const int n=p.serial_.availableForWrite();return n>0?static_cast<size_t>(n):0U;
 }
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
 const uart_port_t uartPort_;
 bool commissioningLocked_=false,commissioningFault_=false,applicationBeginAttempted_=false;
 flytec_rtos::E22IsolationProbe isolation_=nullptr;void* isolationContext_=nullptr;
 E22BoardPins pins_={-1,-1,-1,-1,-1,-1,false,false,false};
 E22UartLink link_;
 bool active_=false;
};
