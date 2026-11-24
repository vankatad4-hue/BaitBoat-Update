#pragma once
#include <thread>
#include <atomic>
#include <chrono>
inline thread_local int mockCore=0;
inline std::atomic<int> failCore{-1}, activeTasks{0};
inline int xPortGetCoreID(){return mockCore;}
inline int xTaskCreatePinnedToCore(void(*fn)(void*),const char*,unsigned,void* arg,unsigned,void*,int core){
 if(failCore.load()==core)return 0;
 ++activeTasks;
 std::thread([=]{mockCore=core;fn(arg);--activeTasks;}).detach();
 return pdPASS;
}
inline void vTaskDelay(unsigned ticks){std::this_thread::sleep_for(std::chrono::milliseconds(ticks?1:0));}
inline void vTaskDelete(void*){}
