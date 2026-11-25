#pragma once
#include "mission_control_wire.h"
struct MissionStickInput{uint16_t transaction;int16_t left,right;bool neutral;};
inline bool missionStickValid(const MissionStickInput &s){return s.transaction&&s.left>=-1000&&s.left<=1000&&s.right>=-1000&&s.right<=1000&&(!s.neutral||(!s.left&&!s.right));}
inline bool missionStickEncode(const MissionStickInput &s,uint8_t p[8]){
 if(!p||!missionStickValid(s))return false;
 mcPut16(p,s.transaction);mcPut16(p+2,(uint16_t)s.left);mcPut16(p+4,(uint16_t)s.right);p[6]=s.neutral?1:0;p[7]=0;return true;
}
inline bool missionStickDecode(const uint8_t p[8],MissionStickInput &s){
 if(!p||p[6]>1||p[7])return false;
 MissionStickInput in={mcGet16(p),(int16_t)mcGet16(p+2),(int16_t)mcGet16(p+4),p[6]==1};
 if(!missionStickValid(in))return false;
 s=in;return true;
}
// Local receive lease; only authenticated, replay-checked primary input accepted.
class MissionStickLease{
public:
 void reset(){valid_=false;sample_={};received_=0;}
 bool receive(const MissionStickInput &s,uint16_t transaction,uint32_t now,bool authenticatedPrimary){
  if(!authenticatedPrimary||s.transaction!=transaction||!missionStickValid(s))return false;
  sample_=s;received_=now;valid_=true;return true;
 }
 bool fresh(uint16_t transaction,uint32_t now)const{return valid_&&sample_.transaction==transaction&&now-received_<200U;}
 bool neutral(uint16_t transaction,uint32_t now)const{return fresh(transaction,now)&&sample_.neutral;}
 bool output(uint16_t transaction,uint32_t now,bool permitted,int16_t &left,int16_t &right)const{
  left=right=0;if(!permitted||!fresh(transaction,now))return false;
  left=sample_.left;right=sample_.right;return true;
 }
private:MissionStickInput sample_{};uint32_t received_=0;bool valid_=false;
};
