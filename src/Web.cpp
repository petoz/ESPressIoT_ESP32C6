#include "Web.h"
#include "Configuration.h"
#include "Globals.h"
#include "Tuning.h"
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>

WebServer server(80);

void handleStatusApi() { server.send(200, "application/json", gStatusAsJson); }

void handleNotFound() {
  if (LittleFS.exists(server.uri())) {
    File file = LittleFS.open(server.uri(), "r");
    String contentType = "text/plain";
    if (server.uri().endsWith(".html"))
      contentType = "text/html";
    else if (server.uri().endsWith(".css"))
      contentType = "text/css";
    else if (server.uri().endsWith(".js"))
      contentType = "application/javascript";

    server.streamFile(file, contentType);
    file.close();
  } else {
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
}

// Keep old handlers for backward/logic compatibility where needed,
// but point root to index.html
void handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  server.streamFile(file, "text/html");
  file.close();
}

void handleConfig() {
  String message =
      "<head><meta name=\"viewport\" content=\"width=device-width, "
      "initial-scale=1.0\" /><title>EspressIoT "
      "Configuration</title></head><h1>EspressIoT</h1>\n";
  if (tuning) {
    message += "<h1> PID TUNING MODE RUNNING !</h1>";
    message += "<a href=\"/tuningstats\"><button>Stats</button></a><br/>\n";
    message += "<hr/>\n";
  }
  message += "<form action=\"set_config\">\nTarget Temperature:<br>\n";
  message += "<input type=\"text\" name=\"tset\" value=\"" +
             String(gTargetTemp) + "\"><br/><br/>\n";
  message += "<form action=\"set_config\">\nThreshold for adaptive PID:<br>\n";
  message += "<input type=\"text\" name=\"tband\" value=\"" +
             String(gOvershoot) + "\"><br/><br/>\n";
  message +=
      "normal PID:<br>\n P <input type=\"text\" name=\"pgain\" value=\"" +
      String(gP) + "\"><br/>\n";
  message += "I <input type=\"text\" name=\"igain\" value=\"" + String(gI) +
             "\"><br/>\n";
  message += "D <input type=\"text\" name=\"dgain\" value=\"" + String(gD) +
             "\"><br><br>\n";
  message +=
      "adaptive PID:<br>\n P <input type=\"text\" name=\"apgain\" value=\"" +
      String(gaP) + "\"><br/>\n";
  message += "I <input type=\"text\" name=\"aigain\" value=\"" + String(gaI) +
             "\"><br/>\n";
  message += "D <input type=\"text\" name=\"adgain\" value=\"" + String(gaD) +
             "\"><br><br>\n";
  message += "<input type=\"submit\" value=\"Submit\">\n</form>";
  message += "<hr/>";
  message += "<a href=\"./loadconf\"><button>Load Config</button></a><br/>\n";
  message += "<a href=\"./saveconf\"><button>Save Config</button></a><br/>\n";
  message += "<a href=\"./resetconf\"><button>Reset Config to "
             "Default</button></a><br/>\n";
  message += "<a href=\"./update\"><button>Update Firmware</button></a><br/>\n";
  message += "<hr/>\n";
  message += "<form action=\"set_tuning\">\nTuning Threshold (\°C):<br>\n";
  message += "<input type=\"text\" name=\"tunethres\" value=\"" +
             String(aTuneThres) + "\"><br>\n";
  message += "Tuning Power (heater)<br>\n";
  message += "<input type=\"text\" name=\"tunestep\" value=\"" +
             String(aTuneStep) + "\"><br><br>\n";
  message += "<input type=\"submit\" value=\"Submit\">\n</form><br/>";
  if (!tuning)
    message += "<a href=\"./tuningmode\"><button "
               "style=\"background-color:#98B4D4\">Start PID Tuning "
               "Mode</button></a><br/>\n";
  else
    message += "<a href=\"./tuningmode\"><button "
               "style=\"background-color:#98B4D4\">Finish PID Tuning "
               "Mode</button></a><br/>\n";
  message += "<hr/>\n";
  message += "<a href=\"/\"><button>Back</button></a><br/>\n";
  server.send(200, "text/html", message);
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
    else if (server.argName(i) == "pgain")
      gP = server.arg(i).toFloat();
    else if (server.argName(i) == "igain")
      gI = server.arg(i).toFloat();
    else if (server.argName(i) == "dgain")
      gD = server.arg(i).toFloat();
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

void setupWebSrv() {
  LittleFS.begin();

  server.on("/", handleRoot);
  server.on("/api/status", handleStatusApi);
  server.on("/set_config", handleSetConfig);
  server.on("/heater_on", handleHeaterOn);
  server.on("/heater_off", handleHeaterOff);

  // Legacy/Other handlers can be kept or migrated
  // For brevity/focus on new UI, we rely on LittleFS serving mostly.

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loopWebSrv() { server.handleClient(); }
