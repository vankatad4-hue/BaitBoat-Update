#include <cassert>
#include <cstring>
#include <iostream>
#include "../source/ekm014_logical_interface_r16.h"

using namespace flytec_ekm014_r16;
int main() {
    const Channel expected[] = {LH_BAIT,RH_BAIT,LK_HOOK,RK_HOOK};
    const char* pins[] = {"IN1","IN2","IN3","IN4"};
    const char* outs[] = {"CH1","CH2","CH3","CH4"};
    for(int i=0;i<4;++i) {
        assert(static_cast<int>(expected[i])==i);
        assert(std::strcmp(inputName(expected[i]),pins[i])==0);
        assert(std::strcmp(outputName(expected[i]),outs[i])==0);
        assert(logicalMask(expected[i])==(1u<<i));
        assert(isSingleChannelMask(logicalMask(expected[i])));
    }
    for(int i=-128;i<=127;++i) {
        const Channel candidate=static_cast<Channel>(i);
        if(i<0||i>3) assert(logicalMask(candidate)==0);
    }
    for(int mask=0;mask<=255;++mask) {
        const bool expectedSingle=mask==1||mask==2||mask==4||mask==8;
        assert(isSingleChannelMask(static_cast<uint8_t>(mask))==expectedSingle);
    }
    assert(!isSingleChannelMask(0));
    assert(!isSingleChannelMask(3));
    assert(!isSingleChannelMask(15));
    assert(!proposedSupplyWithinManufacturerBounds(12.6f,12.6f)); // VL max 12V!
    assert(proposedSupplyWithinManufacturerBounds(12.6f,5.0f));
    assert(proposedSupplyWithinManufacturerBounds(9.0f,5.0f));
    assert(!proposedSupplyWithinManufacturerBounds(4.0f,5.0f));
    assert(!proposedSupplyWithinManufacturerBounds(12.6f,3.3f));
    assert(!proposedSupplyWithinManufacturerBounds(4.5f,5.0f));
    assert(!proposedSupplyWithinManufacturerBounds(12.6f,13.0f));
    assert(!proposedSupplyWithinManufacturerBounds(51.0f,5.0f));
    assert(!proposedSupplyWithinManufacturerBounds(12.6f,0.0f));
    assert(proposedSupplyWithinManufacturerBounds(12.6f,12.0f)); // bound only, NOT advised
    std::cout << "R16 EKM014 logical channel & supply-envelope tests PASS; NO GPIO or actuator power enabled\n";
}
