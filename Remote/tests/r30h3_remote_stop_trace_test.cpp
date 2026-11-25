#include "../source/remote_stop_trace.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(){
 using namespace remote_r30h3;
 StopTrace trace;
 Record r{};
 assert(!trace.take(r));
 trace.record(STOP_BUTTON_REQUEST,100,23,0);
 trace.record(E22_PACING_DEFERRED,101,23,2);
 trace.record(NRF_BUFFER_REJECTED,102,23,1);
 trace.record(E22_UART_BUFFER_ADMITTED,3000,23,2);
 assert(trace.size()==4);
 assert(trace.take(r)&&r.event==STOP_BUTTON_REQUEST&&r.commandId==23);
 assert(trace.take(r)&&r.event==E22_PACING_DEFERRED&&r.link==2);
 assert(trace.take(r)&&r.event==NRF_BUFFER_REJECTED&&r.link==1);
 assert(trace.take(r)&&r.event==E22_UART_BUFFER_ADMITTED&&r.localMs==3000);
 assert(!trace.take(r));
 assert(strcmp(name(E22_UART_BUFFER_ADMITTED),"E22_UART_BUFFER_ADMITTED")==0);
 for(int i=0;i<StopTrace::CAPACITY;++i)
   trace.record(NRF_BUFFER_ADMITTED,(uint32_t)i,(uint16_t)i,1);
 trace.record(E22_UART_BUFFER_REJECTED_PENDING,40000,999,2);
 assert(trace.size()==StopTrace::CAPACITY && trace.overflows()==1);
 for(int i=0;i<StopTrace::CAPACITY;++i){
   assert(trace.take(r)&&r.commandId==i&&r.localMs==(uint32_t)i);
 }
 trace.reset();
 assert(trace.overflows()==0 && trace.size()==0);
 puts("R30H-3 Remote-local STOP request/transport-buffer trace passed (NO_RF)");
 return 0;
}
