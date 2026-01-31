#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <PID_v1.h>

#define FW_VERSION "1.1.0"
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

extern PID ESPPID;

#endif
