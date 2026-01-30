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

void setupSensor() {
  max31865.begin(MAX31865_3WIRE); // set to 2WIRE, 3WIRE or 4WIRE as necessary
  lastSensTime = millis();
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
    } else {
      // very simple selection of noise hits/invalid values
      if (abs(curT - lastT) < 1.0 ||
          lastT < 1) { // Filter spikes? Or maybe just simple averaging
        // For simple averaging
        SumT += curT;
        CntT++;
        lastT = curT;
      }
    }
    lastSensTime = millis();
  }
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
