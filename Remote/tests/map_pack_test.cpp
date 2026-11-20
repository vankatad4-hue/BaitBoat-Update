#include "map_pack_manifest.h"
#include "offline_map_coverage.h"
#include <SD.h>
#include <cassert>
#include <string>
#include <iostream>
MockSD SD;
static void load(const std::string &s){SD.files["/maps/coverage_index.csv"]={s.begin(),s.end()};}
static bool manifest(const std::string &s){MapPackManifest p={};return mapPack_parseManifest(s.data(),s.size(),p);}
int main(){
 const std::string good="FLYTEC_MAP1\r\nstyle=street\r\npack_complete=true\r\nattribution=Test\r\nlicence_url=https://example.test\r\nzoom_levels=0,1\r\nexported_tile_count=2\r\n";
 assert(manifest(good));assert(!manifest(good+"style=satellite\n"));assert(!manifest(good+"pack_complete=false\n"));assert(!manifest("x"+good));assert(!manifest(good+std::string(1,'\0')));
 auto bad=good;bad.replace(bad.find("0,1"),3,"1,1");assert(!manifest(bad));bad=good;bad.replace(bad.find("count=2"),7,"count=99999999999999999999");assert(!manifest(bad));
 load("z,x,y,present\r\n0,0,0,1\r\n1,1,1,0\r\n");assert(mapCoverage_begin("/maps"));assert(mapCoverage_tilePresent(0,0,0));assert(!mapCoverage_tilePresent(1,1,1));
 uint16_t n,p,m;mapCoverage_getStats(n,p,m);assert(n==2&&p==1&&m==1);
 for(const auto &v:{"z,x,y,present\n0,0,0,1\n0,0,0,1\n","z,x,y,present\n1,1,1,1\n0,0,0,1\n","z,x,y,present\n0,1,0,1\n","z,x,y,present\n0,0,0,2\n","z,x,y,present\n0,0,0,1\n1,1,1,1,5\n","z,x,y,present\n22,99999999999999999999999,0,1\n"}){load(v);assert(!mapCoverage_begin("/maps"));mapCoverage_getStats(n,p,m);assert(!n&&!p&&!m);}
 load("z,x,y,present\n0,0,0,1\n");assert(mapCoverage_begin("/maps"));uint8_t z=0;mapCoverage_checkSquare(0,179.999,1000,&z,1,n,p,m);assert(n==1&&p==1&&m==0);z=22;mapCoverage_checkSquare(43,25,1000,&z,1,n,p,m);assert(mapCoverage_calculationCapped());
 // A syntactically valid index padded with blank lines must fail before unbounded parsing.
 load("z,x,y,present\n0,0,0,1\n"+std::string(6001U*48U,'\n'));
 assert(!mapCoverage_begin("/maps"));mapCoverage_getStats(n,p,m);assert(!n&&!p&&!m);
 assert(!mapCoverage_tilePresent(0,0,0));
 // A subsequent valid pack must recover after rejection.
 load("z,x,y,present\n0,0,0,1\n");assert(mapCoverage_begin("/maps"));
 assert(mapCoverage_tilePresent(0,0,0));
 assert(!mapCoverage_begin(nullptr));mapCoverage_getStats(n,p,m);assert(!n&&!p&&!m);
 std::cout<<"Manifest and coverage parser fault cases passed\n";
}
