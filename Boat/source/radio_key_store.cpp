#include "radio_key_store.h"
#include <string.h>
bool radioKey_get(uint8_t keyOut[32]) {
    if(!keyOut) return false;
    memset(keyOut,0,32);
    // TODO: secure NVS provisioning. No shared test key is embedded.
    return false;
}
