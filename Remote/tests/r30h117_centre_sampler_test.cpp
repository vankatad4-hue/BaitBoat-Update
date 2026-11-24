#include "../source/centre_sampler.h"
#include <cassert>
int main(){
    CentreSampler s;
    int a[4]={2048,2000,2100,1900};
    s.start(0xfffffff0U);
    assert(s.sample(0xfffffff0U,a,true)==CentreSampler::Sampling);
    assert(s.sample(0xfffffff1U,a,true)==CentreSampler::Waiting);
    for(unsigned i=1;i<32;++i)
        assert(s.sample(0xfffffff0U+i*4U,a,true)==(i==31?CentreSampler::Complete:CentreSampler::Sampling));
    assert(s.count()==32 && s.centre(0)==2048 && s.centre(3)==1900);
    s.start(0);
    assert(s.sample(0,a,true)==CentreSampler::Sampling);
    a[0]+=101;
    assert(s.sample(4,a,true)==CentreSampler::Failed);
    s.start(0); a[0]=-1;
    assert(s.sample(0,a,true)==CentreSampler::Failed);
    a[0]=4096;
    assert(s.sample(0,a,true)==CentreSampler::Failed);
    a[0]=2048;
    s.start(0);
    assert(s.sample(0,a,false)==CentreSampler::Failed);
    s.start(0);
    assert(s.sample(0,a,true)==CentreSampler::Sampling);
    assert(s.sample(1,a,false)==CentreSampler::Failed);
}
