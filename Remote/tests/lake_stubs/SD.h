#pragma once
#include "Arduino.h"
#include <map>
#include <set>
#include <vector>
#include <algorithm>
constexpr int CARD_NONE=0, FILE_READ=0, FILE_WRITE=1;
class File {
    std::vector<uint8_t>* data=nullptr;size_t pos=0;bool writable=false;bool shortWrite=false;
public:
    File()=default;
    File(std::vector<uint8_t>* d,bool w,bool shortW):data(d),pos(w?d->size():0),writable(w),shortWrite(shortW){}
    explicit operator bool()const{return data!=nullptr;}
    size_t size()const{return data?data->size():0;}
    int read(){return data&&pos<data->size()?(*data)[pos++]:-1;}
    size_t read(uint8_t *p,size_t n){size_t count=0;while(count<n){int c=read();if(c<0)break;p[count++]=(uint8_t)c;}return count;}
    size_t write(const uint8_t*p,size_t n){if(!data||!writable)return 0;if(shortWrite&&n)n--;data->insert(data->end(),p,p+n);return n;}
    int available()const{return data?(int)(data->size()-pos):0;}
    size_t readBytesUntil(char delim,char *out,size_t max){size_t n=0;while(n<max&&available()){int c=read();if(c==delim)break;out[n++]=(char)c;}return n;}
    void flush(){}
    void close(){data=nullptr;}
};
class MockSD {
public:
    bool mounted=true,shortWrite=false;
    std::string failRenameFrom,failRemove;
    std::map<std::string,std::vector<uint8_t>> files;
    std::set<std::string> dirs={"/"};
    int cardType()const{return mounted?1:CARD_NONE;}
    bool exists(const char*p)const{return mounted&&(files.count(p)||dirs.count(p));}
    bool mkdir(const char*p){return mounted&&dirs.insert(p).second;}
    bool remove(const char*p){return mounted&&failRemove!=p&&files.erase(p);}
    bool rename(const char*a,const char*b){if(!mounted||failRenameFrom==a||exists(b)||!files.count(a))return false;files[b]=files[a];files.erase(a);return true;}
    File open(const char*p,int mode){
        if(!mounted||!p||!*p)return {};
        if(mode==FILE_READ){auto i=files.find(p);return i==files.end()?File():File(&i->second,false,false);}
        return File(&files[p],true,shortWrite);
    }
};
extern MockSD SD;
