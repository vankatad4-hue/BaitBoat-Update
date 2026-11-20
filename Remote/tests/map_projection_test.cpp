#include "map_projection.h"
#include <cassert>
#include <cmath>
int main(){
 double lat=43,lon=25;auto p=mapProjection_toWorldPixel(lat,lon,17);double a,b;mapProjection_toLatLon(p,17,a,b);assert(std::fabs(a-lat)<1e-7&&std::fabs(b-lon)<1e-7);
 lat=0;lon=179.99;assert(mapProjection_pan(lat,lon,12,100,0));assert(lon<0&&lon>=-180);
 assert(mapProjection_pan(lat,lon,12,0,-1e9));assert(lat<=85.051129&&lat>85);
 assert(!mapProjection_pan(lat,lon,23,0,0));assert(!mapProjection_pan(lat,lon,12,NAN,0));
 auto q=mapProjection_screenOffsetToWorld(10,0,90);assert(std::fabs(q.x)<1e-8&&std::fabs(q.y-10)<1e-8);
}
