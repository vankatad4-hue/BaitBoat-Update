#pragma once
#include "FreeRTOS.h"
#include <deque>
#include <vector>
#include <mutex>
#include <cstring>
struct MockQueue { std::mutex lock; std::deque<std::vector<unsigned char>> entries; size_t capacity; size_t width; };
using QueueHandle_t=MockQueue*;
inline QueueHandle_t xQueueCreate(UBaseType_t count, UBaseType_t width) { return new MockQueue{{},{},count,width}; }
inline void vQueueDelete(QueueHandle_t q) { delete q; }
inline BaseType_t xQueueSend(QueueHandle_t q,const void* value,unsigned) {
 std::lock_guard<std::mutex> g(q->lock);
 if(q->entries.size()>=q->capacity)return pdFALSE;
 const auto* p=static_cast<const unsigned char*>(value);
 q->entries.emplace_back(p,p+q->width);return pdTRUE;
}
inline BaseType_t xQueueOverwrite(QueueHandle_t q,const void* value) {
 std::lock_guard<std::mutex> g(q->lock);
 if(q->capacity!=1)return pdFALSE;
 const auto* p=static_cast<const unsigned char*>(value);
 q->entries.clear();q->entries.emplace_back(p,p+q->width);return pdTRUE;
}
inline BaseType_t xQueueReceive(QueueHandle_t q,void* value,unsigned) {
 std::lock_guard<std::mutex> g(q->lock);
 if(q->entries.empty())return pdFALSE;
 std::memcpy(value,q->entries.front().data(),q->width);
 q->entries.pop_front();return pdTRUE;
}
inline BaseType_t xQueueReset(QueueHandle_t q) {
 std::lock_guard<std::mutex> g(q->lock);q->entries.clear();return pdTRUE;
}
