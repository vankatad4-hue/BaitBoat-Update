#pragma once
#include "boat_commands.h"
#include "boat_hardware_status.h"
#include <deque>
#include <vector>
#include <utility>
extern uint32_t fakeNow;
inline uint32_t millis(){return fakeNow;}
enum ManagedLink:uint8_t{LINK_NONE,LINK_NRF_PRIMARY,LINK_LORA_BACKUP};
struct FakeAck{uint32_t id;bool success;ManagedLink reply;};
class BoatDualRadioPort{
public:
 bool primary=true,backup=false,detached=false,failsafeRth=false;uint32_t gen=1;
 bool returnAllowed=false;unsigned returnRequests=0;
 MissionControlPhase phase=MC_IDLE;ExecOutput intent={INTENT_STOP,-1,0};
 MissionNavigationResult nav{};uint16_t tx=0;
 bool (*input)(ExecInputs &)=nullptr;bool (*gate)()=nullptr;
 std::deque<std::pair<BoatCommand,ManagedLink>> queue;
 std::vector<FakeAck> acks;std::vector<RadioDecodedPayload> telemetry;
 void setMissionInputs(bool (*p)(ExecInputs &)){input=p;}
 void setMissionPreparationGate(bool (*p)()){gate=p;}
 void update(){}
 bool takeCommand(BoatCommand &c,bool &dup,ManagedLink &link){if(queue.empty())return false;c=queue.front().first;link=queue.front().second;queue.pop_front();dup=false;return true;}
 void localEmergencyStop(){phase=MC_ABORTED;intent={INTENT_STOP,-1,0};queue.clear();}
 bool primaryFresh(uint32_t)const{return primary;}
 bool backupFresh(uint32_t)const{return backup;}
 bool detachedReturnActive()const{return detached;}
 bool failsafeReturnHomeActive()const{return failsafeRth;}
 uint32_t sessionGeneration()const{return gen;}
 MissionControlPhase missionPhase()const{return phase;}
 uint16_t missionTransaction()const{return tx;}
 ExecOutput missionIntent()const{return intent;}
 MissionNavigationResult missionNavigation()const{return nav;}
 bool confirmManual(){return false;}
 bool requestReturnHome(){++returnRequests;if(!returnAllowed)return false;detached=true;phase=MC_RETURNING;intent={INTENT_STOP,-1,0};return true;}
 bool missionOverrideOutput(int16_t &l,int16_t &r)const{l=r=0;return false;}
 bool missionReleaseComplete(uint16_t,uint16_t,bool){return true;}
 bool completeApplicationAction(const BoatCommand &c,uint32_t g,bool ok,uint8_t,uint8_t,ManagedLink reply=LINK_NRF_PRIMARY){if(g!=gen)return false;acks.push_back({c.commandId,ok,reply});return true;}
 bool sendHardwareTelemetry(const RadioDecodedPayload &d){if(!primary)return false;telemetry.push_back(d);return true;}
};
