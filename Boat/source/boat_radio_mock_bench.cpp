#if defined(FLYTEC_R30H118_MOCK_BENCH)
#include "boat_radio_mock_task.h"
using namespace flytec_r30h118;
static MockTask mock;
static uint32_t began=0;
static bool submitted=false,stopped=false,reported=false;
void setup(){Serial.begin(115200);began=millis();if(!mock.start()){Serial.println("MOCK START FAIL");stopped=true;reported=true;}}
void loop(){
 if(!stopped&&mock.ready()&&!submitted)submitted=mock.submit(42);
 bool admitted=false;
 if(!stopped&&mock.poll(admitted)){
  Serial.println(admitted?"MOCK UNEXPECTED ADMISSION":"MOCK REJECTED AS EXPECTED");
  mock.stop();stopped=true;
 }
 if(!stopped&&uint32_t(millis()-began)>=5000U){Serial.println("MOCK TIMEOUT");mock.stop();stopped=true;}
 if(stopped&&!reported&&mock.finished()){
  reported=true;
  Serial.printf("MOCK EXIT calls=%lu max_us=%lu stack_raw=%lu\n",(unsigned long)mock.mockCalls(),(unsigned long)mock.maxStepUs(),(unsigned long)mock.stackHighWaterRaw());
 }
 delay(1);
}
#endif
