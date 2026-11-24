#pragma once
#include "r30h121_app_radio.h"
namespace flytec_r30h123 {
// Owner-side RX boundary. Capture time is local PHY observation, not sender
// tick or proof of RF age. The existing application must still enforce replay,
// command decoding and actuator policy. No RX callback runs on the PHY core.
template<class Frame> class OwnerRx {
 using Events=flytec_r30h118::PhyEvents<Frame>;
 flytec_r30h121::AppRadio<Frame>& app_;
 const uint8_t source_,destination_,flags_;
public:
 OwnerRx(flytec_r30h121::AppRadio<Frame>& app,uint8_t source,uint8_t destination,bool backup):
  app_(app),source_(source),destination_(destination),flags_(backup?1U:0U){}
 // Re-read session epoch per frame: an earlier handshake can roll the key.
 // Budget counts discarded frames as well as accepted frames.
 template<class Epoch,class RootVerify,class SessionVerify,class Dispatch>
 unsigned drain(uint32_t now,uint32_t age,Epoch epoch,RootVerify rootVerify,SessionVerify sessionVerify,Dispatch dispatch){
  unsigned delivered=0;
  typename Events::Rx rx{};
  for(unsigned i=0;i<4&&app_.receive(rx);++i){
   const auto& f=rx.frame;
   if(!age||uint32_t(now-rx.capturedAt)>=age||f.sourceId!=source_||f.destinationId!=destination_||f.flags!=flags_)continue;
   // Boat receives HELLO and PROOF only; reply types belong to the Remote.
   if(f.type==10U||f.type==12U){
    if(f.sessionEpoch||f.sequence||f.commandId||!rootVerify(f))continue;
   }else{
    const uint8_t active=epoch();
    if(!active||f.sessionEpoch!=active||!f.sequence)continue;
    if(f.type!=1U&&f.type!=2U&&f.type!=4U&&f.type!=8U&&f.type!=14U&&f.type!=16U)continue;
    if(!sessionVerify(f))continue;
   }
   dispatch(f,rx.capturedAt);++delivered;
  }
  return delivered;
 }
};
}
