#include "radio_worker.h"
#include <cassert>
#include <iostream>
static void poll(void* p){static_cast<std::atomic<unsigned>*>(p)->fetch_add(1);}
int main(){
 flytec_rtos::RadioWorker w;
 std::atomic<unsigned> count{0};
 assert(!w.begin(nullptr,nullptr));
 failCore=0;assert(!w.begin(poll,&count));assert(!w.running());
 failCore=-1;
 for(int i=0;i<50;++i){
  assert(w.begin(poll,&count));assert(!w.begin(poll,&count));
  auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
  while(w.heartbeat()<3){assert(std::chrono::steady_clock::now()<deadline);vTaskDelay(1);}
  assert(!w.wrongCore());w.stop();w.stop();assert(!w.running());
  auto before=count.load();vTaskDelay(1);assert(count.load()==before);
 }
 while(activeTasks.load())vTaskDelay(1);
 std::cout<<"R30H-95 worker mock PASS\n";
}
