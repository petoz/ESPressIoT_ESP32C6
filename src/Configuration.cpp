#include "Configuration.h"
#include "Globals.h"
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
// #include <StreamUtils.h> // removed for now, maybe not needed if we use
// standard file stream

#define BUF_SIZE 1024

bool prepareFS() {
  if (!SPIFFS.begin(true)) { // true = format if failed
    Serial.println("Failed to mount file system");
    return false;
  }
  return true;
}

bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  // DynamicJsonDocument jsonDocument(BUF_SIZE);
  // Using new ArduinoJson syntax recommended or stick to old?
  // v6 is what was likely used.
  DynamicJsonDocument jsonDocument(BUF_SIZE);

  DeserializationError parsingError = deserializeJson(jsonDocument, configFile);
  if (parsingError) {
    Serial.println("Failed to deserialize json config file");
    Serial.println(parsingError.c_str());
    return false;
  }

  // wifi_ssid = jsonDocument["ssid"];
  // wifi_pass = jsonDocument["password"];
  if (jsonDocument.containsKey("tset"))
    gTargetTemp = jsonDocument["tset"];
  if (jsonDocument.containsKey("tband"))
    gOvershoot = jsonDocument["tband"];

  if (jsonDocument.containsKey("P"))
    gP = jsonDocument["P"];
  if (jsonDocument.containsKey("I"))
    gI = jsonDocument["I"];
  if (jsonDocument.containsKey("D"))
    gD = jsonDocument["D"];

  if (jsonDocument.containsKey("aP"))
    gaP = jsonDocument["aP"];
  if (jsonDocument.containsKey("aI"))
    gaI = jsonDocument["aI"];
  if (jsonDocument.containsKey("aD"))
    gaD = jsonDocument["aD"];

  return true;
}

bool saveConfig() {
  DynamicJsonDocument jsonDocument(BUF_SIZE);

  // jsonDocument["ssid"] = wifi_ssid;  jsonDocument["password"] = wifi_pass;
  jsonDocument["tset"] = gTargetTemp;
  jsonDocument["tband"] = gOvershoot;
  jsonDocument["P"] = gP;
  jsonDocument["I"] = gI;
  jsonDocument["D"] = gD;
  jsonDocument["aP"] = gaP;
  jsonDocument["aI"] = gaI;
  jsonDocument["aD"] = gaD;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  size_t writtenBytes = serializeJson(jsonDocument, configFile);

  if (writtenBytes == 0) {
    Serial.println(F("Failed to write to file"));
    return false;
  }
  Serial.println("Bytes written: " + String(writtenBytes));

  configFile.close();
  return true;
}

void resetConfig() {
  gP = S_P;
  gI = S_I;
  gD = S_D;
  gaP = S_aP;
  gaI = S_aI;
  gaD = S_aD;
  gTargetTemp = S_TSET;
  gOvershoot = S_TBAND;
}
