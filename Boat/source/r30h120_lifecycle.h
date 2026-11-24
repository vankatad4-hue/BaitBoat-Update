#pragma once
#include <atomic>
#include <stdint.h>
namespace flytec_r30h120 {
enum class Phase:uint8_t {Idle,Starting,Running,Stopping,Stopped,Failed};
// Owner requests start/stop; worker alone initializes and quiesces drivers.
// Failed/Stopped are terminal. No fallback driver access before worker join.
class Lifecycle {
 std::atomic<Phase> phase_{Phase::Idle};
public:
 Phase phase()const{return phase_.load(std::memory_order_acquire);}
 bool start(){Phase p=Phase::Idle;return phase_.compare_exchange_strong(p,Phase::Starting);}
 bool ready(){Phase p=Phase::Starting;return phase_.compare_exchange_strong(p,Phase::Running);}
 void stop(){
  Phase p=phase();
  while(p==Phase::Starting||p==Phase::Running){
   if(phase_.compare_exchange_weak(p,Phase::Stopping))return;
  }
 }
 bool enabled()const{return phase()==Phase::Running;}
 // Only worker after quiescence, or owner if task creation failed.
 bool finish(bool quiesced){
  Phase p=phase();
  while(p==Phase::Starting||p==Phase::Stopping||p==Phase::Running){
   if(phase_.compare_exchange_weak(p,quiesced&&p==Phase::Stopping?Phase::Stopped:Phase::Failed))return true;
  }
  return false;
 }
};
}
