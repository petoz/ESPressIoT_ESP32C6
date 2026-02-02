#include "Web.h"
#include "Configuration.h"
#include "Globals.h"
#include "Tuning.h"
#include <SPIFFS.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>

#include "WebStatic.h"

WebServer server(80);

void handleStatusApi() { server.send(200, "application/json", gStatusAsJson); }

void handleGetConfig() {
  String json = "{";
  json += "\"tset\":" + String(gTargetTemp) + ",";
  json += "\"tband\":" + String(gOvershoot) + ",";
  json += "\"eco_time\":" + String(gEcoTime) + ",";
  json += "\"mqtt_enabled\":" + String(mqtt_enabled) + ",";
  json += "\"mqtt_topic\":\"" + String(mqtt_topic) + "\",";
  json += "\"rref\":" + String(gRref) + ",";
  
  json += "\"pgain\":" + String(gP) + ",";
  json += "\"igain\":" + String(gI) + ",";
  json += "\"dgain\":" + String(gD) + ",";
  
  json += "\"apgain\":" + String(gaP) + ",";
  json += "\"aigain\":" + String(gaI) + ",";
  json += "\"adgain\":" + String(gaD) + ",";
  
  json += "\"tunethres\":" + String(aTuneThres) + ",";
  json += "\"tunestep\":" + String(aTuneStep) + ",";

  json += "\"fw_version\":\"" + String(FW_VERSION) + "\",";
  json += "\"git_commit\":\"" + String(GIT_COMMIT) + "\",";
  json += "\"build_time\":\"" + String(BUILD_TIME) + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleRoot() { server.send(200, "text/html", index_html); }

void handleCss() { server.send(200, "text/css", style_css); }

void handleJs() { server.send(200, "application/javascript", script_js); }

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  server.send(404, "text/plain", message);
}

// Keep old handlers for backward/logic compatibility where needed,
// but point root to index.html
// void handleRoot() {
//   File file = SPIFFS.open("/index.html", "r");
//   server.streamFile(file, "text/html");
//   file.close();
// }

// Serve static config file
void handleConfig() {
  server.send(200, "text/html", config_html);
}

void handleTuningStats() {
  String message =
      "<head><meta http-equiv=\"refresh\" content=\"5\">\n<meta "
      "name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" "
      "/><title>EspressIoT Configuration</title></head><h1>EspressIoT</h1>\n";
  message += "<h1> PID TUNING STATS </h1>";
  message += "Total power-on-cycles: " + String(tune_count) + "<br/>\n";
  message += "Time elapsed: " + String(tune_time - tune_start) + " ms<br/>\n";
  message +=
      "Average Period: " + String(float(tune_time - tune_start) / tune_count) +
      " ms<br/>\n";
  message += "Upper Average: " + String(AvgUpperT / UpperCnt) + " °C<br/>\n";
  message += "Lower Average: " + String(AvgLowerT / LowerCnt) + " °C<br/>\n";
  message += "<hr/>\n";
  message += "<a href=\"./tuningmode\"><button "
             "style=\"background-color:#7070EE\">Finish PID Tuning "
             "Mode</button></a><br/>\n";
  message += "<hr/>\n";
  message += "<a href=\"/config\"><button>Back</button></a><br/>\n";
  server.send(200, "text/html", message);
}

void handleSetConfig() {
  // Same logic but returns JSON or redirects
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "tset")
      gTargetTemp = server.arg(i).toFloat();
    else if (server.argName(i) == "tband")
      gOvershoot = server.arg(i).toFloat();
    else if (server.argName(i) == "eco_time") {
      gEcoTime = server.arg(i).toFloat();
      gEcoStartTime = millis(); // Reset timer when config changes
    } else if (server.argName(i) == "pgain")
      gP = server.arg(i).toFloat();
    else if (server.argName(i) == "igain")
      gI = server.arg(i).toFloat();
    else if (server.argName(i) == "dgain")
      gD = server.arg(i).toFloat();
    else if (server.argName(i) == "rref")
      gRref = server.arg(i).toFloat();
    else if (server.argName(i) == "mqtt_enabled")
      mqtt_enabled = server.arg(i).toInt();
    else if (server.argName(i) == "mqtt_topic")
      strcpy(mqtt_topic, server.arg(i).c_str());
  }

  if (server.header("Accept").indexOf("application/json") >= 0) {
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(200, "text/plain", "OK");
  }
}

void handleSetTuning() {
  String message =
      "<head><meta http-equiv=\"refresh\" content=\"2;url=/config\">\n<meta "
      "name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" "
      "/><title>EspressIoT</title></head><h1>Configuration changed !</h1>\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "tunethres") {
      message += "new tuning threshold: " + server.arg(i) + "<br/>\n";
      aTuneThres = ((server.arg(i)).toFloat());
    } else if (server.argName(i) == "tunestep") {
      message += "new tuning power: " + server.arg(i) + "<br/>\n";
      aTuneStep = ((server.arg(i)).toFloat());
    }
  }
  server.send(200, "text/html", message);
}

void handleLoadConfig() {
  String message =
      "<head><meta http-equiv=\"refresh\" content=\"2;url=/config\">\n<meta "
      "name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" "
      "/><title>EspressIoT</title></head>";
  if (loadConfig())
    message += "<h1>Configuration loaded !</h1>\n";
  else
    message += "<h1>Error loading configuration !</h1>\n";
  server.send(200, "text/html", message);
}

