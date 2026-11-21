#include <cassert>
#include "../source/boat_endpoint_memory_gate.h"
int main(){
 assert(boatEndpointMemoryMatches(32UL*1024UL*1024UL,16UL*1024UL*1024UL));
 assert(!boatEndpointMemoryMatches(16UL*1024UL*1024UL,16UL*1024UL*1024UL));
 assert(!boatEndpointMemoryMatches(32UL*1024UL*1024UL,8UL*1024UL*1024UL));
 assert(!boatEndpointMemoryMatches(32UL*1024UL*1024UL,0));
 assert(!boatEndpointMemoryMatches(0,0));
}
