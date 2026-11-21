#pragma once
#include <stdint.h>
// HGLRC M100 PRO: UART NMEA candidate. Exact purchased SKU and electrical
// interface MUST be confirmed before calling begin() on any physical UART.
struct HglrcM100ProProfile {
 int8_t rx,tx;
 uint32_t baud;
 bool exactSkuVerified,voltageVerified,pinAuditSigned,nmeaObserved;
};
inline bool hglrcM100ProProfileValid(const HglrcM100ProProfile &p){
 return p.exactSkuVerified&&p.voltageVerified&&p.pinAuditSigned&&p.nmeaObserved&&
   p.rx>=0&&p.rx<=48&&p.tx>=0&&p.tx<=48&&p.rx!=p.tx&&
   (p.baud==9600||p.baud==115200);
}
inline HglrcM100ProProfile hglrcM100ProUnassigned(){
 return {-1,-1,0,false,false,false,false};
}
