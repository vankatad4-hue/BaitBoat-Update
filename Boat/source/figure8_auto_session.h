#pragma once
#include <stdint.h>
#include "figure8_compass_calibration.h"
// Non-actuating coordinator. Operator manually steers a figure eight.
// Completes when both independent compass sweeps have coverage and a minimum
// elapsed time; no automatic actuator commands or navigation enable.
enum class Figure8AutoState:uint8_t{IDLE,COLLECTING,CANDIDATE_READY,TIMED_OUT,ABORTED};
class Figure8AutoSession{
public:
 bool start(uint32_t now,bool gyReady,bool m100Ready){
  if(!gyReady||!m100Ready||state_==Figure8AutoState::COLLECTING)return false;
  gy_.start();m100_.start();start_=now;state_=Figure8AutoState::COLLECTING;
  gyLast_=mLast_=now;return true;
 }
 void addGy(SensorVector m,uint32_t now){if(state_==Figure8AutoState::COLLECTING&&gy_.add(m))gyLast_=now;}
 void addM100(SensorVector m,uint32_t now){if(state_==Figure8AutoState::COLLECTING&&m100_.add(m))mLast_=now;}
 void update(uint32_t now){
  if(state_!=Figure8AutoState::COLLECTING)return;
  const uint32_t elapsed=now-start_;
  if(elapsed>180000U||(uint32_t)(now-gyLast_)>5000U||(uint32_t)(now-mLast_)>5000U){
   state_=Figure8AutoState::TIMED_OUT;return;
  }
  if(elapsed<30000U)return;
  // Use copies so incomplete sweeps remain active and can gather more samples.
  Figure8CompassCalibration a=gy_,b=m100_;
  Figure8Candidate ga{},ma{};
  if(a.finish(ga)&&b.finish(ma)){
   gyCandidate_=ga;mCandidate_=ma;state_=Figure8AutoState::CANDIDATE_READY;
  }
 }
 void abort(){state_=Figure8AutoState::ABORTED;}
 Figure8AutoState state()const{return state_;}
 bool candidates(Figure8Candidate &gy,Figure8Candidate &m100)const{
  if(state_!=Figure8AutoState::CANDIDATE_READY)return false;
  gy=gyCandidate_;m100=mCandidate_;return true;
 }
private:
 Figure8CompassCalibration gy_,m100_;
 Figure8Candidate gyCandidate_{},mCandidate_{};
 Figure8AutoState state_=Figure8AutoState::IDLE;
 uint32_t start_=0,gyLast_=0,mLast_=0;
};
