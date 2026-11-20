#include "lake_storage.h"
#include "boat_state.h"
#include <SD.h>
static uint8_t active = 0;
static bool ready = false;
static String root(uint8_t n) {
    char path[16];
    snprintf(path,sizeof(path),"/Lake_%02u",(unsigned)n+1);
    return String(path);
}
static bool ensure(uint8_t n) {
    String p=root(n);
    return SD.exists(p.c_str()) || SD.mkdir(p.c_str());
}
static bool readSelection(const char *path,uint8_t &n) {
    File f=SD.open(path,FILE_READ);
    if(!f || f.size()!=2) return false;
    int a=f.read(), c=f.read(); f.close();
    if(a<0 || a>=LAKE_COUNT || c!=(a^0xff)) return false;
    n=(uint8_t)a; return true;
}
bool lakeStorage_begin(bool sdReady) {
    ready=false; active=0;
    if(!sdReady || SD.cardType()==CARD_NONE) return false;
    for(uint8_t i=0;i<LAKE_COUNT;++i) if(!ensure(i)) return false;
    if(!readSelection("/active_lake.bin",active)) {
        if(!readSelection("/active_lake.bak",active) &&
           (SD.exists("/active_lake.bin") || SD.exists("/active_lake.bak"))) return false;
    }
    ready=true; return true;
}
uint8_t lakeStorage_active(){return active;}
bool lakeStorage_ready(){return ready && SD.cardType()!=CARD_NONE;}
String lakeStorage_path(const char *relative){
    if(!lakeStorage_ready() || !relative || relative[0]!='/' || strstr(relative,"..")) return String();
    return root(active)+relative;
}
bool lakeStorage_select(uint8_t n){
    if(!lakeStorage_ready() || n>=LAKE_COUNT || boatState.mode!=MODE_MANUAL ||
       !boatState.emergencyStop || !boatState.emergencyStopConfirmed ||
       !boatState.radioConnected || boatState.motorLeftActive || boatState.motorRightActive ||
       boatState.failsafeActive) return false;
    if(n==active) return true;
    const char *tmp="/active_lake.tmp", *dst="/active_lake.bin", *bak="/active_lake.bak";
    SD.remove(tmp);
    File f=SD.open(tmp,FILE_WRITE); if(!f) return false;
    uint8_t bytes[2]={n,(uint8_t)(n^0xff)};
    size_t count=f.write(bytes,2); f.flush(); f.close();
    uint8_t check;
    if(count!=2 || !readSelection(tmp,check) || check!=n) return false;
    if(SD.exists(dst)){
        uint8_t old;
        if(!readSelection(dst,old)) { if(!SD.remove(dst)) return false; }
        else {
            if(SD.exists(bak) && !SD.remove(bak)) return false;
            if(!SD.rename(dst,bak)) return false;
        }
    }
    if(!SD.rename(tmp,dst)) return false;
    active=n; return true;
}
