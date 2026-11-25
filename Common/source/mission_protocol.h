#pragma once
#include <stdint.h>
// Portable mission protocol candidate. Payloads are exactly eight bytes.
// The authenticated transport must enforce peer/session/replay validation before calling this API.
enum MissionOpcode:uint8_t { MISSION_BEGIN=1,MISSION_TARGET_LAT=2,MISSION_TARGET_LON=3,MISSION_HOME_LAT=4,MISSION_HOME_LON=5,MISSION_COMMIT=6,MISSION_CANCEL=7 };
struct MissionPlan{uint16_t transaction;uint8_t lake,waypoint,releaseMask;uint16_t radiusCm;int32_t targetLatE7,targetLonE7,homeLatE7,homeLonE7;};
bool missionPlan_valid(const MissionPlan &p);
bool missionPlan_encode(const MissionPlan &p,MissionOpcode opcode,uint8_t out[8]);
uint32_t missionPlan_digest(const MissionPlan &p);
enum MissionReceive:uint8_t { MISSION_REJECTED=0,MISSION_PARTIAL,MISSION_PREPARED,MISSION_DUPLICATE,MISSION_CANCELLED };
class MissionAssembler{
public:
 MissionAssembler():plan_{},mask_(0),startedAt_(0),seen_(false),prepared_(false),cancelled_(false){}
 void reset();
 // The caller must reject unauthenticated/out-of-session/replayed packets before ingestion.
 MissionReceive ingest(const uint8_t payload[8],uint32_t now,bool authenticated,bool stopped);
 bool getPrepared(MissionPlan &out,uint32_t now)const;
private:
 MissionPlan plan_;uint8_t mask_;uint32_t startedAt_;bool seen_,prepared_,cancelled_;
};
