#ifndef HELPERS_H
#define HELPERS_H

#include <Arduino.h>
#include <ArduinoJson.h>

String macToID(const uint8_t *mac);
String macToString(const uint8_t *mac);
String statusAsJson();

#endif
