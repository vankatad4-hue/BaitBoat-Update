#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <string>
using std::size_t;
class String {
    std::string value;
public:
    String()=default;
    String(const char *s):value(s?s:""){}
    explicit String(std::string s):value(s){}
    const char *c_str()const{return value.c_str();}
    size_t length()const{return value.size();}
    char operator[](size_t i)const{return value[i];}
    String substring(size_t a,size_t b)const{return String(value.substr(a,b-a));}
    friend String operator+(const String&a,const String&b){return String(a.value+b.value);}
    friend bool operator==(const String&a,const String&b){return a.value==b.value;}
    friend bool operator!=(const String&a,const String&b){return !(a==b);}
};
