#pragma once
#include "radio_protocol.h"
#include <string.h>
// Owner-only root reply retention. This is local admission, not peer receipt.
// A repeat never extends the lifetime of the original pending reply.
class BoatHandshakeReply {
public:
 void reset(){pending_=false;attempted_=false;}
 bool pending()const{return pending_;}
 void expire(uint32_t now,uint32_t ttl){
  if(pending_&&(!ttl||uint32_t(now-created_)>=ttl))reset();
 }
 bool offer(const RadioFrame& f,uint32_t now,bool backup){
  if((f.type!=RADIO_MSG_SESSION_CHALLENGE&&f.type!=RADIO_MSG_SESSION_CONFIRM)||
     f.sourceId!=2||f.destinationId!=1||f.sequence||f.commandId||f.sessionEpoch||
     !radioLinkFlagMatches(f.flags,backup))return false;
  if(pending_){
   if(frame_.type==f.type&&memcmp(frame_.payload,f.payload,8)==0)return true;
   // A duplicate old-session CONFIRM must not erase a candidate CHALLENGE.
   // New committed proof explicitly clears both lanes before offering CONFIRM.
   return false;
  }
  frame_=f;created_=now;pending_=true;attempted_=false;return true;
 }
 template<class Admit> bool service(uint32_t now,uint32_t ttl,uint32_t gap,
                                    bool ready,bool safetyPending,Admit admit){
  expire(now,ttl);
  if(!pending_||!ready||safetyPending||(attempted_&&uint32_t(now-lastAttempt_)<gap))return false;
  attempted_=true;lastAttempt_=now;
  if(!admit(frame_))return false;
  reset();return true;
 }
private:
 RadioFrame frame_{};
 uint32_t created_=0,lastAttempt_=0;
 bool pending_=false,attempted_=false;
};
