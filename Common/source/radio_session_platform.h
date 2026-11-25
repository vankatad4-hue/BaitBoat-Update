#pragma once
#include "radio_handshake.h"
#include <mbedtls/md.h>
#include <esp_system.h>
// Provisioning remains disabled until entropy availability is verified on the
// exact target. esp_fill_random alone is not proof of RF-off true entropy.
inline uint64_t radioPlatformNonce(){uint64_t n=0;esp_fill_random(&n,sizeof(n));return n;}
inline bool radioPlatformHmac(const uint8_t key[32],const uint8_t *data,size_t n,uint8_t out[32]){
 const auto *info=mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
 return info&&mbedtls_md_hmac(info,key,32,data,n,out)==0;
}
