#pragma once
#include "FreeRTOS.h"
#include <mutex>
using SemaphoreHandle_t=std::mutex*;
inline SemaphoreHandle_t xSemaphoreCreateMutex() { return new std::mutex; }
inline void vSemaphoreDelete(SemaphoreHandle_t m) { delete m; }
inline BaseType_t xSemaphoreTake(SemaphoreHandle_t m,unsigned) {
 return m->try_lock()?pdTRUE:pdFALSE;
}
inline BaseType_t xSemaphoreGive(SemaphoreHandle_t m) {
 m->unlock();return pdTRUE;
}
