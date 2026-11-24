#pragma once
#include <stdint.h>
// Owner-local only. Transfer immutable tickets through synchronized queues.
// This gate never authorizes motion or proves RF delivery.
namespace flytec_r30h118 {
enum class TicketDomain:uint8_t { Application, Handshake };
struct Ticket { uint32_t generation=0; uint32_t serial=0; uint8_t epoch=0; TicketDomain domain=TicketDomain::Application; };
class CompletionGate {
 const TicketDomain domain_;
 uint32_t generation_=0, serial_=0;
 uint8_t epoch_=0;
 bool pending_=false, exhausted_=false;
public:
 explicit CompletionGate(TicketDomain domain=TicketDomain::Application):domain_(domain){}
 bool invalidate(uint8_t epoch){
  pending_=false; epoch_=0;
  if(exhausted_||generation_==UINT32_MAX){exhausted_=true;return false;}
  ++generation_; serial_=0; epoch_=epoch; return epoch!=0;
 }
 uint32_t activeGeneration() const {return epoch_&&!exhausted_?generation_:0;}
 bool issue(Ticket &out){
  if(exhausted_||!epoch_||pending_||serial_==UINT32_MAX)return false;
  ++serial_; pending_=true; out={generation_,serial_,epoch_,domain_};return true;
 }
 bool consume(const Ticket &t){
  if(!pending_||!epoch_||t.domain!=domain_||t.epoch!=epoch_||t.generation!=generation_||t.serial!=serial_)return false;
  pending_=false;return true;
 }
};
}
