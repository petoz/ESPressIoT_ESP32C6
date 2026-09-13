#include "AppNetwork.h"
#include "Configuration.h" // Needed for saveConfig/loadConfig if we move parameters there
#include "Globals.h"
#include "Heater.h"
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

void sendHADiscovery() {
  String devId = String(mqtt_topic);
  devId.replace('/', '_');
  devId.replace(' ', '_');

  String deviceJson = "\"device\":{"
                      "\"ids\":[\"" + devId + "\"],"
                      "\"name\":\"Silvia\","
                      "\"mdl\":\"ESP32-C6 PID\","
                      "\"mf\":\"ESPressIoT\","
                      "\"sw\":\"" + String(FW_VERSION) + "\""
                      "}";

  String baseTopic = String(mqtt_topic);
  String statusTopic = baseTopic + "/status";

  String climateTopic = "homeassistant/climate/" + devId + "/config";
  String heaterTopic = "homeassistant/sensor/" + devId + "_power/config";
  String tempTopic = "homeassistant/sensor/" + devId + "_temp/config";
  String switchTopic = "homeassistant/switch/" + devId + "_power/config";
  String ecoTopic = "homeassistant/sensor/" + devId + "_eco/config";

  if (!ha_discovery_enabled) {
    client.publish(climateTopic.c_str(), "", true);
    client.publish(heaterTopic.c_str(), "", true);
    client.publish(tempTopic.c_str(), "", true);
    client.publish(switchTopic.c_str(), "", true);
    client.publish(ecoTopic.c_str(), "", true);
    client.publish(("homeassistant/number/" + devId + "_kp/config").c_str(), "", true);
    client.publish(("homeassistant/number/" + devId + "_ki/config").c_str(), "", true);
    client.publish(("homeassistant/number/" + devId + "_kd/config").c_str(), "", true);
    Serial.println("HA Discovery is disabled (unregistered from broker).");
    return;
  }
  String climateConfig = "{"
      "\"name\":\"Silvia PID\","
      "\"unique_id\":\"" + devId + "_climate\","
      "\"mode_cmd_t\":\"" + baseTopic + "/set/mode\","
      "\"mode_stat_t\":\"" + statusTopic + "\","
      "\"mode_stat_tpl\":\"{% if value_json.heaterOn %}heat{% else %}off{% endif %}\","
      "\"temp_cmd_t\":\"" + baseTopic + "/set/target\","
      "\"temp_stat_t\":\"" + statusTopic + "\","
      "\"temp_stat_tpl\":\"{{ value_json.targetTemperature | round(1) }}\","
      "\"curr_temp_t\":\"" + statusTopic + "\","
      "\"curr_temp_tpl\":\"{{ value_json.mesauredTemperature | round(1) }}\","
      "\"act_t\":\"" + statusTopic + "\","
      "\"act_tpl\":\"{% if value_json.poweroffMode %}off{% elif value_json.heaterPower > 0 %}heating{% else %}idle{% endif %}\","
      "\"min_temp\":70,\"max_temp\":120,\"temp_step\":0.1,"
      "\"temp_unit\":\"C\","
      "\"modes\":[\"heat\",\"off\"]," +
      deviceJson + "}";
  client.publish(climateTopic.c_str(), climateConfig.c_str(), true);

  // 2. Heater Power Sensor
  String heaterConfig = "{"
      "\"name\":\"Heater Power\","
      "\"unique_id\":\"" + devId + "_heater_power\","
      "\"stat_t\":\"" + statusTopic + "\","
      "\"val_tpl\":\"{{ (value_json.heaterPower / 10.0) | round(1) }}\","
      "\"unit_of_meas\":\"%\","
      "\"icon\":\"mdi:lightning-bolt\"," +
      deviceJson + "}";
  client.publish(heaterTopic.c_str(), heaterConfig.c_str(), true);

  // 3. Current Temperature Sensor
  String tempConfig = "{"
      "\"name\":\"Current Temperature\","
      "\"unique_id\":\"" + devId + "_temperature\","
      "\"stat_t\":\"" + statusTopic + "\","
      "\"val_tpl\":\"{{ value_json.mesauredTemperature | round(1) }}\","
      "\"unit_of_meas\":\"°C\","
      "\"dev_cla\":\"temperature\","
      "\"state_class\":\"measurement\"," +
      deviceJson + "}";
  client.publish(tempTopic.c_str(), tempConfig.c_str(), true);

  // 4. Power Switch
  String switchConfig = "{"
      "\"name\":\"Power\","
      "\"unique_id\":\"" + devId + "_power_switch\","
      "\"cmd_t\":\"" + baseTopic + "/set/power\","
      "\"stat_t\":\"" + statusTopic + "\","
      "\"val_tpl\":\"{% if value_json.heaterOn %}ON{% else %}OFF{% endif %}\","
      "\"pl_on\":\"ON\","
      "\"pl_off\":\"OFF\","
      "\"icon\":\"mdi:power\"," +
      deviceJson + "}";
  client.publish(switchTopic.c_str(), switchConfig.c_str(), true);

  // 5. ECO Time Remaining Sensor
  String ecoConfig = "{"
      "\"name\":\"ECO Time Remaining\","
      "\"unique_id\":\"" + devId + "_eco_remaining\","
      "\"stat_t\":\"" + statusTopic + "\","
      "\"val_tpl\":\"{% if value_json.ecoTimeRemaining >= 0 %}{{ (value_json.ecoTimeRemaining / 60000) | round(0) }}{% else %}0{% endif %}\","
      "\"unit_of_meas\":\"min\","
      "\"icon\":\"mdi:timer-outline\"," +
      deviceJson + "}";
  client.publish(ecoTopic.c_str(), ecoConfig.c_str(), true);

  // Clear any legacy / unwanted PID number entities if they were previously published to broker
  client.publish(("homeassistant/number/" + devId + "_kp/config").c_str(), "", true);
  client.publish(("homeassistant/number/" + devId + "_ki/config").c_str(), "", true);
  client.publish(("homeassistant/number/" + devId + "_kd/config").c_str(), "", true);

  Serial.println("HA MQTT Discovery published.");
}

