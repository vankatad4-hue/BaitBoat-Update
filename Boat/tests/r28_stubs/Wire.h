#pragma once
#include <stdint.h>
#include <stddef.h>
#include <vector>
class TwoWire {
public:
 uint8_t address=0;
 std::vector<uint8_t> tx;
 std::vector<uint8_t> rx;
 size_t index=0;
 bool failEnd=false, failRequest=false, failRead=false;
 unsigned begins=0;
 void beginTransmission(uint8_t a){address=a;tx.clear();begins++;}
 size_t write(uint8_t b){tx.push_back(b);return 1;}
 uint8_t endTransmission(bool stop=true){(void)stop;return failEnd?2:0;}
 size_t requestFrom(uint8_t a,uint8_t len){if(a!=address || failRequest)return 0;index=0;return rx.size()<len?rx.size():len;}
 int read(){if(failRead||index>=rx.size())return -1;return rx[index++];}
};
