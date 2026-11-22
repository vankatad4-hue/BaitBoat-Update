#pragma once
#include <stdint.h>
uint32_t millis();
template <typename T> constexpr T constrain(T v,T lo,T hi){
    return v<lo?lo:(v>hi?hi:v);
}
