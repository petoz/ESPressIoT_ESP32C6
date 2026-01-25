#include "Tuning.h"
#include "Globals.h"
#include "Heater.h"

void tuning_on() {
  tune_time = 0;
  tune_start = 0;
  tune_count = 0;
  UpperCnt = 0;
  LowerCnt = 0;
  AvgUpperT = 0;
  AvgLowerT = 0;
  maxUpperT = 0;
  minLowerT = 0;
  ESPPID.SetMode(MANUAL);
  tuning = true;
}

void tuning_off() {
  ESPPID.SetMode(AUTOMATIC);
  tuning = false;

  double dt = float(tune_time - tune_start) / tune_count;
  double dT = ((AvgUpperT / UpperCnt) - (AvgLowerT / LowerCnt));

  double Ku = 4 * (2 * aTuneStep) / (dT * 3.14159);
  double Pu = dt / 1000; // units of seconds

  gP = 0.6 * Ku;
  gI = 1.2 * Ku / Pu;
  gD = 0.075 * Ku * Pu;
}

void tuning_loop() {
  //
  // count seconds between power-on-cycles
  //
  //
  if (gInputTemp <
      (gTargetTemp - aTuneThres)) { // below lower threshold -> power on
    if (gOutputPwr == 0) {          // just fell below threshold
      if (tune_count == 0)
        tune_start = time_now;
      tune_time = time_now;
      tune_count++;
      // AvgLowerT += gInputTemp;
      // LowerCnt++;
    }
    if (minLowerT > gInputTemp || minLowerT == 0)
      minLowerT = gInputTemp;
    gOutputPwr = aTuneStep;
    setHeatPowerPercentage(aTuneStep);
  } else if (gInputTemp >
             (gTargetTemp + aTuneThres)) { // above upper threshold -> power off
    // if (gOutputPwr == aTuneStep) { // just crossed upper threshold
    // AvgUpperT += gInputTemp;
    // UpperCnt++;
    //}
    if (maxUpperT < gInputTemp)
      maxUpperT = gInputTemp;
    gOutputPwr = 0;
    setHeatPowerPercentage(0);
  }

  // store min / max
  if ((gInputTemp > (gTargetTemp - (aTuneThres / 2))) &&
      (gInputTemp < (gTargetTemp + (aTuneThres / 2)))) {
    if (maxUpperT != 0) {
      Serial.println("Adding new upper T");
      Serial.println(maxUpperT);
      AvgUpperT += maxUpperT;
      UpperCnt++;
      maxUpperT = 0;
    }
    if (minLowerT != 0) {
      Serial.println("Adding new lower T");
      Serial.println(minLowerT);
      AvgLowerT += minLowerT;
      LowerCnt++;
      minLowerT = 0;
    }
  }
}
