/*
 * Battery Measurement Module
 */
#include "batt.h"
#include "adc.h"
#include "cmd.h"
#include "eeprom.h"
#include "system.h"
#include "ups.h"
#include <stdlib.h>


// Averaging parameters - averaging buffer should be a power-of-2 so that
// the rounding function can work correctly.  the NUM_SMPLS and SMPL_SHIFT must
// match: BATT_NUM_SMPLS = 2^BATT_SMPL_SHIFT
#define BATT_NUM_SMPLS       16
#define BATT_SMPL_SHIFT      4
#define BATT_AVG_ROUND_MASK  (1 << (BATT_SMPL_SHIFT-1))
#define BATT_AVG_ROUND_INC   (1 << BATT_SMPL_SHIFT)


// Battery State
#define BATT_ST_OK        0
#define BATT_ST_LOW       1
#define BATT_ST_CRITICAL  2
#define BATT_ST_USBPWR    3


// Variables
uint16_t battIntAdcCount;  // Vref as input against VDD
uint16_t battLowAdcVal;
uint16_t battCritAdcVal;
uint8_t battCritTurnOffTimeout;
bit battWarnMsgEn;
bit battCritMsgEn;

uint16_t battAdcSamples[BATT_NUM_SMPLS];
uint8_t battAdcSamplePushIndex;
uint8_t battState;
uint16_t battAdcAvg;
uint8_t battLowDetectedCount;
uint8_t battCritDetectedCount;
uint8_t battCritTurnOffTimer;



// Internal function forward references
void BATT_ComputeAvg();

void BATT_EvalCondition();

void BATT_NoteOkCondition();

void BATT_NoteLowCondition();

void BATT_NoteCriticalCondition();



// API

// Must be called after ADC is initialized - designed to be called during a
// wake-up event to read the battery voltage using the battery-derived PIC
// VDD against the internal Vref.  Must be called before BATT_UndervoltageDetected
// or BATT_RestartvoltageDetected.
void BATT_ReadBatteryInternal()
{
    uint8_t i;

    // Take 4 readings and compute an average
    battIntAdcCount = 0;
    for (i=0; i<4; i++) {
        battIntAdcCount += ADC_Read(3);   // Read Vref against our supply voltage
    }
    // Round then compute the average by shifting (since we read power-of-two)
    if ((battIntAdcCount & 0x0002) != 0) {
        // Round up
        battIntAdcCount += 0x0004;
    }
    battIntAdcCount = battIntAdcCount >> 2;   // Rounded average value now
}


// Determine if battery is in good-enough condition to allow powering on
// the entire system.
bool BATT_UndervoltageDetected()
{
    // Under-voltage condition detected if the adcVal is above the threshold
    return(battIntAdcCount >= EEP_GetBattStartUvAdc());
}


// Determine if the battery is charged enough to allow an auto-restart
bool BATT_RestartvoltageDetected()
{
    // Good battery condition detected if the adcVal is below the threshold
    return(battIntAdcCount < EEP_GetBattRestartAdc());
}


// Must be called after ADC is initialized
void BATT_SetupPowerup()
{
    // Setup other variables
    battState = BATT_ST_OK;
    battLowDetectedCount = 0;
    battCritDetectedCount = 0;
    battCritTurnOffTimer = 0;  // Disabled
    battAdcSamplePushIndex = 0;
    battAdcAvg = 0;            // Force a full initialization the first time BATT_Tasks is executed
    battLowAdcVal = EEP_GetBattWarnAdc();
    battCritAdcVal = EEP_GetBattCritAdc();
    battCritTurnOffTimeout = EEP_GetBattCritTimeout();
    battWarnMsgEn = EEP_GetBattWarnMsgEn() ? 1 : 0;
    battCritMsgEn = EEP_GetBattCritMsgEn() ? 1 : 0;
}


float BATT_GetBattVoltage()
{
    // This function encapsulates the actual transfer function implemented
    // by the resistor divider in the hardware with the 10-bit ADC Vref
    // set to 1.024 volts.
    //
    // AdcCount = (Vbatt * 0.2362)/1.024 * 1023
    //     ====>
    // Vbatt (volts) = 0.0042 * battAdcAvg
    return((float) battAdcAvg * 0.0042);
}


