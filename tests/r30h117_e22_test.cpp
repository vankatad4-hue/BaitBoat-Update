#include "e22_uart_link.h"
#include <cassert>
#include <deque>
#include <cstring>
static uint32_t now=0;
uint32_t millis(){return now;}
void delay(unsigned long n){now+=n;}
uint16_t radio_crc16(const uint8_t*,size_t){return 0;}
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
 for(unsigned i=1;i<=5;++i){f.commandId=i;uart.inject(f);link.update();}
 assert(uart.available()==32);assert(link.rxFullPollCount()>=2);
 assert(link.receiveFrame(out)&&out.commandId==1);
 link.update();assert(uart.available()==0);
 for(unsigned i=2;i<=5;++i)assert(link.receiveFrame(out)&&out.commandId==i);
 assert(!link.receiveFrame(out));
 uart.shortWrite=true;
 assert(link.trySendFrame(f,now)==E22UartLink::Admission::Uncertain);
 assert(link.transportFault()&&!link.configured());
}
