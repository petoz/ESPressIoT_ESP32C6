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

  if (jsonDocument.containsKey("rref"))
    gRref = jsonDocument["rref"];

  if (jsonDocument.containsKey("eco_time"))
    gEcoTime = jsonDocument["eco_time"];

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

  if (jsonDocument.containsKey("mqtt_server"))
    strcpy(mqtt_server, jsonDocument["mqtt_server"]);
  if (jsonDocument.containsKey("mqtt_port"))
    strcpy(mqtt_port, jsonDocument["mqtt_port"]);
  if (jsonDocument.containsKey("mqtt_user"))
    strcpy(mqtt_user, jsonDocument["mqtt_user"]);
  if (jsonDocument.containsKey("mqtt_pass"))
    strcpy(mqtt_pass, jsonDocument["mqtt_pass"]);

  if (jsonDocument.containsKey("mqtt_enabled"))
    mqtt_enabled = jsonDocument["mqtt_enabled"];

  if (jsonDocument.containsKey("mqtt_topic"))
    strcpy(mqtt_topic, jsonDocument["mqtt_topic"]);

  return true;
}

bool saveConfig() {
  DynamicJsonDocument jsonDocument(BUF_SIZE);

  jsonDocument["tset"] = gTargetTemp;
  jsonDocument["tband"] = gOvershoot;
  jsonDocument["rref"] = gRref;
  jsonDocument["eco_time"] = gEcoTime;
  jsonDocument["P"] = gP;
  jsonDocument["I"] = gI;
  jsonDocument["D"] = gD;
  jsonDocument["aP"] = gaP;
  jsonDocument["aI"] = gaI;
  jsonDocument["aD"] = gaD;

  jsonDocument["mqtt_server"] = mqtt_server;
  jsonDocument["mqtt_port"] = mqtt_port;
  jsonDocument["mqtt_user"] = mqtt_user;
  jsonDocument["mqtt_pass"] = mqtt_pass;
  jsonDocument["mqtt_enabled"] = mqtt_enabled;
  jsonDocument["mqtt_topic"] = mqtt_topic;

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
  gTargetTemp = S_TSET;
  gOvershoot = S_TBAND;
  gRref = DEFAULT_RREF;
  gEcoTime = DEFAULT_ECO_TIME;
  mqtt_enabled = true;
  strlcpy(mqtt_topic, "espressiot", sizeof(mqtt_topic));
}
