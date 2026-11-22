#pragma once
#include <stdint.h>

// R30H-3: on-demand Remote-LOCAL STOP request/TX-admission trace.
// NEVER ACK evidence, radio-delivery proof, physical current-zero or authority.
// No heap, I/O, callbacks, control messages or transmit-side timing changes.
namespace remote_r30h3 {
enum StopEvent : uint8_t {
 STOP_BUTTON_REQUEST=1,
 NRF_BUFFER_ADMITTED=2,
 NRF_BUFFER_REJECTED=3,
 E22_PACING_DEFERRED=4,
 E22_UART_BUFFER_ADMITTED=5,
 E22_UART_BUFFER_REJECTED_PENDING=6
};
struct Record {
 uint32_t localMs;
 uint16_t commandId;
 uint8_t event;
 uint8_t link; // 0=command admission; 1=nRF; 2=E22
};
class StopTrace {
public:
 static constexpr uint8_t CAPACITY=16;
 void record(StopEvent event,uint32_t ms,uint16_t id,uint8_t link){
  if(size_>=CAPACITY){++overflows_;return;}
  entries_[tail_]={ms,id,(uint8_t)event,link};
  tail_=(uint8_t)((tail_+1U)%CAPACITY);++size_;
 }
 bool take(Record &out){
  if(!size_)return false;
  out=entries_[head_];head_=(uint8_t)((head_+1U)%CAPACITY);--size_;return true;
 }
 uint8_t size()const{return size_;}
 uint32_t overflows()const{return overflows_;}
 void reset(){head_=tail_=size_=0;overflows_=0;}
private:
 Record entries_[CAPACITY]{};
 uint8_t head_=0,tail_=0,size_=0;
 uint32_t overflows_=0;
};
inline const char *name(uint8_t event){
 switch(event){
 case STOP_BUTTON_REQUEST:return "STOP_BUTTON_REQUEST";
 case NRF_BUFFER_ADMITTED:return "NRF_BUFFER_ADMITTED";
 case NRF_BUFFER_REJECTED:return "NRF_BUFFER_REJECTED";
 case E22_PACING_DEFERRED:return "E22_PACING_DEFERRED";
 case E22_UART_BUFFER_ADMITTED:return "E22_UART_BUFFER_ADMITTED";
 case E22_UART_BUFFER_REJECTED_PENDING:return "E22_UART_BUFFER_REJECTED_PENDING";
 default:return "INVALID_STOP_EVENT";
 }
}
} // namespace remote_r30h3
