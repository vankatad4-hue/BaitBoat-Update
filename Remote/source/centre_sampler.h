#pragma once
#include <stdint.h>
class CentreSampler {
    uint32_t last_=0;
    int32_t sums_[4]={};
    int low_[4]={}, high_[4]={};
    uint8_t count_=0;
public:
    enum Result { Waiting, Sampling, Complete, Failed };
    void start(uint32_t now) {
        last_=now-4U; count_=0;
        for(int i=0;i<4;++i){sums_[i]=0;low_[i]=4095;high_[i]=0;}
    }
    uint8_t count() const {return count_;}
    int centre(int i) const {return sums_[i]/32;}
    Result sample(uint32_t now,const int (&axes)[4],bool safe) {
        if(!safe) return Failed;
        for(int i=0;i<4;++i) if(axes[i]<0||axes[i]>4095) return Failed;
        if(count_==32) return Complete;
        if(now-last_<4U) return Waiting;
        last_=now;
        for(int i=0;i<4;++i){
            if(axes[i]<low_[i]) low_[i]=axes[i];
            if(axes[i]>high_[i]) high_[i]=axes[i];
            if(high_[i]-low_[i]>100) return Failed;
            sums_[i]+=axes[i];
        }
        return ++count_==32?Complete:Sampling;
    }
};
