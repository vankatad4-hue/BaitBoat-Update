#pragma once
#include "r30h118_phy_events.h"
namespace flytec_r30h118 {
// One per link. Register exact submitted ticket; invalidate on STOP/session change.
class OutcomeGate {
 Ticket expected_{};
 bool pending_=false;
public:
 void reset(){pending_=false;}
 bool expect(const Ticket &t){
  if(pending_||!t.generation||!t.serial||!t.epoch)return false;
  expected_=t;pending_=true;return true;
 }
 bool accept(const Ticket &t,uint32_t activeGeneration){
  if(!pending_||!activeGeneration||t.generation!=activeGeneration||!sameTicket(t,expected_))return false;
  pending_=false;return true;
 }
};
}
