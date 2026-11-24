#pragma once
// Each host thread has a distinct simulated task identity.
inline void* xTaskGetCurrentTaskHandle(){static thread_local int token;return &token;}