void MQTT_reconnect() {
  if (mqtt_server[0] == '\0') {
    return;
  }
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection to ");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.println(mqtt_port);

    String clientId = "EspressIoT-";
    clientId += String(random(0xffff), HEX);

    bool connected = false;
    if (strlen(mqtt_user) > 0) {
      Serial.printf("MQTT: Connecting as user '%s'\n", mqtt_user);
      connected = client.connect(clientId.c_str(), mqtt_user, mqtt_pass);
    } else {
      Serial.println("MQTT: Connecting anonymously");
      connected = client.connect(clientId.c_str());
    }

    if (connected) {
      Serial.println("MQTT connected!");
      client.subscribe(mqttConfigTopic, 1);
      String setTopic = String(mqtt_topic) + "/set/#";
      client.subscribe(setTopic.c_str(), 1);
      sendHADiscovery();
    } else {
      Serial.print("MQTT connection failed, rc=");
      Serial.println(client.state());
    }
  }
}

void MQTT_callback(char *topic, byte *payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  msg.trim();
  Serial.print("MQTT message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);

  String topicStr = String(topic);

  // Target temperature: <topic>/set/target or <topic>/config/tset
  if (topicStr.endsWith("/set/target") || topicStr.endsWith("/config/tset")) {
    double val = msg.toFloat();
    if (val > 10.0 && val < 150.0) {
      gTargetTemp = val;
      gEcoStartTime = millis();
      saveConfig();
      Serial.printf("MQTT: Target temp set to %.1f\n", gTargetTemp);
    }
  }
  // Mode command (HA climate sends "heat" or "off"): <topic>/set/mode
  else if (topicStr.endsWith("/set/mode")) {
    if (msg == "off") {
      poweroffMode = true;
      gOutputPwr = 0;
      setHeatPowerPercentage(0);
      Serial.println("MQTT: Mode set to OFF");
    } else if (msg == "heat") {
      poweroffMode = false;
      gEcoStartTime = millis();
      Serial.println("MQTT: Mode set to HEAT");
    }
  }
  // Power switch command: <topic>/set/power
  else if (topicStr.endsWith("/set/power")) {
    if (msg.equalsIgnoreCase("off") || msg == "0") {
      poweroffMode = true;
      gOutputPwr = 0;
      setHeatPowerPercentage(0);
      Serial.println("MQTT: Power turned OFF");
    } else if (msg.equalsIgnoreCase("on") || msg == "1") {
      poweroffMode = false;
      gEcoStartTime = millis();
      Serial.println("MQTT: Power turned ON");
    }
  }
  // Toggle command: <topic>/config/toggle
  else if (topicStr.endsWith("/config/toggle")) {
    poweroffMode = !poweroffMode;
    if (poweroffMode) {
      gOutputPwr = 0;
      setHeatPowerPercentage(0);
      Serial.println("MQTT: Toggled OFF");
    } else {
      gEcoStartTime = millis();
      Serial.println("MQTT: Toggled ON");
    }
  }
}

void setupMQTT() {
  uint16_t port = atoi(mqtt_port);
  if (port == 0) {
    port = 1883;
  }
  client.setServer(mqtt_server, port);
  client.setCallback(MQTT_callback);
  client.setBufferSize(1536);

  mqttConfigTopicStr = String(mqtt_topic) + "/config/#";
  mqttConfigTopic = mqttConfigTopicStr.c_str();

  mqttStatusTopicStr = String(mqtt_topic) + "/status";
  mqttStatusTopic = mqttStatusTopicStr.c_str();

  if (client.connected()) {
    client.disconnect();
  }
}

void loopMQTT() {
  if (!mqtt_enabled || mqtt_server[0] == '\0') {
    if (client.connected()) {
      client.disconnect();
    }
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      static unsigned long lastReconnectAttempt = 0;
      unsigned long now = millis();
      if (now - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = now;
        MQTT_reconnect();
      }
    } else {
      client.loop();

      static unsigned long lastPublish = 0;
      unsigned long now = millis();
      if (now - lastPublish >= 1000) {
        lastPublish = now;
        client.publish(mqttStatusTopic, gStatusAsJson.c_str());
      }
    }
  }
}
