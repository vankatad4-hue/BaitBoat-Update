#pragma once
#include <stdint.h>

// R30F OPTIONAL authenticated TELEMETRY subtype 8; never a command or
// RADIO_PAYLOAD_STATUS. The surrounding existing 32-byte RadioFrame supplies
// HMAC/session/replay validation. This eight-byte field contains NO keys,
// command IDs, STOP acknowledgements or physical commissioning permission.
//
//   byte 0  subtype (8)
//   byte 1  sub-version (1)
//   byte 2  diagnostic fault reason (1..14; 14=UNCOMMISSIONED)
//   byte 3  diagnostic flags bits 7..0
//   byte 4  diagnostic flags bits 11..8 (upper four bits MUST be zero)
//   bytes5-6  voltage mV LE, or 0xffff when not observed/fresh
//   byte 7  diagnostic sequence counter
//
// This is a separate wire type from the R30E LOCAL D5 serial sidecar.
// Do not interpret "observed" as "electrically commissioned".
namespace radio_r30f {
static constexpr uint8_t SUBTYPE = 8;
static constexpr uint8_t VERSION = 1;
static constexpr uint8_t MAX_REASON = 14;
static constexpr uint16_t FLAGS_OBSERVED = 0x0001U;
static constexpr uint16_t FLAGS_UNVERIFIED = 0x0002U;
static constexpr uint16_t FLAGS_COMMISSIONING_LOCK = 0x0800U;
static constexpr uint16_t FLAGS_DEFINED_MASK = 0x0FFFU;
struct Diagnostic {
 uint8_t reason = 0;
 uint16_t flags = 0;
 uint16_t millivolts = 0xFFFFU;
 uint8_t sequence = 0;
};
inline bool validate(const Diagnostic &d) {
 if(d.reason < 1U || d.reason > MAX_REASON ||
    (d.flags & ~FLAGS_DEFINED_MASK) != 0U ||
    (d.flags & (FLAGS_UNVERIFIED | FLAGS_COMMISSIONING_LOCK)) !=
         (FLAGS_UNVERIFIED | FLAGS_COMMISSIONING_LOCK)) return false;
 const bool observed = (d.flags & FLAGS_OBSERVED) != 0U;
 if(observed != (d.reason >= 8U))return false;
 if(observed) {
  if(d.millivolts < 9000U || d.millivolts > 13000U)return false;
 }else if(d.millivolts != 0xFFFFU) return false;
 return true;
}
inline bool encode(const Diagnostic &d, uint8_t out[8]) {
 if(!out)return false;
 for(unsigned i=0;i<8;++i)out[i]=0;
 if(!validate(d))return false;
 out[0]=SUBTYPE;out[1]=VERSION;out[2]=d.reason;
 out[3]=static_cast<uint8_t>(d.flags);
 out[4]=static_cast<uint8_t>(d.flags>>8);
 out[5]=static_cast<uint8_t>(d.millivolts);
 out[6]=static_cast<uint8_t>(d.millivolts>>8);
 out[7]=d.sequence;
 return true;
}
inline bool decode(const uint8_t in[8],Diagnostic &out){
 out={};
 if(!in||in[0]!=SUBTYPE||in[1]!=VERSION)return false;
 Diagnostic d{};
 d.reason=in[2];d.flags=static_cast<uint16_t>(
      static_cast<uint16_t>(in[3]) | (static_cast<uint16_t>(in[4])<<8));
 d.millivolts=static_cast<uint16_t>(
      static_cast<uint16_t>(in[5]) | (static_cast<uint16_t>(in[6])<<8));
 d.sequence=in[7];
 if(!validate(d))return false;
 out=d;return true;
}
} // namespace radio_r30f
