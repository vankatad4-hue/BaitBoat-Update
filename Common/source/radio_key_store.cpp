#include "radio_key_store.h"
#include <string.h>
// Explicit opt-in only after secure boot, encrypted flash/NVS and entropy
// are verified on the physical device. Default Arduino builds stay disabled.
#if defined(FLYTEC_ENABLE_SECURE_PROVISIONED_RADIO) && FLYTEC_ENABLE_SECURE_PROVISIONED_RADIO && defined(CONFIG_NVS_ENCRYPTION) && CONFIG_NVS_ENCRYPTION
#include <nvs.h>
#include <esp_flash_encrypt.h>
#include <esp_secure_boot.h>
#endif
bool radioKey_get(uint8_t keyOut[32]){
 if(!keyOut)return false;
 memset(keyOut,0,32);
#if defined(FLYTEC_ENABLE_SECURE_PROVISIONED_RADIO) && FLYTEC_ENABLE_SECURE_PROVISIONED_RADIO && defined(CONFIG_NVS_ENCRYPTION) && CONFIG_NVS_ENCRYPTION
 if(!esp_flash_encryption_enabled()||!esp_secure_boot_enabled())return false;
 nvs_handle_t handle;
 if(nvs_open("flytec_radio",NVS_READONLY,&handle)!=ESP_OK)return false;
 size_t length=32;
 const esp_err_t result=nvs_get_blob(handle,"pair_key",keyOut,&length);
 nvs_close(handle);
 uint8_t nonzero=0;for(unsigned i=0;i<32;++i)nonzero|=keyOut[i];
 if(result==ESP_OK&&length==32&&nonzero)return true;
 memset(keyOut,0,32);
#endif
 return false;
}
