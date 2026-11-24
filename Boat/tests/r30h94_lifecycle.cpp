#include "dual_core_diagnostic.h"
#include <cassert>
#include <iostream>
static void drain(){
 auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
 while(activeTasks.load()){assert(std::chrono::steady_clock::now()<deadline);vTaskDelay(1);}
}
int main(){
 flytec_rtos::DualCoreDiagnostic d;
 for(int core=0;core<2;++core){
  failCore=core;assert(!d.begin());assert(!d.snapshot().running);drain();
 }
 failCore=-1;
 for(int i=0;i<50;++i){
  assert(d.begin());assert(d.begin());
  auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
  while(d.snapshot().core0Ticks<2 || d.snapshot().core1Ticks<2){
   assert(std::chrono::steady_clock::now()<deadline);vTaskDelay(1);
  }
  auto s=d.snapshot();assert(s.running && !s.core0WrongCore && !s.core1WrongCore);
  d.stop();d.stop();assert(!d.snapshot().running);drain();
 }
 {flytec_rtos::DualCoreDiagnostic scoped;assert(scoped.begin());}
 drain();std::cout<<"R30H-94 mock lifecycle PASS\n";
}
