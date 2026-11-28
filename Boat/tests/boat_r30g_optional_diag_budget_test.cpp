#include "../source/boat_optional_diagnostic_budget.h"
#include <cassert>
#include <cstring>
#include <iostream>
using namespace boat_r30g;

static Opportunity idle(uint32_t heartbeat){
 Opportunity s{};
 s.authenticatedSession=true;
 s.primaryFresh=true;
 s.haveHeartbeat=true;
 s.lastHeartbeatAtMs=heartbeat;
 return s;
}
static void skipped(OptionalDiagnosticBudget &b,Opportunity s,uint32_t now) {
 uint8_t out[8]{};
 assert(!b.take(s,now,out));
}
int main() {
 // A sonar profile may use a heartbeat gap during idle operation, but STOP
 // and all other radio-owned work suppress fragments until the slot is free.
 auto sonarSlot=idle(1000);
 assert(sonarProfileSlotAllowed(sonarSlot,20U));
 sonarSlot.commandQueued=true; // Includes pending STOP ACK/STATUS work.
 assert(!sonarProfileSlotAllowed(sonarSlot,20U));
 sonarSlot.commandQueued=false;sonarSlot.txPending=true;
 assert(!sonarProfileSlotAllowed(sonarSlot,20U));
 sonarSlot.txPending=false;sonarSlot.activeMission=true;
 assert(!sonarProfileSlotAllowed(sonarSlot,20U));
 sonarSlot.activeMission=false;sonarSlot.failsafeOrLinkLoss=true;
 assert(!sonarProfileSlotAllowed(sonarSlot,20U));
 sonarSlot.failsafeOrLinkLoss=false;sonarSlot.recentCommand=true;
 assert(!sonarProfileSlotAllowed(sonarSlot,20U));
 sonarSlot.recentCommand=false;
 assert(!sonarProfileSlotAllowed(sonarSlot,14U));
 assert(!sonarProfileSlotAllowed(sonarSlot,41U));

 const uint8_t oldPacket[8]={8,1,1,2,10,255,255,7};
 const uint8_t newPacket[8]={8,1,1,2,10,255,255,8};
 uint8_t payload[8]{};
 OptionalDiagnosticBudget b;
 assert(!b.pending());
 assert(!b.offer(nullptr,1000));
 const uint8_t wrong[8]={7,1,0,0,0,0,0,0};
 assert(!b.offer(wrong,1000));
 assert(b.offer(oldPacket,1000)&&b.pending());
 auto s=idle(1000);
 // A heartbeat and RX safety service must run before optional TX.
 s.haveHeartbeat=false;skipped(b,s,1020);s.haveHeartbeat=true;
 skipped(b,s,1001); // too soon, heartbeat TX likely still pending
 s.txPending=true;skipped(b,s,1020);s.txPending=false;
 s.commandQueued=true;skipped(b,s,1020);s.commandQueued=false;
 s.activeMission=true;skipped(b,s,1020);s.activeMission=false;
 s.recentCommand=true;skipped(b,s,1020);s.recentCommand=false;
 // Link loss is destructive (tested separately below); do not use it
 // while checking the same record remains pending across busy TX windows.
 skipped(b,s,1041); // heartbeat due soon; no preemption
 assert(b.pending());
 assert(b.offer(newPacket,1010)); // replaces older record, bounded size 1
 assert(b.replaced()==1);
 assert(b.take(s,1020,payload)&&!b.pending());
 assert(std::memcmp(payload,newPacket,8)==0);
 b.sent(1020);
 assert(b.transmitted()==1);
 // Packet is one-shot: never sent again without a NEW offer.
 assert(!b.take(s,1021,payload));
 // 2-second airtime cap, even for repeated offers.
 assert(b.offer(oldPacket,2000));
 s.lastHeartbeatAtMs=2000;skipped(b,s,2020);
 assert(b.pending()); // rate-gated but not transmitted
 skipped(b,s,2501); // old record expired, never replay it
 assert(!b.pending()&&b.discarded()==1);
 assert(b.offer(oldPacket,3000));s.lastHeartbeatAtMs=3000;
 assert(b.take(s,3020,payload)); b.sent(3020);
 assert(b.transmitted()==2);
 assert(b.offer(oldPacket,4000));s.lastHeartbeatAtMs=4000;
 skipped(b,s,4020); // already transmitted recently
 b.discard();assert(!b.pending());
 b.reset();assert(!b.pending());
 // A failed authentication or lost primary discards rather than queues.
 assert(b.offer(oldPacket,5000));
 s=idle(5000);s.authenticatedSession=false;
 skipped(b,s,5020);assert(!b.pending());
 assert(b.offer(oldPacket,5100));
 s=idle(5100);s.primaryFresh=false;
 skipped(b,s,5120);assert(!b.pending());
 assert(b.offer(oldPacket,5200));
 s=idle(5200);s.failsafeOrLinkLoss=true;
 skipped(b,s,5220);assert(!b.pending());
 // Expiration is explicit: old diagnostic never crosses into a new mission.
 b.reset();
 assert(b.offer(oldPacket,6000));
 s=idle(6500); // still <= 500ms
 skipped(b,s,6500); // outside heartbeat window, record can still expire
 s=idle(6501);skipped(b,s,6521);
 assert(!b.pending());
 // A rejected underlying TX discards (no hidden retry loop).
 b.reset(); assert(b.offer(newPacket,7000));s=idle(7000);
 assert(b.take(s,7020,payload)); b.rejectedTx();
 assert(!b.pending()&&b.discarded()>=1);
 // Wrap-aware heartbeat and offer expiry.
 b.reset();assert(b.offer(oldPacket,0xfffffff0U));
 s=idle(0xfffffff2U);
 assert(b.take(s,0x00000006U,payload)); // heartbeat age 20ms
 b.sent(0x00000006U);
 assert(b.offer(oldPacket,0x00000010U));
 s=idle(0x00000012U);skipped(b,s,0x00000026U); // cooldown
 // Simulation: sustained control traffic and mission activity produce ZERO
 // optional TX; urgent heartbeat / ACK remain on the priority path.
 b.reset();
 unsigned attempted=0;
 for(uint32_t now=10000;now<13000;now+=10U){
  if(now%200U==0U) assert(b.offer(oldPacket,now));
  s=idle(now-now%100U);
  s.recentCommand=true; // manual motor updates more often than 250ms
  if(b.take(s,now,payload)) ++attempted;
 }
 assert(attempted==0);
 b.reset();
 // Once idle and link authenticated, first idle heartbeat gap may send one.
 assert(b.offer(newPacket,14000));
 s=idle(14000);
 assert(b.take(s,14020,payload));
 std::cout<<"R30G one-slot drop/expiry heartbeat-first and manual/mission priority PASS\n";
}
