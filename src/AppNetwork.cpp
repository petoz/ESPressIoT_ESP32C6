#include "AppNetwork.h"
#include "Configuration.h" // Needed for saveConfig/loadConfig if we move parameters there
#include "Globals.h"
#include "Helpers.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiManager.h>

WiFiClient espClient;
PubSubClient client(espClient);

String mqttConfigTopicStr;
const char *mqttConfigTopic;

String mqttStatusTopicStr;
const char *mqttStatusTopic;

// Flag for saving data
bool shouldSaveConfig = false;

// Callback notifying us of the need to save config
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setupWiFi() {
  WiFiManager wifiManager;

  // set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // Custom parameters for MQTT
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server,
                                          40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 20);
  WiFiManagerParameter custom_mqtt_pass("pass", "mqtt pass", mqtt_pass, 20);

  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);

  // wifiManager.resetSettings(); // Un-comment to reset for testing

  // Fetches ssid and pass and tries to connect
  // If it does not connect it starts an access point with the specified name
  // and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("ESPressIoT-Setup")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    // reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  // if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  // read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  strcpy(mqtt_pass, custom_mqtt_pass.getValue());

  Serial.println("The values in the file are: ");
  Serial.println("\tmqtt_server : " + String(mqtt_server));
  Serial.println("\tmqtt_port : " + String(mqtt_port));
  Serial.println("\tmqtt_user : " + String(mqtt_user));
  Serial.println("\tmqtt_pass : " + String(mqtt_pass));

  // save the custom parameters to FS
  if (shouldSaveConfig) {
    // Logic handled in Configuration.cpp, we just need to ensure global vars
    // are updated which they are via strcpy above. We should call saveConfig()
    // here. Note: saveConfig() is in Configuration.h/cpp declared in main scope
    // mostly but we included Configuration.h so we can use it.
    saveConfig();
  }

  // Re-init topics with loaded/updated values if they depend on user/etc
  // (optional) For now fixed topics
  mqttConfigTopicStr = String(mqtt_topic) + "/config/#";
  mqttConfigTopic = mqttConfigTopicStr.c_str();

  mqttStatusTopicStr = String(mqtt_topic) + "/status";
  mqttStatusTopic = mqttStatusTopicStr.c_str();

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("silvia")) {
    Serial.println("MDNS responder started");
    MDNS.addService("http", "tcp", 80);
  }
}

void MQTT_reconnect() {
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      client.subscribe(mqttConfigTopic, 1);
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      // Wait 1 seconds before retrying handled in loop
    }
  }
}

void MQTT_callback(char *topic, byte *payload, unsigned int length) {
  // same callback logic
  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  double val = msg.toFloat();

  if (strstr(topic, "/config/tset")) {
    if (val > 1e-3)
      gTargetTemp = val;
  } else if (strstr(topic, "/config/toggle")) {
    poweroffMode = (!poweroffMode);
  }
}

void setupMQTT() {
  uint16_t port = atoi(mqtt_port);
  client.setServer(mqtt_server, port);
  client.setCallback(MQTT_callback);
}

void loopMQTT() {
  if (!mqtt_enabled) {
    if (client.connected()) {
      client.disconnect();
    }
    return;
  }
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      MQTT_reconnect();
    }
    if (client.connected()) {
      client.loop();
      client.publish(mqttStatusTopic, gStatusAsJson.c_str());
    }
  }
}
