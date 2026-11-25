#include "mission_protocol.h"
#include <string.h>
static constexpr uint32_t PREPARE_TIMEOUT=10000U;
static uint16_t get16(const uint8_t*p){return (uint16_t)p[0]|((uint16_t)p[1]<<8);}
static uint32_t get32(const uint8_t*p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void put16(uint8_t*p,uint16_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
static void put32(uint8_t*p,uint32_t v){for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(v>>(8*i));}
static bool coordinate(int32_t v,bool lat){return lat?(v>=-900000000&&v<=900000000):(v>=-1800000000&&v<=1800000000);}
bool missionPlan_valid(const MissionPlan&p){
 return p.transaction!=0&&p.lake<8&&p.waypoint<16&&!(p.releaseMask&0xF0)&&p.radiusCm>=300&&p.radiusCm<=1000&&
 coordinate(p.targetLatE7,true)&&coordinate(p.targetLonE7,false)&&coordinate(p.homeLatE7,true)&&coordinate(p.homeLonE7,false);
}
static void beginPayload(const MissionPlan&p,uint8_t out[8]){
 out[0]=MISSION_BEGIN;put16(out+1,p.transaction);out[3]=p.lake;out[4]=p.waypoint;out[5]=p.releaseMask;put16(out+6,p.radiusCm);
}
static int32_t component(const MissionPlan&p,uint8_t op){
 switch(op){case MISSION_TARGET_LAT:return p.targetLatE7;case MISSION_TARGET_LON:return p.targetLonE7;case MISSION_HOME_LAT:return p.homeLatE7;default:return p.homeLonE7;}
}
uint32_t missionPlan_digest(const MissionPlan&p){
 uint8_t bytes[24]={};beginPayload(p,bytes);
 for(uint8_t op=MISSION_TARGET_LAT;op<=MISSION_HOME_LON;++op)put32(bytes+8+4*(op-MISSION_TARGET_LAT),(uint32_t)component(p,op));
 uint32_t crc=0xFFFFFFFFU;for(uint8_t b:bytes){crc^=b;for(int i=0;i<8;++i)crc=(crc&1)?(crc>>1)^0xEDB88320U:crc>>1;}return ~crc;
}
bool missionPlan_encode(const MissionPlan&p,MissionOpcode op,uint8_t out[8]){
 if(!out||!missionPlan_valid(p)||op<MISSION_BEGIN||op>MISSION_CANCEL)return false;
 memset(out,0,8);out[0]=(uint8_t)op;put16(out+1,p.transaction);
 if(op==MISSION_BEGIN)beginPayload(p,out);
 else if(op>=MISSION_TARGET_LAT&&op<=MISSION_HOME_LON)put32(out+4,(uint32_t)component(p,op));
 else if(op==MISSION_COMMIT)put32(out+4,missionPlan_digest(p));
 return true;
}
void MissionAssembler::reset(){plan_={};mask_=0;startedAt_=0;seen_=prepared_=cancelled_=false;}
MissionReceive MissionAssembler::ingest(const uint8_t p[8],uint32_t now,bool authenticated,bool stopped){
 if(!p||!authenticated)return MISSION_REJECTED;
 uint8_t op=p[0];uint16_t id=get16(p+1);
 if(!id||op<MISSION_BEGIN||op>MISSION_CANCEL)return MISSION_REJECTED;
 if(op==MISSION_CANCEL){
  if(!seen_||id!=plan_.transaction||p[3]||get32(p+4))return MISSION_REJECTED;
  if(cancelled_)return MISSION_DUPLICATE;
  prepared_=false;cancelled_=true;return MISSION_CANCELLED;
 }
 if(!stopped)return MISSION_REJECTED;
 if(op==MISSION_BEGIN){
  MissionPlan next={};next.transaction=id;next.lake=p[3];next.waypoint=p[4];next.releaseMask=p[5];next.radiusCm=get16(p+6);
  if(!missionPlan_valid(next))return MISSION_REJECTED;
  if(seen_){
   uint16_t delta=(uint16_t)(id-plan_.transaction);
   if(!delta){if(cancelled_||now-startedAt_>=PREPARE_TIMEOUT)return MISSION_REJECTED;return next.lake==plan_.lake&&next.waypoint==plan_.waypoint&&next.releaseMask==plan_.releaseMask&&next.radiusCm==plan_.radiusCm?MISSION_DUPLICATE:MISSION_REJECTED;}
   if(delta>=0x8000U)return MISSION_REJECTED;
  }
  plan_=next;mask_=0;seen_=true;prepared_=cancelled_=false;startedAt_=now;return MISSION_PARTIAL;
 }
 if(!seen_||cancelled_||id!=plan_.transaction||now-startedAt_>=PREPARE_TIMEOUT||p[3])return MISSION_REJECTED;
 if(op==MISSION_COMMIT){
  if(mask_!=15||!missionPlan_valid(plan_)||get32(p+4)!=missionPlan_digest(plan_))return MISSION_REJECTED;
  if(prepared_)return MISSION_DUPLICATE;
  prepared_=true;return MISSION_PREPARED;
 }
 int32_t v=(int32_t)get32(p+4);bool lat=op==MISSION_TARGET_LAT||op==MISSION_HOME_LAT;
 if(!coordinate(v,lat))return MISSION_REJECTED;
 uint8_t bit=(uint8_t)(1U<<(op-MISSION_TARGET_LAT));
 if(mask_&bit)return component(plan_,op)==v?MISSION_DUPLICATE:MISSION_REJECTED;
 if(prepared_)return MISSION_REJECTED;
 switch(op){case MISSION_TARGET_LAT:plan_.targetLatE7=v;break;case MISSION_TARGET_LON:plan_.targetLonE7=v;break;case MISSION_HOME_LAT:plan_.homeLatE7=v;break;case MISSION_HOME_LON:plan_.homeLonE7=v;break;default:return MISSION_REJECTED;}
 mask_|=bit;return MISSION_PARTIAL;
}
bool MissionAssembler::getPrepared(MissionPlan &out,uint32_t now)const{
 if(!prepared_||cancelled_||now-startedAt_>=PREPARE_TIMEOUT)return false;
 out=plan_;return true;
}
