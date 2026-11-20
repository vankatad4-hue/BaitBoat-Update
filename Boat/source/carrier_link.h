#pragma once
#include <stdint.h>
#include <stddef.h>
// Private wired link: CRC detects corruption, not malicious authentication.
// 16-byte little-endian records; no packed structs or ABI dependencies.
namespace carrier {
constexpr uint8_t ADDRESS=0x32, SIZE=16, MAGIC=0xc7, VERSION=1;
enum Op:uint8_t{SYNC_OFF=1,OUTPUT=2,DISARM=3};
enum State:uint8_t{LOCKED=0,READY=1,ARMED=2,FAULT=3};
enum Flag:uint8_t{STOP_OK=1,WATCHDOG_OK=2,INTERLOCK_OK=4,GATE_ON=8,LEFT_IDLE=16,RIGHT_IDLE=32,COILS_IDLE=64,FEEDBACK_OK=128};
struct Command{Op op;uint32_t seq;int16_t left,right;int8_t coil;};
struct Status{State state;uint32_t echoed;uint16_t sample;uint8_t flags,coilIdleMask,fault;};
inline uint16_t crc(const uint8_t *p,size_t n){uint16_t c=0xffff;while(n--){c^=(uint16_t)*p++<<8;for(unsigned i=0;i<8;++i)c=(uint16_t)((c&0x8000)?(c<<1)^0x1021:c<<1);}return c;}
inline void put16(uint8_t *p,uint16_t n){p[0]=(uint8_t)n;p[1]=(uint8_t)(n>>8);}
inline uint16_t get16(const uint8_t *p){return (uint16_t)p[0]|((uint16_t)p[1]<<8);}
inline void put32(uint8_t *p,uint32_t n){for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(n>>(8*i));}
inline uint32_t get32(const uint8_t *p){uint32_t n=0;for(unsigned i=0;i<4;++i)n|=(uint32_t)p[i]<<(8*i);return n;}
inline bool valid(const Command &c){
 return c.op>=SYNC_OFF&&c.op<=DISARM&&c.seq&&c.left>=-1000&&c.left<=1000&&c.right>=-1000&&c.right<=1000&&c.coil>=-1&&c.coil<=3&&
 !(c.coil>=0&&(c.left||c.right))&&(c.op==OUTPUT||(!c.left&&!c.right&&c.coil==-1));
}
inline bool encode(const Command &c,uint8_t p[SIZE]){
 if(!valid(c))return false;
 p[0]=MAGIC;p[1]=VERSION;p[2]=c.op;p[3]=0;put32(p+4,c.seq);put16(p+8,(uint16_t)c.left);put16(p+10,(uint16_t)c.right);p[12]=(uint8_t)(c.coil+1);p[13]=0;put16(p+14,crc(p,14));return true;
}
inline bool decode(const uint8_t *p,size_t n,Command &c){
 if(!p||n!=SIZE||p[0]!=MAGIC||p[1]!=VERSION||p[3]||p[13]||p[12]>4||get16(p+14)!=crc(p,14))return false;
 Command v={(Op)p[2],get32(p+4),(int16_t)get16(p+8),(int16_t)get16(p+10),(int8_t)(p[12]-1)};
 if(!valid(v))return false;
 c=v;return true;
}
inline void encodeStatus(const Status &s,uint8_t p[SIZE]){
 p[0]=MAGIC;p[1]=VERSION;p[2]=0x80;p[3]=s.state;put32(p+4,s.echoed);put16(p+8,s.sample);p[10]=s.flags;p[11]=s.coilIdleMask;p[12]=s.fault;p[13]=0;put16(p+14,crc(p,14));
}
inline bool decodeStatus(const uint8_t *p,size_t n,Status &s){
 if(!p||n!=SIZE||p[0]!=MAGIC||p[1]!=VERSION||p[2]!=0x80||p[3]>FAULT||p[11]>15||p[13]||get16(p+14)!=crc(p,14))return false;
 s={(State)p[3],get32(p+4),get16(p+8),p[10],p[11],p[12]};return true;
}
}
