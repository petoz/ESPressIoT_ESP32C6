#include "Heater.h"
#include "Globals.h"
#include <esp_timer.h>

#define HEAT_RELAY_PIN 20 // ESP32-C6 GPIO 20 for SSR

float heatcycles; // the number of millis out of 1000 for the current heat
                  // amount (percent * 10)
bool heaterState = 0;
unsigned long heatCurrentTime = 0, heatLastTime = 0;

// Independent safety timer. It is re-armed every time the heater is switched
// on. If the main loop stalls (blocking network call, hang, ...) and does not
// switch the heater off or re-arm the timer, the timer forces the SSR pin low.
static esp_timer_handle_t heaterWatchdogTimer = nullptr;

static void heaterWatchdogExpired(void *) {
  digitalWrite(HEAT_RELAY_PIN, LOW);
  heaterState = 0;
}

void setupHeater() {
  // Drive the pin low before anything else.
  digitalWrite(HEAT_RELAY_PIN, LOW);
  pinMode(HEAT_RELAY_PIN, OUTPUT);
  digitalWrite(HEAT_RELAY_PIN, LOW);
  heatcycles = 0;

  esp_timer_create_args_t args = {};
  args.callback = heaterWatchdogExpired;
  args.name = "heater_wd";
  esp_timer_create(&args, &heaterWatchdogTimer);
}

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
  if (heaterWatchdogTimer) {
    esp_timer_stop(heaterWatchdogTimer); // ignore error if not running
    if (on) {
      esp_timer_start_once(heaterWatchdogTimer,
                           (uint64_t)HEATER_MAX_ON_MS * 1000ULL);
    }
  }
  digitalWrite(HEAT_RELAY_PIN, on); // turn pin high
  heaterState = on;
}
