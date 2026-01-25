#include "Telnet.h"
#include "Globals.h"
#include <WiFi.h>

WiFiServer telnetServer(23);
WiFiClient telnetClient;

void telnetStatus() { telnetClient.println(gStatusAsJson); }

void setupTelnet() {
  telnetServer.begin();
  telnetServer.setNoDelay(true);
  Serial.println("Please connect Telnet Client, exit with ^] and 'quit'");

  Serial.print("Free Heap[B]: ");
  Serial.println(ESP.getFreeHeap());
}

void loopTelnet() {
  if (telnetServer.hasClient()) {
    if (!telnetClient || !telnetClient.connected()) {
      if (telnetClient)
        telnetClient.stop();
      telnetClient = telnetServer.available();
    } else {
      telnetServer.available().stop();
    }
  }
  // The original code only sent status if client sent data?
  // "if (telnetClient && telnetClient.connected() && telnetClient.available())"
  // This means it responds to any key press with status.
  if (telnetClient && telnetClient.connected() && telnetClient.available()) {
    // Read and discard data to clear buffer
    while (telnetClient.available())
      telnetClient.read();
    telnetStatus();
  }
}
