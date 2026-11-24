#pragma once
#include <stdint.h>
// Owner-only, allocation-free statistics. Read after the measuring task joins,
// or from that same task. Never treat wall-clock busy time as CPU utilization.
namespace flytec_rtos {
struct LatencyStats {
 uint32_t count=0,max=0;
 uint64_t total=0;
 void observe(uint32_t value){
  if(value>max)max=value;
  if(count!=UINT32_MAX){++count;total+=value;}
 }
 uint32_t mean()const{return count?static_cast<uint32_t>(total/count):0;}
};
struct PollTiming {
 LatencyStats duration,gap;
 bool seen=false;
 uint32_t last=0;
 void begin(uint32_t now){
  if(seen)gap.observe(now-last);
  last=now;seen=true;
 }
 void end(uint32_t now){if(seen)duration.observe(now-last);}
};
// Clock samples use modulo-2^32 subtraction. Each measured interval must
// be shorter than one clock wrap (about 71 minutes for a microsecond clock).
template<class Poller> class TimedPoller {
public:
 using Clock=uint32_t(*)();
 TimedPoller(Poller& p,Clock clock):poller_(p),clock_(clock){}
 void tick(uint32_t now){
  timing_.begin(clock_());poller_.tick(now);timing_.end(clock_());
 }
 const PollTiming& timing()const{return timing_;}
private:
 Poller& poller_;Clock clock_;PollTiming timing_;
};
}
