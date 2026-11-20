#include "map_pack_manifest.h"
#include <cstring>
#include <cstdlib>
#include <cerrno>
bool mapPack_parseUnsigned(const char *text,unsigned long max,unsigned long &value){
 if(!text||!*text)return false;
 for(const char *p=text;*p;++p)if(*p<'0'||*p>'9')return false;
 errno=0;char *end=nullptr;unsigned long n=strtoul(text,&end,10);
 if(errno==ERANGE||*end||n>max)return false;
 value=n;return true;
}
static bool getValue(const char *s,const char *key,char *out,size_t cap){
 size_t k=strlen(key);bool found=false;
 while(*s){
  const char *end=strchr(s,'\n');size_t n=end?(size_t)(end-s):strlen(s);
  if(n&&s[n-1]=='\r')--n;
  if(n>k&&s[k]=='='&&!memcmp(s,key,k)){
   size_t len=n-k-1;if(found||!len||len>=cap)return false;
   memcpy(out,s+k+1,len);out[len]=0;found=true;
  }
  if(!end)break;
  s=end+1;
 }
 return found;
}
bool mapPack_parseManifest(const char *bytes,size_t length,MapPackManifest &out){
 out={};if(!bytes||length<12||length>=MAP_MANIFEST_LIMIT||memchr(bytes,0,length))return false;
 char data[MAP_MANIFEST_LIMIT];memcpy(data,bytes,length);data[length]=0;
 if(strncmp(data,"FLYTEC_MAP1\n",12)&&strncmp(data,"FLYTEC_MAP1\r\n",13))return false;
 char value[256];unsigned long count=0;MapPackManifest parsed={};
 if(!getValue(data,"pack_complete",value,sizeof(value))||strcmp(value,"true"))return false;
 if(!getValue(data,"style",value,sizeof(value)))return false;
 if(!strcmp(value,"street"))parsed.style=0;else if(!strcmp(value,"satellite"))parsed.style=1;else return false;
 if(!getValue(data,"attribution",parsed.attribution,sizeof(parsed.attribution)))return false;
 if(!getValue(data,"licence_url",value,sizeof(value)))return false;
 if(!getValue(data,"zoom_levels",value,sizeof(value)))return false;
 char *p=value;uint32_t seen=0;
 while(true){char *comma=strchr(p,',');if(comma)*comma=0;unsigned long z=0;if(!mapPack_parseUnsigned(p,22,z)||(seen&(1UL<<z)))return false;seen|=1UL<<z;if(!comma)break;p=comma+1;}
 if(!getValue(data,"exported_tile_count",value,sizeof(value))||!mapPack_parseUnsigned(value,6000,count)||!count)return false;
 parsed.tileCount=(uint16_t)count;out=parsed;return true;
}
