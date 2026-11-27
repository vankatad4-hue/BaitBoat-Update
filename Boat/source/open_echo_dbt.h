#pragma once
#include <stdint.h>

// Bounded NMEA-0183 DBT sentence decoder for Open Echo's serial depth mode.
// It accepts checksummed `$??DBT,feet,f,meters,M,fathoms,F*HH` records and
// returns centimeters only for the project's intended 0.50..100.00 m range.
// This parser has no UART, GPIO, SPI, ADC, or actuator access.
class OpenEchoDbtDecoder {
public:
 bool feed(char byte,uint32_t now,uint16_t &depthCm){
  if(active_&&(uint32_t)(now-lastByteAt_)>250U)reset();
  lastByteAt_=now;
  if(byte=='$'){length_=0;active_=true;line_[length_++]=byte;return false;}
  if(!active_)return false;
  if(length_>=sizeof(line_)-1U){reset();return false;}
  line_[length_++]=byte;
  if(byte!='\n')return false;
  active_=false;line_[length_]='\0';
  return decodeLine(depthCm);
 }
 void reset(){length_=0;active_=false;}
private:
 char line_[96]{};uint8_t length_=0;uint32_t lastByteAt_=0;bool active_=false;
 static bool equalsIgnoreCase(const char *a,const char *b){
  while(*a&&*b){char x=*a++,y=*b++;if(x>='a'&&x<='z')x=(char)(x-'a'+'A');if(y>='a'&&y<='z')y=(char)(y-'a'+'A');if(x!=y)return false;}
  return *a=='\0'&&*b=='\0';
 }
 static bool parseMeters(const char *text,uint16_t &cm){
  if(!text||!*text)return false;
  uint32_t whole=0,fraction=0,scale=1;bool decimal=false,haveDigit=false;
  for(const char *p=text;*p;++p){
   if(*p=='.'&&!decimal){decimal=true;continue;}
   if(*p<'0'||*p>'9')return false;
   haveDigit=true;const uint8_t d=(uint8_t)(*p-'0');
   if(decimal){if(scale>=1000U)return false;fraction=fraction*10U+d;scale*=10U;}
   else{if(whole>100U)return false;whole=whole*10U+d;}
  }
  if(!haveDigit||whole>100U||(whole==100U&&fraction!=0U))return false;
  const uint32_t roundedCm=whole*100U+(decimal?(fraction*100U+scale/2U)/scale:0U);
  if(roundedCm<50U||roundedCm>10000U)return false;
  cm=(uint16_t)roundedCm;return true;
 }
 bool decodeLine(uint16_t &depthCm){
  if(length_<15U||line_[0]!='$')return false;
  char *star=nullptr;for(uint8_t i=1;i<length_;++i)if(line_[i]=='*'){star=&line_[i];break;}
  if(!star||star-line_<6||star[1]=='\0'||star[2]=='\0')return false;
  char *tail=star+3;if(*tail=='\r')++tail;if(*tail!='\n'||tail[1]!='\0')return false;
  const int hi=hex(star[1]),lo=hex(star[2]);if(hi<0||lo<0)return false;
  uint8_t checksum=0;for(char *p=line_+1;p<star;++p)checksum^=(uint8_t)*p;
  if(checksum!=(uint8_t)((hi<<4)|lo))return false;
  const uint8_t starOffset=(uint8_t)(star-line_);*star='\0';
  char *fields[7]={};uint8_t count=0;fields[count++]=line_+1;
  for(uint8_t i=1;i<starOffset;++i)if(line_[i]==','){line_[i]='\0';if(count>=7U)return false;fields[count++]=line_+i+1;}
  if(count!=7U)return false;
  const char *tag=fields[0];uint8_t tagLen=0;while(tag[tagLen])++tagLen;
  if(tagLen<5U||tag[tagLen-3]!='D'||tag[tagLen-2]!='B'||tag[tagLen-1]!='T')return false;
  if(!equalsIgnoreCase(fields[2],"f")||!equalsIgnoreCase(fields[4],"M")||!equalsIgnoreCase(fields[6],"F"))return false;
  return parseMeters(fields[3],depthCm);
 }
 static int hex(char c){if(c>='0'&&c<='9')return c-'0';if(c>='A'&&c<='F')return c-'A'+10;if(c>='a'&&c<='f')return c-'a'+10;return -1;}
};
