#pragma once
// R47: maintenance-only Wi-Fi HTTPS firmware pull. Not Bluetooth OTA.
// No server, credentials, certificate or update trigger compiled by default.
// ESP32 Secure Boot V2 + flash encryption are required for production use.
#include <stdint.h>
struct BoatOtaPreflight {
 bool physicalMaintenanceSwitch=false;
 bool motorPowerIsolated=false;
 bool solenoidPowerIsolated=false;
 bool physicalStopLatched=false;
 bool stationary=false;
 bool batteryHealthy=false;
 bool secureBootEnabled=false;
 bool flashEncryptionEnabled=false;
 bool missionInactive=false;
 bool calibrationInactive=false;
 bool operatorConfirmed=false;
};
inline bool boatOtaPreflightOK(const BoatOtaPreflight &s){
 return s.physicalMaintenanceSwitch&&s.motorPowerIsolated&&s.solenoidPowerIsolated&&
 s.physicalStopLatched&&s.stationary&&s.batteryHealthy&&s.secureBootEnabled&&
 s.flashEncryptionEnabled&&s.missionInactive&&s.calibrationInactive&&s.operatorConfirmed;
}
#if defined(ARDUINO_ARCH_ESP32) && defined(BOAT_WIFI_OTA_COMMISSIONED) && BOAT_WIFI_OTA_COMMISSIONED
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>
#include "esp_secure_boot.h"
#include "esp_flash_encrypt.h"
class BoatWifiOta {
public:
 // Explicit, local maintenance call ONLY. Never call from radio command,
 // web endpoint, boot loop or unattended on-water navigation.
 static bool install(const BoatOtaPreflight &preflight,
                     const char *ssid,const char *password,
                     const char *httpsUrl,const char *caPem){
  if(!boatOtaPreflightOK(preflight)||!esp_secure_boot_enabled()||
     !esp_flash_encryption_enabled()||!ssid||!*ssid||!password||
     !httpsUrl||strncmp(httpsUrl,"https://",8)!=0||!caPem||
     !strstr(caPem,"BEGIN CERTIFICATE"))return false;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  const uint32_t started=millis();
  while(WiFi.status()!=WL_CONNECTED && (uint32_t)(millis()-started)<12000U)delay(25);
  if(WiFi.status()!=WL_CONNECTED){WiFi.disconnect(true);return false;}
  WiFiClientSecure client;
  client.setCACert(caPem); // Never setInsecure().
  httpUpdate.rebootOnUpdate(false);
  // Secure Boot V2 validates the image signature at boot; app must also
  // use the correct OTA partition layout and rollback/health confirmation.
  const t_httpUpdate_return result=httpUpdate.update(client,httpsUrl);
  WiFi.disconnect(true);
  return result==HTTP_UPDATE_OK;
 }
};
#endif
