#pragma once
// HOST TEST ONLY. Model exactly the mbedTLS SHA-256/HMAC calls used by
// radio_protocol.cpp with OpenSSL; the target builds keep real mbedTLS.
#include <openssl/hmac.h>
#include <stdint.h>
#include <stddef.h>
struct mbedtls_md_info_t {};
enum { MBEDTLS_MD_SHA256 = 6 };
inline const mbedtls_md_info_t *mbedtls_md_info_from_type(int type){
    static const mbedtls_md_info_t sha256{};
    return type==MBEDTLS_MD_SHA256?&sha256:nullptr;
}
inline int mbedtls_md_hmac(const mbedtls_md_info_t *info,const unsigned char *key,
                          size_t keyLen,const unsigned char *data,size_t length,
                          unsigned char *digest){
    if(!info||!key||!digest)return -1;
    unsigned outLen=0;
    return HMAC(EVP_sha256(),key,(int)keyLen,data,length,digest,&outLen)&&outLen==32U?0:-1;
}
