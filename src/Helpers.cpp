#include "Helpers.h"
#include "Globals.h"

String macToID(const uint8_t *mac) {
  String result;
  for (int i = 3; i < 6; ++i) {
    result += String(mac[i], 16);
  }
  result.toUpperCase();
  return result;
}

String macToString(const uint8_t *mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ":";
  }
  result.toUpperCase();
  return result;
}

String statusAsJson() {
  StaticJsonDocument<256> statusObject;
  String outputString;

  statusObject["time"] = time_now;
  statusObject["mesauredTemperature"] = gInputTemp;
  statusObject["targetTemperature"] = gTargetTemp;
  statusObject["heaterPower"] = gOutputPwr;
  statusObject["externalControlMode"] = externalControlMode;
  statusObject["externalControlMode"] = externalControlMode;
  statusObject["externalButtonState"] = gButtonState;

  // ECO Time Remaining logic
  long remainingMs = -1;
  if (gEcoTime > 0.1 && !poweroffMode) {
    remainingMs = (long)(gEcoTime * 60000) - (long)(time_now - gEcoStartTime);
    if (remainingMs < 0)
      remainingMs = 0;
  }
  statusObject["ecoTimeRemaining"] = remainingMs;

  serializeJson(statusObject, outputString);
  return outputString;
}
