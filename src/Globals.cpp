#include "Globals.h"

char mqtt_server[40] = "contabo2.usemy.cloud";
char mqtt_port[6] = "1883";
char mqtt_user[20] = "petoz";
char mqtt_pass[20] = "xanticavid";

double gTargetTemp = S_TSET;
double gOvershoot = S_TBAND;
double gInputTemp = 20.0;
double gOutputPwr = 0.0;
double gP = S_P, gI = S_I, gD = S_D;
double gaP = S_aP, gaI = S_aI, gaD = S_aD;

unsigned long time_now = 0;
unsigned long time_last = 0;

int gButtonState = 0;
uint8_t mac[6];

boolean tuning = false;
boolean osmode = false;
boolean poweroffMode = false;
boolean externalControlMode = false;

String gStatusAsJson;

// Tuning variables
double aTuneStep = 100.0;
double aTuneThres = 0.2;
double maxUpperT = 0;
double minLowerT = 0;
double AvgUpperT = 0;
double AvgLowerT = 0;
int UpperCnt = 0;
int LowerCnt = 0;
int tune_count = 0;
unsigned long tune_time = 0;
unsigned long tune_start = 0;

PID ESPPID(&gInputTemp, &gOutputPwr, &gTargetTemp, gP, gI, gD, DIRECT);
