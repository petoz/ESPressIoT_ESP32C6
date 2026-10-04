#include "AppNetwork.h"
#include "Configuration.h"
#include "Globals.h"
#include "Heater.h"
#include "Helpers.h"
#include "Sensor.h"
#include "Telnet.h"
#include "Tuning.h"
#include "Web.h"
#include <Arduino.h>
#include <esp_task_wdt.h>

// Options
#define ENABLE_JSON
#define ENABLE_HTTP
#define ENABLE_TELNET
#define ENABLE_MQTT
#define ENABLE_SERIAL // Added explicit define for usage in loop

// #define SIMULATION_MODE // Disabled for real hardware

// Forward declarations
void setupWiFi();
void setupMQTT();
void loopMQTT();

void setup() {
  // Safety first: make sure the SSR is off before anything that can block
  // (WiFiManager portal, SPIFFS, ...) runs.
  setupHeater();
  gOutputPwr = 0;

  Serial.begin(115200);

  Serial.println("Mounting SPIFFS...");
  if (!prepareFS()) {
    Serial.println("Failed to mount SPIFFS !");
  } else {
    Serial.println("Mounted.");
  }

  Serial.println("Loading config...");
  if (!loadConfig()) {
    Serial.println(
        "Failed to load config. Using default values and creating config...");
    if (!saveConfig()) {
      Serial.println("Failed to save config");
    } else {
      Serial.println("Config saved");
    }
  } else {
    Serial.println("Config loaded");
  }

  Serial.println("Settin up PID...");

  setupWiFi(); // Replaces WiFi setup block

#ifdef ENABLE_TELNET
  setupTelnet();
#endif

#ifdef ENABLE_HTTP
  setupWebSrv();
#endif

#ifdef ENABLE_MQTT
  setupMQTT();
#endif

  // setup components
  setupSensor();

  // start PID
  ESPPID.SetTunings(gP, gI, gD);
  ESPPID.SetSampleTime(PID_INTERVAL);
  ESPPID.SetOutputLimits(0, 1000);
  ESPPID.SetMode(AUTOMATIC);

  time_now = millis();
  time_now = millis();
  time_last = time_now;
  gEcoStartTime = time_now; // Initialize ECO timer

  // Watchdog: reboot if the main loop hangs. The heater pin is also forced low
  // by its own hardware timer if the loop stalls.
  esp_task_wdt_config_t wdtConfig = {};
  wdtConfig.timeout_ms = LOOP_WDT_TIMEOUT_MS;
  wdtConfig.idle_core_mask = 0;
  wdtConfig.trigger_panic = true;
  if (esp_task_wdt_reconfigure(&wdtConfig) != ESP_OK) {
    esp_task_wdt_init(&wdtConfig);
  }
  esp_task_wdt_add(NULL);
}

void serialStatus() { Serial.println(gStatusAsJson); }

void loop() {
  esp_task_wdt_reset();
  time_now = millis();

  updateTempSensor();
  gInputTemp = getTemp();

  // loopSwitch(); // Switch detection disabled

  if (abs((long)(time_now - time_last)) >= PID_INTERVAL or
      time_last > time_now) {
    // Failsafe: no fresh valid temperature, or temperature above the limit.
    bool failsafe = !sensorIsHealthy() || gInputTemp > MAX_SAFE_TEMP;
    if (failsafe != gSensorFault) {
      gSensorFault = failsafe;
      if (failsafe) {
        Serial.println("FAILSAFE: stale/invalid temperature or over limit, "
                       "heater forced OFF");
        if (!tuning) {
          ESPPID.SetMode(MANUAL);
        }
      } else {
        Serial.println("FAILSAFE cleared: temperature valid again");
        if (!tuning) {
          ESPPID.SetMode(AUTOMATIC); // re-initialises the integral term
        }
      }
    }

    if (poweroffMode == true) {
      gOutputPwr = 0;
      setHeatPowerPercentage(gOutputPwr);
    } else if (gSensorFault) {
      gOutputPwr = 0;
      setHeatPowerPercentage(gOutputPwr);
    } else if (externalControlMode == true) {
      gOutputPwr = 1000 * gButtonState;
      setHeatPowerPercentage(gOutputPwr);
    } else if (tuning == true) {
      tuning_loop();
    } else {
      if (!osmode && abs(gTargetTemp - gInputTemp) >= gOvershoot) {
        ESPPID.SetTunings(gaP, gaI, gaD);
        osmode = true;
      } else if (osmode && abs(gTargetTemp - gInputTemp) < gOvershoot) {
        ESPPID.SetTunings(gP, gI, gD);
        osmode = false;
      }
      if (ESPPID.Compute() == true) {
        setHeatPowerPercentage(gOutputPwr);
      }
    }

    // create status String (JSON)
    gStatusAsJson = statusAsJson();
    setMqttStatus(gStatusAsJson);

#ifdef ENABLE_TELNET
    loopTelnet();
#endif

#ifdef ENABLE_SERIAL
    serialStatus();
#endif

    // ECO Mode Logic
    if (gEcoTime > 0.1 &&
        !poweroffMode) { // Use small threshold for float comparison
      if ((time_now - gEcoStartTime) > (unsigned long)(gEcoTime * 60000)) {
        handleHeaterOff();
        Serial.println("ECO Mode: Heater turned off automatically.");
      }
    }

    time_last = time_now;
  }

  updateHeater();

#ifdef ENABLE_HTTP
  loopWebSrv();
#endif

#ifdef ENABLE_MQTT
  loopMQTT();
#endif
}
