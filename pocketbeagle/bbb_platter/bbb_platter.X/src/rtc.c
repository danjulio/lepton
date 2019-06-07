/*
 * Real Time Clock Module
 */
#include "rtc.h"

// Variables - initialized here for power-on-reset condition
volatile bit rtcAlarmEnable = 0;
volatile bit rtcSawAlarm = 0;
volatile bit rtcEnableDetect = 0;
volatile uint32_t rtcTime = 0;
volatile uint32_t rtcAlarmTime = 0;
volatile uint32_t rtcRepeatTime = 0;


// API
void RTC_Initialize()
{
    // Initialize TIMER1
    T1CON = 0x8D;  // External Oscillator, 1:1 pre-scale, unsynchronized
    T1GCON = 0x00; // Gate disabled
    TMR1L = 0;
    TMR1H = 0x80;  // Rollover in 32,768 counts - our frequency!!! ;-)

    // Enable TIMER1 interrupts
    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 1;
    INTCONbits.PEIE = 1;
}


void RTC_SetupPowerup()
{
    // Disable generating alarms when powered up
    rtcEnableDetect = 0;
    rtcSawAlarm = 0;
}


void RTC_SetupSleep()
{
    // Enable generating alarms when asleep
    rtcSawAlarm = 0;
    rtcEnableDetect = 1;
}


void RTC_SetTime(uint32_t t)
{
    // Disable interrupts
    PIE1bits.TMR1IE = 0;

    rtcTime = t;

    // Re-enable interrupts
    PIE1bits.TMR1IE = 1;
}


uint32_t RTC_GetTime()
{
    uint32_t tmp;

    // Disable interrupts
    PIE1bits.TMR1IE = 0;

    tmp = rtcTime;

    // Re-enable interrupts
    PIE1bits.TMR1IE = 1;

    return(tmp);
}


bool RTC_SawAlarm()
{
    bool tmp;

    // Disable interrupts
    PIE1bits.TMR1IE = 0;

    tmp = (rtcSawAlarm == 1);
    rtcSawAlarm = 0;

    // Re-enable interrupts
    PIE1bits.TMR1IE = 1;

    return(tmp);
}


void RTC_ISR_EvaluateState()
{
    // Reset TIMER1
    PIR1bits.TMR1IF = 0;
    TMR1H = 0x80;  // Preset for next second count

    // Bump time
    rtcTime = rtcTime+1;

    // Evaluate Alarm
    if (rtcAlarmEnable == 1) {
        if (rtcTime == rtcAlarmTime) {
            if (rtcEnableDetect == 1) {
                // Only note an alarm if we are enabled to do so
                rtcSawAlarm = 1;
            }

            // Setup for next alarm if desired
            if (rtcRepeatTime != 0) {
                rtcAlarmTime += rtcRepeatTime;
            } else {
                // Disable alarms
                rtcAlarmEnable = 0;
            }
        }
    }
}