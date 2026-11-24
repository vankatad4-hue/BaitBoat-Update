#pragma once
#include <stdint.h>
namespace flytec_r30h123 {
// Owner-core-only selection. Claim is irreversible for this object, even
// when initialization fails. Task exit alone does not grant legacy fallback.
enum class RadioOwner:uint8_t { Unclaimed,Legacy,Worker,Retired };
class RuntimeOwner {
 RadioOwner owner_=RadioOwner::Unclaimed;
public:
 RadioOwner mode()const{return owner_;}
 bool claimLegacy(){
  if(owner_!=RadioOwner::Unclaimed)return false;
  owner_=RadioOwner::Legacy;return true;
 }
 bool claimWorker(bool commissioned){
  if(!commissioned||owner_!=RadioOwner::Unclaimed)return false;
  owner_=RadioOwner::Worker;return true;
 }
 void retire(){if(owner_!=RadioOwner::Unclaimed)owner_=RadioOwner::Retired;}
 bool legacy()const{return owner_==RadioOwner::Legacy;}
 bool worker()const{return owner_==RadioOwner::Worker;}
};
}
