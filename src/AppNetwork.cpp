#include "AppNetwork.h"
#include "Globals.h"
#include "Helpers.h"
#include "WiFiSecrets.h"
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClient.h>

#define MAX_CONNECTION_RETRIES 20

// MQTT Settings
// If defined in WiFiSecrets.h, use them. If not, define default or rely on them
// being there. Original code had them in mqtt.ino, but here user might have
// them in WiFiSecrets.h? The warning said MQTT_HOST is in WiFiSecrets.h
#ifndef MQTT_HOST
#define MQTT_HOST "contabo2.usemy.cloud"
#endif

// Some might not be in Secrets
#ifndef MQTT_TOPIC
#define MQTT_TOPIC "espressiot"
#endif
#ifndef MQTT_USER
#define MQTT_USER "petoz"
#endif
#ifndef MQTT_PASS
#define MQTT_PASS "xanticavid"
#endif
#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif

WiFiClient espClient;
PubSubClient client(espClient);

String mqttConfigTopicStr = String(MQTT_TOPIC) + "/config/#";
const char *mqttConfigTopic = mqttConfigTopicStr.c_str();

String mqttStatusTopicStr = String(MQTT_TOPIC) + "/status";
const char *mqttStatusTopic = mqttStatusTopicStr.c_str();

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.macAddress(mac);

  Serial.println("");
  Serial.print("MAC address: ");
  Serial.println(macToString(mac));

  Serial.print("Connecting to Wifi AP");
  for (int i = 0; i < MAX_CONNECTION_RETRIES && WiFi.status() != WL_CONNECTED;
       i++) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Error connection to AP after ");
    Serial.print(MAX_CONNECTION_RETRIES);
    Serial.println(" retries.");
  } else {
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void MQTT_reconnect() {
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("connected");
      client.subscribe(mqttConfigTopic, 1);
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
    }
  }
}

void MQTT_callback(char *topic, byte *payload, unsigned int length) {
#ifdef MQTT_DEBUG
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] '");
#endif

  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
#ifdef MQTT_DEBUG
    Serial.print((char)payload[i]);
#endif
    msg += (char)payload[i];
  }

#ifdef MQTT_DEBUG
  Serial.println("'");
#endif

  double val = msg.toFloat();

  if (strstr(topic, "/config/tset")) {
    if (val > 1e-3)
      gTargetTemp = val;
  } else if (strstr(topic, "/config/toggle")) {
    poweroffMode = (!poweroffMode);
  }
}

void setupMQTT() {
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(MQTT_callback);
}

void loopMQTT() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      for (int i = 0; i < 2 && !client.connected(); i++) {
        MQTT_reconnect();
        if (!client.connected())
          delay(100);
      }
    }
    if (client.connected()) {
      client.loop();
      client.publish(mqttStatusTopic, gStatusAsJson.c_str());
    }
  }
}
