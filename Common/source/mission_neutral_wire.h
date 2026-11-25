#pragma once
#ifndef FLYTEC_MISSION_NEUTRAL_WIRE_H
#define FLYTEC_MISSION_NEUTRAL_WIRE_H
#include <stdint.h>
// Experimental receipt: transaction LE16, stick sequence LE16, 1, zero[3].
// Sender must have accepted the exact neutral sample into its local lease.
inline bool missionNeutralEncode(uint16_t transaction,uint16_t sequence,uint8_t p[8]){
 if(!p||!transaction||!sequence)return false;
 p[0]=uint8_t(transaction);p[1]=uint8_t(transaction>>8);
 p[2]=uint8_t(sequence);p[3]=uint8_t(sequence>>8);p[4]=1;p[5]=p[6]=p[7]=0;return true;
}
inline bool missionNeutralDecode(const uint8_t p[8],uint16_t& transaction,uint16_t& sequence){
 if(!p||p[4]!=1||p[5]||p[6]||p[7])return false;
 const uint16_t id=uint16_t(p[0])|(uint16_t(p[1])<<8),seq=uint16_t(p[2])|(uint16_t(p[3])<<8);
 if(!id||!seq)return false;
 transaction=id;sequence=seq;return true;
}
#endif
