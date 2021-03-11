/*
 * DS3232 RTC Module
 *
 * Provides access to the DS3232 Real-Time clock (both timekeeping and parameter RAM).
 *
 * Based on Jack Christensen's Arduino library
 *  https://github.com/JChristensen/DS3232RTC
 *
 * with routines from Michael Margolis' time.c file.
 *
 * Copyright 2020 Jack Christensen, Michael Margolis and Dan Julio
 *
 * This file is part of tCam.
 *
 * tCam is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tCam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tCam.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef DS3232_H
#define DS3232_H

#include <stdint.h>
#include "esp_system.h"
#include <time.h>
#include <sys/time.h>


//
// RTC constants
//

// DS3232 I2C Address
#define RTC_ADDR 0x68

// DS3232 Register Addresses
#define RTC_SECONDS 0x00
#define RTC_MINUTES 0x01
#define RTC_HOURS 0x02
#define RTC_DAY 0x03
#define RTC_DATE 0x04
#define RTC_MONTH 0x05
#define RTC_YEAR 0x06
#define ALM1_SECONDS 0x07
#define ALM1_MINUTES 0x08
#define ALM1_HOURS 0x09
#define ALM1_DAYDATE 0x0A
#define ALM2_MINUTES 0x0B
#define ALM2_HOURS 0x0C
#define ALM2_DAYDATE 0x0D
#define RTC_CONTROL 0x0E
#define RTC_STATUS 0x0F
#define RTC_AGING 0x10
#define RTC_TEMP_MSB 0x11
#define RTC_TEMP_LSB 0x12
#define SRAM_START_ADDR 0x14    // first SRAM address
#define SRAM_SIZE 236           // number of bytes of SRAM

// Alarm mask bits
#define A1M1 7
#define A1M2 7
#define A1M3 7
#define A1M4 7
#define A2M2 7
#define A2M3 7
#define A2M4 7

// Control register bits
#define EOSC 7
#define BBSQW 6
#define CONV 5
#define RS2 4
#define RS1 3
#define INTCN 2
#define A2IE 1
#define A1IE 0

// Status register bits
#define OSF 7
#define BB32KHZ 6
#define CRATE1 5
#define CRATE0 4
#define EN32KHZ 3
#define BSY 2
#define A2F 1
#define A1F 0

// Other
#define DS1307_CH 7                // for DS1307 compatibility, Clock Halt bit in Seconds register
#define HR1224 6                   // Hours register 12 or 24 hour mode (24 hour mode==0)
#define CENTURY 7                  // Century bit in Month register
#define DYDT 6                     // Day/Date flag bit in alarm Day/Date registers

// Alarm masks
enum ALARM_TYPES_t {
    ALM1_EVERY_SECOND = 0x0F,
    ALM1_MATCH_SECONDS = 0x0E,
    ALM1_MATCH_MINUTES = 0x0C,     // match minutes *and* seconds
    ALM1_MATCH_HOURS = 0x08,       // match hours *and* minutes, seconds
    ALM1_MATCH_DATE = 0x00,        // match date *and* hours, minutes, seconds
    ALM1_MATCH_DAY = 0x10,         // match day *and* hours, minutes, seconds
    ALM2_EVERY_MINUTE = 0x8E,
    ALM2_MATCH_MINUTES = 0x8C,     // match minutes
    ALM2_MATCH_HOURS = 0x88,       // match hours *and* minutes
    ALM2_MATCH_DATE = 0x80,        // match date *and* hours, minutes
    ALM2_MATCH_DAY = 0x90,         // match day *and* hours, minutes
};

// Square-wave output frequency (TS2, RS1 bits)
enum SQWAVE_FREQS_t {
    SQWAVE_1_HZ,
    SQWAVE_1024_HZ,
    SQWAVE_4096_HZ,
    SQWAVE_8192_HZ,
    SQWAVE_NONE
};

#define ALARM_1 1                  // constants for alarm functions
#define ALARM_2 2


// Convenience macros to convert to and from tm years 
#define  tmYearToCalendar(Y) ((Y) + 1970)  // full four digit year 
#define  CalendarYrToTm(Y)   ((Y) - 1970)
#define  tmYearToY2k(Y)      ((Y) - 30)    // offset is from 2000
#define  y2kYearToTm(Y)      ((Y) + 30)


//
// Time structures
//
typedef enum {
    tmSecond, tmMinute, tmHour, tmWday, tmDay,tmMonth, tmYear, tmNbrFields
} tmByteFields;	 

typedef struct  {
	uint16_t Millisecond;  // 0 - 999
	uint8_t Second;        // 0 - 59 
	uint8_t Minute;        // 0 - 59
	uint8_t Hour;          // 0 - 23
	uint8_t Wday;          // 1 - 7; day of week, sunday is day 1
	uint8_t Day;           // 1 - 31
	uint8_t Month;         // 1 - 12
	uint8_t Year;          // offset from 1970; 
} tmElements_t;



//
// RTC API
//

time_t get_rtc_time_secs();
int set_rtc_time_secs(time_t t);
int read_rtc_time(tmElements_t* tm);
int write_rtc_time(tmElements_t tm);
int write_rtc_bytes(uint8_t *values, uint8_t nBytes);
int write_rtc_byte(uint8_t addr, uint8_t value);
int read_rtc_bytes(uint8_t addr, uint8_t *values, uint8_t nBytes);
int read_rtc_byte(uint8_t addr, uint8_t *value);
void set_rtc_alarm_secs(enum ALARM_TYPES_t alarmType, uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t daydate);
void set_rtc_alarm(enum ALARM_TYPES_t alarmType, uint8_t minutes, uint8_t hours, uint8_t daydate);
void set_rtc_alarm_interrupt(uint8_t alarmNumber, bool alarmEnabled);
bool is_rtc_alarm(uint8_t alarmNumber);
void set_rtc_squareWave(enum SQWAVE_FREQS_t freq);
bool get_rtc_osc_stopped(bool clearOSF);
int16_t get_rtc_temperature();

void rtc_breakTime(time_t time, tmElements_t* tm);
time_t rtc_makeTime(const tmElements_t tm);



#endif /* DS3232_H */
