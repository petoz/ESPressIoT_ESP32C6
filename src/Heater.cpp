#include "Heater.h"
#include "Globals.h"

#define HEAT_RELAY_PIN 20 // ESP32-C6 GPIO 20 for SSR

float heatcycles; // the number of millis out of 1000 for the current heat
                  // amount (percent * 10)
bool heaterState = 0;
unsigned long heatCurrentTime = 0, heatLastTime = 0;

void setupHeater() { pinMode(HEAT_RELAY_PIN, OUTPUT); }

void updateHeater() {
  heatCurrentTime = time_now;
  if (heatCurrentTime - heatLastTime >= HEATER_INTERVAL or
      heatLastTime >
          heatCurrentTime) { // second statement prevents overflow errors
    // begin cycle
    _turnHeatElementOnOff(1); //
    heatLastTime = heatCurrentTime;
  }
  if (heatCurrentTime - heatLastTime >= heatcycles) {
    _turnHeatElementOnOff(0);
  }
}

void setHeatPowerPercentage(float power) {
  if (power < 0.0) {
    power = 0.0;
  }
  if (power > 1000.0) {
    power = 1000.0;
  }
  heatcycles = power;
}

float getHeatCycles() { return heatcycles; }

void _turnHeatElementOnOff(bool on) {
  digitalWrite(HEAT_RELAY_PIN, on); // turn pin high
  heaterState = on;
}
