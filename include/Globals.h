#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <PID_v1.h>

#define FW_VERSION "1.5.0-beta.2"
#ifndef GIT_COMMIT
#define GIT_COMMIT "unknown"
#endif
#ifndef BUILD_TIME
#define BUILD_TIME "unknown"
#endif

extern char mqtt_server[40];
extern char mqtt_port[6];
extern char mqtt_user[20];
extern char mqtt_pass[20];
extern char mqtt_topic[32];

// Standard reset values (optimal for Rancilio Silvia)
#define S_P 91.0
#define S_I 0.26
#define S_D 7950.0
#define S_aP 100.0
#define S_aI 0.0
#define S_aD 0.0
#define S_TSET 90.0
#define S_TBAND 1.5
#define DEFAULT_RREF 430.0
#define DEFAULT_ECO_TIME 0.0

// Intervals
#define HEATER_INTERVAL 1000
#define DISPLAY_INTERVAL 1000
#define PID_INTERVAL 200

// Safety limits
// Heater is forced off if no valid temperature sample arrives for this long.
#define SENSOR_STALE_MS 2000
// ESP-side cutoff. Must stay above the normal steam-mode temperature
// (~160 C on Silvia), where the PID output is bypassed anyway.
#define MAX_SAFE_TEMP 175.0
// Readings outside this range are treated as sensor errors.
#define SENSOR_MIN_VALID -20.0
#define SENSOR_MAX_VALID 250.0
// A sample differing from the previous one by more than this is held back
// as a possible spike until it is confirmed by following samples.
#define SENSOR_SPIKE_LIMIT 5.0
#define SENSOR_SPIKE_CONFIRM 3
// Independent hardware-timer watchdog: SSR pin is forced low if the main
// loop does not refresh the heater for this long.
#define HEATER_MAX_ON_MS 1500
// Task watchdog timeout for the main loop.
#define LOOP_WDT_TIMEOUT_MS 10000

// Global variables
extern double gTargetTemp;
extern double gOvershoot;
extern double gInputTemp;
extern double gOutputPwr;
extern double gP, gI, gD;
extern double gaP, gaI, gaD;
extern double gRref;    // Reference resistor for MAX31865
extern double gEcoTime; // ECO timer in minutes

extern unsigned long time_now;
extern unsigned long time_last;
extern unsigned long gEcoStartTime; // Timestamp when heater involved

extern int gButtonState;
extern uint8_t mac[6];

extern boolean tuning;
extern boolean osmode;
extern boolean poweroffMode;
extern boolean externalControlMode;
extern boolean gSensorFault; // true while heater is forced off (sensor/over-temp)

extern String gStatusAsJson;

// Tuning variables
extern double aTuneStep;
extern double aTuneThres;
extern double maxUpperT;
extern double minLowerT;
extern double AvgUpperT;
extern double AvgLowerT;
extern int UpperCnt;
extern int LowerCnt;
extern int tune_count;
extern unsigned long tune_time;
extern unsigned long tune_start;

extern bool mqtt_enabled;
extern bool ha_discovery_enabled;

extern PID ESPPID;

#endif
