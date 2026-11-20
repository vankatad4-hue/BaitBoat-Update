#include "radio_key_store.h"
#include <string.h>

// TODO: provision a per-remote/boat 256-bit key into encrypted NVS, add
// recovery/rotation procedure, and protect debug output. Fail closed for now.
bool radioKey_get(uint8_t keyOut[32]) {
    if (!keyOut) return false;
    memset(keyOut,0,32);
    return false;
}
