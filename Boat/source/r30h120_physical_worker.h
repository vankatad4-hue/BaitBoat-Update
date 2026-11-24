#pragma once
#include "nrf24_link.h"
#include "e22_uart_board_port.h"
#include "r30h118_phy_adapters.h"
#include "r30h120_phy_service.h"
namespace flytec_r30h120 {
// Construct and call only inside the physical worker task.
// No task launch or hardware enable is implied by this class.
class PhysicalWorker {
 using Bus=flytec_r30h118::Transport<RadioFrame>;
 using Events=flytec_r30h118::PhyEvents<RadioFrame>;
 NRF24Link nrf_;
 HardwareSerial serial_{1};
 E22UartBoardPort e22_{serial_};
 flytec_r30h118::NrfAdapter<RadioFrame,NRF24Link> na_;
 flytec_r30h118::E22Adapter<RadioFrame,E22UartBoardPort> ea_;
 PhyService<RadioFrame> ns_,es_;
 bool nrfInitialized_=false,e22Initialized_=false,started_=false,closed_=false;
public:
 PhysicalWorker(int ce,int csn,SPIClass *spi,uint8_t channel,int sck,int miso,int mosi,
                Bus &nb,Bus &eb,Events &ne,Events &ee):
 nrf_(ce,csn,spi,channel,sck,miso,mosi),na_(nrf_,ne),ea_(e22_,ee),ns_(nb),es_(eb){}
 PhysicalWorker(const PhysicalWorker&)=delete;
 PhysicalWorker& operator=(const PhysicalWorker&)=delete;
 // Caller must enforce existing board pin/RF commissioning gates first.
 bool begin(bool authorized,bool primary,const E22BoardPins &pins,bool backup){
  if(!authorized||started_||closed_)return false;
  started_=true;
  if(primary)nrfInitialized_=nrf_.begin(false);
  if(backup)e22Initialized_=e22_.begin(pins);
  return nrfInitialized_||e22Initialized_;
 }
 void step(uint32_t now,uint32_t age,bool enabled){
  if(!started_||closed_)return;
  if(nrfInitialized_)na_.poll(now);
  if(e22Initialized_)ea_.poll(now);
  ns_.step(now,age,enabled&&nrfInitialized_,[&](const Bus::Request &r){return na_.admit(r.ticket,r.frame);});
  es_.step(now,age,enabled&&e22Initialized_,[&](const Bus::Request &r){return ea_.admit(r.ticket,r.frame,now);});
 }
 // Software closure only. Caller invalidates tickets before shutdown.
 // No claim of RF quiescence or completed application ACK.
 void shutdown(){
  if(closed_)return;
  closed_=true;
  if(nrfInitialized_)nrf_.shutdownInitialized();
  if(e22Initialized_)e22_.shutdownLocal();
 }
};
}
