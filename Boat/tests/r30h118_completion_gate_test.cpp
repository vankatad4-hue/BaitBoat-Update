#include "r30h118_completion_gate.h"
#include <cassert>
#include "r30h118_radio_handoff.h"
using namespace flytec_r30h118;
int main(){
 CompletionGate g; Ticket a,b,c;
 assert(!g.issue(a));
 assert(g.invalidate(1));
 assert(g.issue(a)); assert(!g.issue(b));
 assert(g.invalidate(1)); // newer STOP, same session
 assert(g.issue(b)); assert(!g.consume(a));
 assert(g.consume(b)); assert(!g.consume(b));
 assert(g.issue(c)); assert(!g.consume(b));
 assert(g.invalidate(2)); assert(!g.consume(c));
 assert(g.issue(a)); assert(g.invalidate(0));
 assert(!g.consume(a)); assert(!g.issue(b));
 assert(g.invalidate(1)); assert(g.issue(b));
 assert(!g.consume(a)); assert(g.consume(b));
 RadioHandoff<uint32_t> lane;
 RadioHandoff<uint32_t>::Request old{},fresh{};
 bool admitted=false;
 assert(lane.invalidate(1));
 assert(lane.prepare(42,old));
 assert(lane.invalidate(1));
 assert(lane.prepare(99,fresh));
 assert(!lane.accept({old.ticket,true},admitted));
 assert(!admitted);
 assert(lane.accept({fresh.ticket,true},admitted));
 assert(admitted);
 assert(lane.prepare(7,fresh));
 assert(lane.cancelUnqueued(fresh));
 assert(!lane.accept({fresh.ticket,true},admitted));
}
