#include "offline_map_coverage.h"
#include <SD.h>
#include <math.h>
static constexpr uint16_t INDEX_LIMIT=6000;
static CoverageTile rows[INDEX_LIMIT];static uint16_t rowCount=0,presentCount=0,missingCount=0;static bool valid=false;
bool mapCoverage_begin(){rowCount=presentCount=missingCount=0;valid=false;File f=SD.open("/maps/coverage_index.csv",FILE_READ);if(!f)return false;char line[48];bool header=false;
while(f.available()){size_t n=f.readBytesUntil('\n',line,sizeof(line)-1);line[n]=0;if(!n)continue;if(!header){header=true;if(strcmp(line,"z,x,y,present")){f.close();return false;}continue;}if(rowCount>=INDEX_LIMIT){f.close();rowCount=0;return false;}char* a=strtok(line,",");char* b=strtok(nullptr,",");char* c=strtok(nullptr,",");char* d=strtok(nullptr,",");if(!a||!b||!c||!d){f.close();rowCount=0;return false;}int z=atoi(a),p=atoi(d);uint32_t x=strtoul(b,nullptr,10),y=strtoul(c,nullptr,10);if(z<0||z>22||x>=(1UL<<z)||y>=(1UL<<z)||(p!=0&&p!=1)){f.close();rowCount=0;return false;}rows[rowCount++]={(uint8_t)z,x,y,(uint8_t)p};if(p)presentCount++;else missingCount++;}
f.close();valid=header&&rowCount>0;return valid;}
bool mapCoverage_hasIndex(){return valid;}
bool mapCoverage_tilePresent(uint8_t z,uint32_t x,uint32_t y){if(!valid)return false;for(uint16_t i=0;i<rowCount;i++)if(rows[i].z==z&&rows[i].x==x&&rows[i].y==y)return rows[i].present!=0;return false;}
void mapCoverage_getStats(uint16_t &p,uint16_t &ok,uint16_t &m){p=rowCount;ok=presentCount;m=missingCount;}
static constexpr double MERCATOR=85.05112878;
static int tx(double lon,uint8_t z){return floor((lon+180.0)/360.0*(1UL<<z));}
static int ty(double lat,uint8_t z){lat=fmax(-MERCATOR,fmin(MERCATOR,lat));double r=lat*M_PI/180.0;return floor((1-asinh(tan(r))/M_PI)/2*(1UL<<z));}
void mapCoverage_checkSquare(double lat,double lon,uint16_t side,const uint8_t*zs,uint8_t count,uint16_t &need,uint16_t &covered,uint16_t &miss){need=covered=miss=0;if(!valid||!zs)return;double h=side/2.0,dl=h/111320.0,co=cos(lat*M_PI/180.0);if(fabs(co)<1e-5)return;double west=lon-h/(111320.0*co),east=lon+h/(111320.0*co),north=lat+dl,south=lat-dl;
for(uint8_t k=0;k<count;k++){uint8_t z=zs[k];if(z>22)continue;int n=1<<z,x0=tx(west,z),x1=tx(east,z),y0=ty(north,z),y1=ty(south,z);if(y0>y1){int t=y0;y0=y1;y1=t;}if(y0<0)y0=0;if(y1>=n)y1=n-1;for(int x=x0;x<=x1;x++)for(int y=y0;y<=y1;y++){need++;uint32_t wx=(x%n+n)%n;if(mapCoverage_tilePresent(z,wx,y))covered++;else miss++;}}
}
