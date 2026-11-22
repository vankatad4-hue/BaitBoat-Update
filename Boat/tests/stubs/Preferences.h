#pragma once
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

// Host-only Preferences emulator. All instances share the same byte store to
// model ESP32 NVS persistence across object destruction/reconstruction.
class Preferences {
public:
 bool begin(const char*,bool){open_=true;return true;}
 void end(){open_=false;}
 size_t getBytesLength(const char *key)const{
  const auto it=store().find(key?key:"");
  return it==store().end()?0U:it->second.size();
 }
 size_t getBytes(const char *key,void *out,size_t maxLen)const{
  const auto it=store().find(key?key:"");
  if(it==store().end()||!out||it->second.size()>maxLen)return 0U;
  for(size_t i=0;i<it->second.size();++i)static_cast<uint8_t*>(out)[i]=it->second[i];
  return it->second.size();
 }
 size_t putBytes(const char *key,const void *data,size_t len){
  if(!open_||!key||!data)return 0U;
  std::vector<uint8_t> value(len);
  for(size_t i=0;i<len;++i)value[i]=static_cast<const uint8_t*>(data)[i];
  store()[key]=value;return len;
 }
 static void clearStore(){store().clear();}
 static bool corrupt(const char *key,size_t offset){
  auto it=store().find(key?key:"");
  if(it==store().end()||offset>=it->second.size())return false;
  it->second[offset]^=0x40U;return true;
 }
private:
 bool open_=false;
 static std::map<std::string,std::vector<uint8_t>>& store(){
  static std::map<std::string,std::vector<uint8_t>> bytes;
  return bytes;
 }
};
