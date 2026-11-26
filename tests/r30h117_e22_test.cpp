#include "e22_uart_link.h"
#include "lora_link_policy.h"
#include <cassert>
#include <deque>
#include <cstring>
static uint32_t fakeNow=0;
uint32_t millis(){return fakeNow;}
void delay(unsigned long n){fakeNow+=n;}
uint16_t radio_crc16(const uint8_t*,size_t){return 0;}
using namespace flytec_rtos;
class FakeStream:public Stream{
public:
 std::deque<uint8_t> rx;
 bool shortWrite=false;
 int available() override{return static_cast<int>(rx.size());}
 int read() override{if(rx.empty())return -1;int v=rx.front();rx.pop_front();return v;}
 void flush() override{}
 size_t write(const uint8_t*,size_t n) override{
  if(n==12||n==3){const uint8_t ack[]={0xc1,0,9,0,0,0,0x67,0xc3,0x12,0,0,0};rx.insert(rx.end(),ack,ack+12);}
  return shortWrite?n-1:n;
 }
 void inject(const RadioFrame& f,size_t start=0,size_t end=sizeof(RadioFrame)){
  const auto* p=reinterpret_cast<const uint8_t*>(&f);rx.insert(rx.end(),p+start,p+end);
 }
};
int main(){
 FakeStream uart;
 E22UartLink link(uart,[](bool,bool,void*){},[](void*){return true;},nullptr,nullptr);
 assert(link.configureEU868TestProfile());
 RadioFrame f{};f.magic[0]=RADIO_FRAME_MAGIC0;f.magic[1]=RADIO_FRAME_MAGIC1;
 RadioFrame out{};
 uart.inject(f,0,10);link.update();assert(!link.receiveFrame(out));
 uart.inject(f,10);link.update();assert(link.receiveFrame(out));
 f.crc16=1;uart.inject(f);link.update();assert(!link.receiveFrame(out));f.crc16=0;
 for(unsigned i=1;i<=5;++i){f.commandId=static_cast<uint16_t>(i);uart.inject(f);link.update();}
 assert(uart.available()==32);assert(link.rxFullPollCount()>=2);
 assert(link.receiveFrame(out)&&out.commandId==1);
 link.update();assert(uart.available()==0);
 for(unsigned i=2;i<=5;++i)assert(link.receiveFrame(out)&&out.commandId==i);
 assert(!link.receiveFrame(out));
 // TX admission must never consume pending RX bytes.
 uart.inject(f);
 assert(link.sendFrame(f,fakeNow));
 assert(uart.available()==32);
 assert(!link.receiveFrame(out));
 link.update();
 assert(link.receiveFrame(out));
#ifdef FLYTEC_TEST_BOAT_SHUTDOWN
 link.shutdownLocal();
 assert(!link.configured()&&!link.txPending()&&link.transportFault());
 assert(!link.sendFrame(f,fakeNow));assert(!link.receiveFrame(out));
#endif
 // Separate instance for uncertain write, independent of pending TX state.
 FakeStream faultUart;
 E22UartLink faultLink(faultUart,[](bool,bool,void*){},[](void*){return true;},nullptr,nullptr);
 assert(faultLink.configureEU868TestProfile());
 faultUart.shortWrite=true;
 assert(faultLink.trySendFrame(f,fakeNow)==E22UartLink::Admission::Uncertain);
 assert(faultLink.transportFault()&&!faultLink.configured());

 // A managed admission binds once, routes its write through the private
 // managed seam, and prevents both direct legacy writes and a second owner.
 FakeStream managedUart;
 E22UartLink managedLink(managedUart,[](bool,bool,void*){},[](void*){return true;},nullptr,nullptr);
 LoRaAirtimeProfile profile;profile.commissioned=true;
 LoRaAirtimePolicy managedBudget,secondBudget;
 LoRaQuietRestart managedProof;
 uint32_t proofNow=fakeNow;managedProof.observe(proofNow,true);
 for(unsigned i=0;i<LoRaQuietRestart::QuietMs/LoRaQuietRestart::MaxSampleGap;++i){
  proofNow+=LoRaQuietRestart::MaxSampleGap;managedProof.observe(proofNow,true);
 }
 assert(managedBudget.configureAfterQuiet(profile,proofNow,managedProof));
 assert(secondBudget.configure(profile,fakeNow,true));
 E22AdaptiveAdmission<E22UartLink> managed(managedLink,managedBudget,20,true);
 E22AdaptiveAdmission<E22UartLink> secondOwner(managedLink,secondBudget,20,true);
 assert(managedLink.configureEU868TestProfile());
 fakeNow=proofNow;
 assert(managedLink.trySendFrame(f,fakeNow)==E22UartLink::Admission::Unavailable);
 assert(secondOwner.start(f,LoRaAirtimePolicy::Traffic::Emergency,fakeNow)==E22AdaptiveAdmission<E22UartLink>::Result::Denied);
 assert(secondBudget.usedMs(fakeNow)==0);
 assert(managed.start(f,LoRaAirtimePolicy::Traffic::Emergency,fakeNow)==E22AdaptiveAdmission<E22UartLink>::Result::Accepted);
 assert(!managedLink.configureEU868TestProfile());
 assert(managedBudget.usedMs(fakeNow)==25);
 delay(flytec_rtos::E22TxState::minUartMs);managedLink.update();
 assert(managed.poll(fakeNow)==E22AdaptiveAdmission<E22UartLink>::Poll::Pending);
 assert(!managedLink.configureEU868TestProfile()); // LocalReady is not the RF-tail end
 assert(managed.poll(fakeNow+20)==E22AdaptiveAdmission<E22UartLink>::Poll::LocalComplete);
 assert(managedBudget.usedMs(fakeNow+20)==25);
 assert(managedLink.configureEU868TestProfile());

 // The legacy configure(profile, priorHourQuiet) overload is insufficient
 // to authorize a managed physical driver without a consumed quiet proof.
 FakeStream unprovenUart;
 E22UartLink unprovenLink(unprovenUart,[](bool,bool,void*){},[](void*){return true;},nullptr,nullptr);
 LoRaAirtimePolicy unprovenBudget;
 assert(unprovenBudget.configure(profile,proofNow,true));
 E22AdaptiveAdmission<E22UartLink> unprovenManager(unprovenLink,unprovenBudget,20,true);
 assert(unprovenLink.configureEU868TestProfile());
 assert(unprovenLink.trySendFrame(f,proofNow)==E22UartLink::Admission::Unavailable);
 assert(unprovenManager.start(f,LoRaAirtimePolicy::Traffic::Emergency,proofNow)==
        E22AdaptiveAdmission<E22UartLink>::Result::Denied);
 assert(unprovenBudget.usedMs(proofNow)==0);

 // A driver with legacy history cannot transfer its owner to a fresh managed
 // ledger, which would otherwise forget already charged airtime.
 FakeStream chargedUart;
 E22UartLink chargedLink(chargedUart,[](bool,bool,void*){},[](void*){return true;},nullptr,nullptr);
 assert(chargedLink.configureEU868TestProfile());
 fakeNow=millis();assert(chargedLink.sendFrame(f,fakeNow));
 delay(flytec_rtos::E22TxState::minUartMs);chargedLink.update();
 LoRaAirtimePolicy lateBudget;assert(lateBudget.configure(profile,fakeNow,true));
 E22AdaptiveAdmission<E22UartLink> lateManager(chargedLink,lateBudget,20,true);
 assert(lateManager.start(f,LoRaAirtimePolicy::Traffic::Emergency,fakeNow)==
        E22AdaptiveAdmission<E22UartLink>::Result::Denied);
 assert(lateBudget.usedMs(fakeNow)==0);
}