void handleSaveConfig() {
  String message =
      "<head><meta http-equiv=\"refresh\" content=\"2;url=/config\">\n<meta "
      "name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" "
      "/><title>EspressIoT</title></head>";
  if (saveConfig())
    message += "<h1>Configuration saved !</h1>\n";
  else
    message += "<h1>Error saving configuration !</h1>\n";
  server.send(200, "text/html", message);
}

void handleResetConfig() {
  String message =
      "<head><meta http-equiv=\"refresh\" content=\"2;url=/config\">\n<meta "
      "name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" "
      "/><title>EspressIoT</title></head>";
  resetConfig();
  message += "<h1>Configuration set to default !</h1>\n";
  server.send(200, "text/html", message);
}

// ... helper wrappers ...
void handleHeaterOn() {
  poweroffMode = false;
  gEcoStartTime = millis(); // Reset eco timer
  server.send(200, "text/plain", "OK");
}
void handleHeaterOff() {
  poweroffMode = true;
  server.send(200, "text/plain", "OK");
}

void handlePidOn() {
  String message =
      "<head><meta http-equiv=\"refresh\" content=\"2;url=/\">\n<meta "
      "name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" "
      "/><title>EspressIoT</title></head>";
  message += "<h1> Done ! </h1>";
  externalControlMode = false;
  server.send(200, "text/html", message);
}

void handlePidOff() {
  String message =
      "<head><meta http-equiv=\"refresh\" content=\"2;url=/\">\n<meta "
      "name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" "
      "/><title>EspressIoT</title></head>";
  message += "<h1> Done ! </h1>";
  externalControlMode = true;
  server.send(200, "text/html", message);
}

void handleTuningMode() {
  String message =
      "<head><meta http-equiv=\"refresh\" content=\"2;url=/config\">\n<meta "
      "name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" "
      "/><title>EspressIoT</title></head>";
  if (!tuning) {
    tuning_on();
    message += "<h1> Started ! </h1>";
  } else {
    message = "<head><meta name=\"viewport\" content=\"width=device-width, "
              "initial-scale=1.0\" /><title>EspressIoT</title></head>";
    tuning_off();
    message += "<h1> Finished and new parameters calculated ! </h1>";
    message += "Total power-on-cycles: " + String(tune_count) + "<br/>\n";
    message += "Average Period: " +
               String(float(tune_time - tune_start) / tune_count) +
               " ms<br/>\n";
    message += "Average Peak-To-Peak Temperature: " +
               String((AvgUpperT / UpperCnt) - (AvgLowerT / LowerCnt)) +
               " °C<br/>\n";
    message += "<a href=\"/config\"><button>Back</button></a><br/>\n";
  }

  server.send(200, "text/html", message);
}

void handleUpdate() {
  String message =
      "<head><meta name=\"viewport\" content=\"width=device-width, "
      "initial-scale=1.0\" /><title>EspressIoT Firmware "
      "Update</title></head><h1>Firmware Update</h1>\n";
  message +=
      "<form method='POST' action='/update' enctype='multipart/form-data'>";
  message += "<input type='file' name='update'><br><br>";
  message += "<input type='submit' value='Update'>";
  message += "</form>";
  message += "<hr/>";
  message += "<a href=\"/config\"><button>Back</button></a><br/>\n";
  server.send(200, "text/html", message);
}

void handleUpdateUpload() {
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Update: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // start with max available size
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    /* flashing firmware to ESP*/
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) { // true to set the size to the current progress
      Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
  }
}

void handleUpdateResult() {
  String message =
      "<head><meta http-equiv=\"refresh\" content=\"10;url=/\">\n<meta "
      "name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" "
      "/><title>EspressIoT Update</title></head>";
  message += Update.hasError() ? "<h1>Update Failed!</h1>"
                               : "<h1>Update Success! Rebooting...</h1>";
  server.send(200, "text/html", message);
  if (!Update.hasError()) {
    delay(1000);
    ESP.restart();
  }
}

void setupWebSrv() {
  // SPIFFS.begin(true); // SPIFFS no longer required for web serving

  server.on("/", handleRoot);
  server.on("/style.css", handleCss);
  server.on("/script.js", handleJs);
  server.on("/api/status", handleStatusApi);
  server.on("/api/config", handleGetConfig);
  server.on("/set_config", handleSetConfig);
  // OTA Update
  server.on("/update", HTTP_GET, handleUpdate);
  server.on("/update", HTTP_POST, handleUpdateResult, handleUpdateUpload);

  server.on("/heater_on", handleHeaterOn);
  server.on("/heater_off", handleHeaterOff);

  server.on("/config", handleConfig);
  server.on("/config.html", handleConfig);
  server.on("/tuningstats", handleTuningStats);
  server.on("/set_tuning", handleSetTuning);
  server.on("/loadconf", handleLoadConfig);
  server.on("/saveconf", handleSaveConfig);
  server.on("/resetconf", handleResetConfig);
  server.on("/tuningmode", handleTuningMode);

  // Legacy/Other handlers can be kept or migrated
  // For brevity/focus on new UI, we rely on LittleFS serving mostly.

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loopWebSrv() { server.handleClient(); }
