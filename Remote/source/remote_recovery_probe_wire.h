#pragma once
#include <stdint.h>
namespace flytec_rtos {
// Experimental mock-only LINK_PROBE payload, never an action or STATUS.
// Distinct tag/version rejects any legacy or generic probe shape.
enum class RecoveryProbeKind:uint8_t{Challenge=1,Echo=2};
inline bool recoveryProbeEncode(RecoveryProbeKind kind,uint16_t sequence,uint8_t out[8]){
 if(!out||!sequence||(kind!=RecoveryProbeKind::Challenge&&kind!=RecoveryProbeKind::Echo))return false;
 out[0]=0x52;out[1]=0x43;out[2]=1;out[3]=uint8_t(kind);
 out[4]=uint8_t(sequence);out[5]=uint8_t(sequence>>8);out[6]=out[7]=0;
 return true;
}
inline bool recoveryProbeDecode(const uint8_t in[8],RecoveryProbeKind kind,uint16_t& sequence){
 sequence=0;
 if(!in||(kind!=RecoveryProbeKind::Challenge&&kind!=RecoveryProbeKind::Echo)||
    in[0]!=0x52||in[1]!=0x43||in[2]!=1||in[3]!=uint8_t(kind)||in[6]||in[7])return false;
 sequence=uint16_t(in[4])|(uint16_t(in[5])<<8);return sequence!=0;
}
}