// Designed to be called only once per second
void BATT_Tasks()
{
    // Get a new sample
    if (battAdcAvg == 0) {
        // First time after powering up, initialize our array all at once
        for (battAdcSamplePushIndex=0; battAdcSamplePushIndex<BATT_NUM_SMPLS; battAdcSamplePushIndex++) {
            battAdcSamples[battAdcSamplePushIndex] = ADC_Read(0);
        }
        battAdcSamplePushIndex = 0;
    } else {
        // Steady state - just add another reading to our average
        battAdcSamples[battAdcSamplePushIndex] = ADC_Read(0);
        if (++battAdcSamplePushIndex == BATT_NUM_SMPLS) {
            battAdcSamplePushIndex = 0;
        }
    }

    BATT_ComputeAvg();
    BATT_EvalCondition();

    // Evaluate our critical battery shut down timer if it is running
    if (battCritTurnOffTimer != 0) {
        if (--battCritTurnOffTimer == 0) {
            SYSTEM_PowerDown();
        }
    }
}


bool BATT_LowDetected()
{
    return((battState == BATT_ST_LOW) || (battState == BATT_ST_CRITICAL));
}


bool BATT_CriticalDetected()
{
    return(battState == BATT_ST_CRITICAL);
}



// Internal functions
void BATT_ComputeAvg()
{
    uint8_t i;
    uint32_t sum;

    // Compute our current average
    sum = 0;
    for (i=0; i<BATT_NUM_SMPLS; i++) {
        sum += battAdcSamples[i];
    }
    if ((sum & BATT_AVG_ROUND_MASK) != 0) {
        // Round up
        sum += BATT_AVG_ROUND_INC;
    }
    battAdcAvg = (sum >> BATT_SMPL_SHIFT);
}


void BATT_EvalCondition()
{
    switch (battState) {
        case BATT_ST_OK:
            if (UPS_UsbPowerEnabled()) {
                battState = BATT_ST_USBPWR;
            } else if (battAdcAvg <= battCritAdcVal) {
                if (++battCritDetectedCount >= BATT_NUM_ST_CHNG_SMPLS) {
                    battState = BATT_ST_CRITICAL;
                    BATT_NoteCriticalCondition();
                }
            } else if (battAdcAvg <= battLowAdcVal) {
                battCritDetectedCount = 0;
                if (++battLowDetectedCount >= BATT_NUM_ST_CHNG_SMPLS) {
                    battState = BATT_ST_LOW;
                    BATT_NoteLowCondition();
                }
            } else {
                battLowDetectedCount = 0;
                battCritDetectedCount = 0;
            }
            break;
        case BATT_ST_LOW:
            if (UPS_UsbPowerEnabled()) {
                battState = BATT_ST_USBPWR;
            } else if (battAdcAvg <= battCritAdcVal) {
                if (++battCritDetectedCount >= BATT_NUM_ST_CHNG_SMPLS) {
                    battState = BATT_ST_CRITICAL;
                    BATT_NoteCriticalCondition();
                }
            } else if (battAdcAvg >= (battLowAdcVal + BATT_LB_HYSTERESIS_COUNT)) {
                battState = BATT_ST_OK;
                battLowDetectedCount = 0;
                battCritDetectedCount = 0;
                BATT_NoteOkCondition();
            } else {
                battCritDetectedCount = 0;
            }
            break;
        case BATT_ST_CRITICAL:
            // Remain in this state until we shut down - unless we get powered
            // by USB
            if (UPS_UsbPowerEnabled()) {
                battState = BATT_ST_USBPWR;
                BATT_NoteOkCondition();
            }
            break;
        case BATT_ST_USBPWR:
            // Remain in this state while powered by USB
            if (UPS_UsbPowerEnabled() == 0) {
                battState = BATT_ST_OK;
                battLowDetectedCount = 0;
                battCritDetectedCount = 0;
            }
            break;
        default:
            battState = BATT_ST_OK;
            BATT_NoteOkCondition();
    }
}


void BATT_NoteOkCondition()
{
    // Disable timer if it's running (we haven't already shut down...)
    battCritTurnOffTimer = 0;
}


void BATT_NoteLowCondition()
{
    if (battWarnMsgEn == 1) {
        CMD_SendWarnMsg(CMD_WARN_LOW_BATT);
    }

    // Disable timer if it's running (we haven't already shut down...)
    battCritTurnOffTimer = 0;
}



void BATT_NoteCriticalCondition()
{
    if (battCritMsgEn == 1) {
        CMD_SendWarnMsg(CMD_WARN_CRIT_BATT);
    }

    // Start shutdown timer
    if (battCritTurnOffTimer == 0) {
        battCritTurnOffTimer = battCritTurnOffTimeout;
    }
}