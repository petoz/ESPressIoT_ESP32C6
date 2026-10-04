#ifndef APP_NETWORK_H
#define APP_NETWORK_H

#include <Arduino.h>

void setupWiFi();
void setupMQTT();   // starts the MQTT task (once) and requests (re)configuration
void loopMQTT();    // main-loop side: only handles deferred work (config save)
void setMqttStatus(const String &json); // thread-safe status hand-over to MQTT

#endif
