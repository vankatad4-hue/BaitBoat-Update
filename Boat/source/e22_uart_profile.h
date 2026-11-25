#pragma once
#ifndef FLYTEC_E22_UART_PROFILE_SHARED_H
#define FLYTEC_E22_UART_PROFILE_SHARED_H
#include <stdint.h>
namespace flytec_rtos {
// E22-900T22S candidate only; confirm module revision before register writes.
// Setup mode is 9600 8N1 even if transparent runtime uses faster UART.
struct E22UartProfile {
 static constexpr uint32_t ConfigBaud=9600U, BaselineBaud=9600U, CandidateBaud=115200U;
 static constexpr uint8_t BaselineReg0=0x67U,CandidateReg0=0xe7U;
 static constexpr uint32_t frameWireMs(uint32_t baud){return baud==BaselineBaud||baud==CandidateBaud?(320000U+baud-1U)/baud:0U;}
 // All prerequisites are independently supplied by the commissioning owner.
 static constexpr bool fastPermitted(bool exactModule,bool hostBaudVerified,bool registerReadback,bool normalModeVerified,bool rfProfileApproved){
  return exactModule&&hostBaudVerified&&registerReadback&&normalModeVerified&&rfProfileApproved;
 }
};
static_assert(E22UartProfile::frameWireMs(9600)==34,"32-byte UART baseline");
static_assert(E22UartProfile::frameWireMs(115200)==3,"32-byte UART candidate");
static_assert((E22UartProfile::BaselineReg0&0x1f)==(E22UartProfile::CandidateReg0&0x1f),"Only UART baud bits change");
}
#endif
