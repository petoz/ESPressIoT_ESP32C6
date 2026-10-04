#include "Sensor.h"
#include "Globals.h"
#include <Adafruit_MAX31865.h>
#include <SPI.h>

// ESP32-C6 MAX31865 Pins
#define MAX_CS 14
#define MAX_MOSI 15
#define MAX_MISO 18
#define MAX_SCK 19

// The reference resistor on the PT100 board (usually 430 or 4300)
// The reference resistor on the PT100 board (usually 430 or 4300)
// #define RREF 430.0 // Now configurable via gRref
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL 100.0

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 max31865 =
    Adafruit_MAX31865(MAX_CS, MAX_MOSI, MAX_MISO, MAX_SCK);

#define TSIC_SMP_TIME                                                          \
  100 // Kept name for compatibility/consistency, but it's for MAX31865 now

float lastT = 0.0;
float SumT = 0.0;
int CntT = 0;
unsigned long lastSensTime;
unsigned long lastValidSampleMs = 0; // time of last accepted sample
bool haveSample = false;             // at least one valid sample received
float spikeT = 0.0;                  // candidate value of a suspected spike
int spikeCnt = 0;

void setupSensor() {
  max31865.begin(MAX31865_3WIRE); // set to 2WIRE, 3WIRE or 4WIRE as necessary
  lastSensTime = millis();
  lastValidSampleMs = millis();
}

void updateTempSensor() {
  if (abs((long)(millis() - lastSensTime)) >= TSIC_SMP_TIME) {

    float curT = max31865.temperature(RNOMINAL, gRref);
    uint8_t fault = max31865.readFault();

    if (fault) {
      Serial.print("Fault 0x");
      Serial.println(fault, HEX);
      if (fault & MAX31865_FAULT_HIGHTHRESH) {
        Serial.println("RTD High Threshold");
      }
      if (fault & MAX31865_FAULT_LOWTHRESH) {
        Serial.println("RTD Low Threshold");
      }
      if (fault & MAX31865_FAULT_REFINLOW) {
        Serial.println("REFIN- > 0.85 x Bias");
      }
      if (fault & MAX31865_FAULT_REFINHIGH) {
        Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
      }
      if (fault & MAX31865_FAULT_RTDINLOW) {
        Serial.println("RTDIN- < 0.85 x Bias - FORCE- open");
      }
      if (fault & MAX31865_FAULT_OVUV) {
        Serial.println("Under/Over voltage");
      }
      max31865.clearFault();
    } else if (isnan(curT) || curT < SENSOR_MIN_VALID ||
               curT > SENSOR_MAX_VALID) {
      // Implausible value (open/shorted sensor, SPI noise): do not use it.
      // No fresh valid sample means the main loop failsafe turns heating off.
      Serial.print("Invalid temperature reading: ");
      Serial.println(curT);
    } else {
      bool accept = false;
      if (!haveSample || fabs(curT - lastT) < SENSOR_SPIKE_LIMIT) {
        accept = true;
        spikeCnt = 0;
      } else {
        // Possible spike. Hold it back, but accept it once enough consecutive
        // samples agree with each other, so a real change can never lock the
        // filter out.
        if (spikeCnt > 0 && fabs(curT - spikeT) < SENSOR_SPIKE_LIMIT) {
          spikeCnt++;
        } else {
          spikeCnt = 1;
        }
        spikeT = curT;
        if (spikeCnt >= SENSOR_SPIKE_CONFIRM) {
          accept = true;
          spikeCnt = 0;
        }
      }
      if (accept) {
        SumT += curT;
        CntT++;
        lastT = curT;
        haveSample = true;
        lastValidSampleMs = millis();
      }
    }
    lastSensTime = millis();
  }
}

bool sensorIsHealthy() {
  return (millis() - lastValidSampleMs) < SENSOR_STALE_MS;
}

float getTemp() {
  float retVal = gInputTemp;

  if (CntT >= 1) {
    retVal = (SumT / CntT);
    SumT = 0.;
    CntT = 0;
  }

  return retVal;
}
