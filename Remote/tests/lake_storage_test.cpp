#include "lake_storage.h"
#include "boat_state.h"
#include <SD.h>
#include <cassert>
#include <iostream>
MockSD SD;
BoatState boatState={};
static void safe(){boatState={};boatState.mode=MODE_MANUAL;boatState.radioConnected=true;boatState.emergencyStop=true;boatState.emergencyStopConfirmed=true;}
int main(){
    safe();assert(!lakeStorage_begin(false));assert(lakeStorage_begin(true));
    for(int i=1;i<=8;i++){char p[16];snprintf(p,sizeof(p),"/Lake_%02d",i);assert(SD.exists(p));}
    assert(lakeStorage_active()==0);assert(!lakeStorage_select(8));
    boatState.mode=MODE_AUTOPILOT;assert(!lakeStorage_select(1));safe();
    boatState.emergencyStopConfirmed=false;assert(!lakeStorage_select(1));safe();
    boatState.motorLeftActive=true;assert(!lakeStorage_select(1));safe();
    boatState.motorRightActive=true;assert(!lakeStorage_select(1));safe();
    boatState.radioConnected=false;assert(!lakeStorage_select(1));safe();
    boatState.failsafeActive=true;assert(!lakeStorage_select(1));safe();
    SD.failRenameFrom="/active_lake.tmp";assert(!lakeStorage_select(1));assert(lakeStorage_begin(true));assert(lakeStorage_active()==0);SD.failRenameFrom.clear();
    for(uint8_t i=0;i<8;i++){
        assert(lakeStorage_select(i));
        char name[48];snprintf(name,sizeof(name),"Test lake %u",(unsigned)i);
        assert(lakeStorage_setName(i,name));assert(lakeStorage_setZoom((uint8_t)(10+i)));
        assert(lakeStorage_mapRoot()==lakeStorage_path("/maps"));
        assert(lakeStorage_commitMap(lakeStorage_mapStage()));
        assert(lakeStorage_mapRoot()==lakeStorage_path("/maps-alt"));
        std::string path=lakeStorage_path("/waypoints.bin").c_str();
        SD.files[path]={(uint8_t)i};
    }
    assert(lakeStorage_begin(true)&&lakeStorage_active()==7);
    for(uint8_t i=0;i<8;i++){
        assert(lakeStorage_select(i));
        char name[48],expected[48];snprintf(expected,sizeof(expected),"Test lake %u",(unsigned)i);
        assert(lakeStorage_getName(i,name,sizeof(name))&&strcmp(name,expected)==0);assert(lakeStorage_getZoom()==10+i);
        assert(SD.files[lakeStorage_path("/waypoints.bin").c_str()][0]==i);
        assert(lakeStorage_mapRoot()==lakeStorage_path("/maps-alt"));
    }
    assert(lakeStorage_select(0));
    SD.failRenameFrom="/Lake_01/map_slot.tmp";assert(!lakeStorage_commitMap(lakeStorage_mapStage()));
    assert(lakeStorage_begin(true));assert(lakeStorage_mapRoot()==lakeStorage_path("/maps-alt"));SD.failRenameFrom.clear();
    assert(lakeStorage_commitMap(lakeStorage_mapStage()));assert(lakeStorage_mapRoot()==lakeStorage_path("/maps"));
    assert(lakeStorage_prepareMapStage());SD.files["/Lake_01/map_slot.bin"][0]^=0xff;assert(lakeStorage_mapRoot()==lakeStorage_path("/maps"));
    SD.failRenameFrom="/Lake_01/name.tmp";assert(!lakeStorage_setName(0,"replacement"));
    char name[48];assert(lakeStorage_getName(0,name,sizeof(name))&&strcmp(name,"Test lake 0")==0);SD.failRenameFrom.clear();
    SD.shortWrite=true;assert(!lakeStorage_setName(0,"short write"));assert(!lakeStorage_select(1));SD.shortWrite=false;
    assert(!lakeStorage_setName(0,"  "));assert(!lakeStorage_setName(8,"bad"));
    assert(lakeStorage_path("/../other").length()==0);
    SD.failRenameFrom="/Lake_01/zoom.tmp";assert(!lakeStorage_setZoom(19));assert(lakeStorage_getZoom()==10);SD.failRenameFrom.clear();
    assert(!lakeStorage_setZoom(20));
    SD.mounted=false;assert(!lakeStorage_ready());assert(!lakeStorage_select(1));assert(lakeStorage_mapRoot().length()==0);
    std::cout<<"Lake storage isolation / safety / simulated write recovery passed\n";
}
