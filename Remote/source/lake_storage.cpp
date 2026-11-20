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

static bool mapSlot(uint8_t &slot) {
    if(!lakeStorage_ready()) return false;
    String current=lakeStorage_path("/map_slot.bin"), backup=lakeStorage_path("/map_slot.bak");
    if(readSelection(current.c_str(),slot) && slot<2) return true;
    if(readSelection(backup.c_str(),slot) && slot<2) return true;
    if(SD.exists(current.c_str()) || SD.exists(backup.c_str())) return false;
    slot=0; return true;
}
static String mapRoot(uint8_t slot) {return lakeStorage_path(slot==0?"/maps":"/maps-alt");}
String lakeStorage_mapRoot(){uint8_t slot;return mapSlot(slot)?mapRoot(slot):String();}
String lakeStorage_mapStage(){uint8_t slot;return mapSlot(slot)?mapRoot(1-slot):String();}
bool lakeStorage_makeDirectories(const char *path){
    if(!lakeStorage_ready() || !path || path[0]!='/') return false;
    String dir(path);
    for(unsigned int i=1;i<=dir.length();++i){
        if(i!=dir.length() && dir[i]!='/')continue;
        String p=dir.substring(0,i);
        if(!SD.exists(p.c_str()) && !SD.mkdir(p.c_str())) return false;
    }
    return true;
}
bool lakeStorage_commitMap(const String &stagedRoot){
    uint8_t old; if(!mapSlot(old) || stagedRoot!=mapRoot(1-old))return false;
    uint8_t next=1-old, check;
    String dst=lakeStorage_path("/map_slot.bin"), tmp=lakeStorage_path("/map_slot.tmp"), bak=lakeStorage_path("/map_slot.bak");
    if(SD.exists(tmp.c_str())&&!SD.remove(tmp.c_str()))return false;
    File out=SD.open(tmp.c_str(),FILE_WRITE);if(!out)return false;
    uint8_t bytes[2]={next,(uint8_t)(next^0xff)};
    size_t written=out.write(bytes,2);out.flush();out.close();
    if(written!=2||!readSelection(tmp.c_str(),check)||check!=next)return false;
    // Ensure recovery points to the old slot, including the initial implicit slot 0.
    if(!readSelection(bak.c_str(),check)||check!=old){
        if(SD.exists(bak.c_str())&&!SD.remove(bak.c_str()))return false;
        File backup=SD.open(bak.c_str(),FILE_WRITE);if(!backup)return false;
        uint8_t prior[2]={old,(uint8_t)(old^0xff)};
        size_t n=backup.write(prior,2);backup.flush();backup.close();
        if(n!=2||!readSelection(bak.c_str(),check)||check!=old)return false;
    }
    if(SD.exists(dst.c_str())&&!SD.remove(dst.c_str()))return false;
    return SD.rename(tmp.c_str(),dst.c_str());
}
