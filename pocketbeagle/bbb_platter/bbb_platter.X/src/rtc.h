/*
 * Real Time Clock Module header
 */
#ifndef RTC_H
#define RTC_H

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>


// Exerns
extern volatile uint32_t rtcTime;
extern volatile uint32_t rtcAlarmTime;
extern volatile uint32_t rtcRepeatTime;
extern volatile bit rtcAlarmEnable;


// API
void RTC_Initialize();

void RTC_SetupPowerup();

void RTC_SetupSleep();

void RTC_SetTime(uint32_t t);

uint32_t RTC_GetTime();

bool RTC_SawAlarm();

void RTC_ISR_EvaluateState();


// API Macros
#define RTC_SetAlarmTime(a)   (rtcAlarmTime = a)
#define RTC_SetRepeatTime(r)  (rtcRepeatTime = r)
#define RTC_GetAlarmTime()    (rtcAlarmTime)
#define RTC_GetRepeatTime()   (rtcRepeatTime)
#define RTC_SetAlarmEnable(e) (rtcAlarmEnable = (e == 0) ? 0 : 1)
#define RTC_GetAlarmEnable()  ((rtcAlarmEnable == 1) ? 1 : 0)

#endif // RTC_H
