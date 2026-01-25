//
// ESPressIoT Controller for Espresso Machines
// 2016–2021 by Roman Schmitz
//
// MQTT integration
//

#define MQTT_DEBUG

#ifdef ENABLE_MQTT

char buf_msg[50];

#include <SPI.h>
#include <Ethernet.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

// 🔧 TU SI DEFINUJ MQTT KONSTANTY (ak nie sú v config.h alebo podobne)
#define MQTT_TOPIC "espressiot"
#define MQTT_USER "petoz"
#define MQTT_PASS "xanticavid"
#define MQTT_HOST "contabo2.usemy.cloud"
#define MQTT_PORT 1883
#define MAX_CONNECTION_RETRIES 5

extern String gStatusAsJson;
extern double gTargetTemp;
extern bool poweroffMode;

WiFiClient espClient;
PubSubClient client(espClient);

// 🔧 STRINGY UDRŽANÉ V PREMENNÝCH, aby c_str() nevypršalo
String mqttConfigTopicStr = String(MQTT_TOPIC) + "/config/#";
const char* mqttConfigTopic = mqttConfigTopicStr.c_str();

String mqttStatusTopicStr = String(MQTT_TOPIC) + "/status";
const char* mqttStatusTopic = mqttStatusTopicStr.c_str();

void MQTT_reconnect() {
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
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

void MQTT_callback(char* topic, byte* payload, unsigned int length) {
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
    msg += (char) payload[i];
  }

#ifdef MQTT_DEBUG
  Serial.println("'");
#endif

  double val = msg.toFloat();

  if (strstr(topic, "/config/tset")) {
    if (val > 1e-3) gTargetTemp = val;
  }
  else if (strstr(topic, "/config/toggle")) {
    poweroffMode = (!poweroffMode);
  }
}

void setupMQTT() {
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(MQTT_callback);
}

void loopMQTT() {
  for (int i = 0; i < MAX_CONNECTION_RETRIES && !client.connected(); i++) {
    MQTT_reconnect();
    Serial.print(".");
  }

  client.loop();
  client.publish(mqttStatusTopic, gStatusAsJson.c_str());
}

#endif // ENABLE_MQTT