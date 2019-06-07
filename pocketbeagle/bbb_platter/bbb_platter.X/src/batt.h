/*
 * Battery Measurement Module header
 */
#ifndef BATT_H
#define BATT_H

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>


// Default battery threshold values
//    AdcCount = (Vbatt * 0.2362)/1.024 * 1023
//
// Low Battery Warning - 3.6V
#define BATT_DEF_LB_WARN_ADC_VAL 849

// Low Battery Hysteresis (voltage above Low Battery Warning we must see
// before detecting a Low Battery condition again)
#define BATT_LB_HYSTERESIS_COUNT 24

// Critical Battery Voltage - 3.1V - Voltage we should shut down immediately at
#define BATT_DEF_CRIT_ADC_VAL    732

// Critical Battery auto-turnoff timeout (seconds)
#define BATT_CRIT_TURNOFF_SEC    30

// Battery restart enable
#define BATT_RESTART_EN          0

// Warning message enables
#define BATT_WARN_MSG_EN         1
#define BATT_CRIT_MSG_EN         1

// Start-up undervoltage threshold - designed to work with the ADC measuring
// the processor's VDD (through the V_BAT blocking diode) using the internal
// 1.024v reference.  Assume a 0.15 volt drop across the diode at the time the
// measurement is made (this is based on testing with a 22 uF capacitor directly
// on the PIC VCC supply after the V_BAT blocking diode).  Undervoltage
// is detected for AdcCounts >= undervoltage threshold.
//
//    AdcCount = (1.024 / (UVthresh - 0.15)) * 1023
//
// UVthresh = 3.2 volts
#define BATT_UV_ADC_VAL          343

// Restart voltage threshold - designed to work with the ADC measuring the
// processor's VDD using the internal 1.024v reference (like the start-up
// undervoltage threshold).
//
//    AdcCount = (1.024 / (restartThresh - 0.15)) * 1023
//
// restartThresh = 3.8 volts
#define BATT_RESTART_ADC_VAL     287

// Number of consecutive samples required below the low- or critical-voltage
// thresholds to cause a state-change.  Prevents instantaneous battery voltage
// drops from triggering the state change.
#define BATT_NUM_ST_CHNG_SMPLS   5


// Externs
extern bit battWarnMsgEn;
extern bit battCritMsgEn;



// API
void BATT_ReadBatteryInternal();

bool BATT_UndervoltageDetected();

bool BATT_RestartvoltageDetected();

void BATT_SetupPowerup();

float BATT_GetBattVoltage();

void BATT_Tasks();

bool BATT_LowDetected();

bool BATT_CriticalDetected();


// API Macros
#define BATT_SetLowBattWarnEn(e)  (battWarnMsgEn = (e == 0) ? 0 : 1)
#define BATT_SetCritBattWarnEn(e) (battCritMsgEn = (e == 0) ? 0 : 1)
#define BATT_GetLowBattWarnEn()   ((battWarnMsgEn == 1) ? 1 : 0)
#define BATT_GetCritBattWarnEn()  ((battCritMsgEn == 1) ? 1 : 0)

#endif // BATT_H
