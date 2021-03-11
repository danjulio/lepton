/*
 * Time related utilities
 *
 * Contains functions to interface the RTC to the system timekeeping
 * capabilities and provide application access to the system time.
 *
 * Copyright 2020 Dan Julio
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
#include "time_utilities.h"
#include "esp_system.h"
#include "esp_log.h"
#include "system_config.h"
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>


//
// Time Utilities constants
//
// Leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+(Y))>0) && !((1970+(Y))%4) && ( ((1970+(Y))%100) || !((1970+(Y))%400) ) )

// Useful time constants
#define SECS_PER_MIN  ((time_t)(60UL))
#define SECS_PER_HOUR ((time_t)(3600UL))
#define SECS_PER_DAY  ((time_t)(SECS_PER_HOUR * 24UL))
#define DAYS_PER_WEEK ((time_t)(7UL))
#define SECS_PER_WEEK ((time_t)(SECS_PER_DAY * DAYS_PER_WEEK))
#define SECS_PER_YEAR ((time_t)(SECS_PER_DAY * 365UL)) // TODO: ought to handle leap years
#define SECS_YR_2000  ((time_t)(946684800UL)) // the time at the start of y2k



//
// Time Utilities date related strings (related to tmElements)
//
static const char* day_strings[] = {
	"Err",
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat"
};

static const char* mon_strings[] = {
	"Err",
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};

 // API starts months from 1, this array starts from 0
static const uint8_t monthDays[] = {31,28,31,30,31,30,31,31,30,31,30,31};


//
// Time Utilities Variables
//
static const char* TAG = "time_utilities";


//
// Forward declarations for internal functions
//
time_t rtc_makeTime(const tmElements_t tm);


 
//
// Time Utilities API
//

/**
 * Initialize system time from the RTC
 */
void time_init()
{
	char buf[26];
	tmElements_t te;
	struct timeval tv;
	time_t secs;
	
	// Set the system time from the RTC
#ifdef HAS_HW_RTC
	secs = get_rtc_time_secs();
#else
	secs = 0;
#endif
	tv.tv_sec = secs;
	tv.tv_usec = 0;
	settimeofday((const struct timeval *) &tv, NULL);
	
	// Diagnostic display of time
	time_get(&te);
	time_get_disp_string(te, buf);
	ESP_LOGI(TAG, "Initial RTC time: %s", buf);
}


/**
 * Set the system time and update the RTC
 */
void time_set(tmElements_t te)
{
	struct timeval tv;
	time_t secs;

#ifdef HAS_HW_RTC
	// Attempt to set the RTC
	if (write_rtc_time(te) != 0) {
		ESP_LOGE(TAG, "Update RTC failed");
	}
#endif
	
	// Set the system time
	secs = rtc_makeTime(te);
	tv.tv_sec = secs;
	tv.tv_usec = 0;
	settimeofday((const struct timeval *) &tv, NULL);
}


/**
 * Get the system time
 */
void time_get(tmElements_t* te)
{
	time_t now;
	struct timeval tv;
	struct tm timeinfo;
	
	// Get the time and convert into our simplified tmElements format
	(void) gettimeofday(&tv, NULL);
	//time(&now);
	now = tv.tv_sec;
    localtime_r(&now, &timeinfo);  // Get the unix formatted timeinfo
    mktime(&timeinfo);             // Fill in the DOW and DOY fields
    te->Millisecond = (uint16_t) (tv.tv_usec / 1000);
    te->Second = (uint8_t) timeinfo.tm_sec;
    te->Minute = (uint8_t) timeinfo.tm_min;
    te->Hour = (uint8_t) timeinfo.tm_hour;
    te->Wday = (uint8_t) timeinfo.tm_wday + 1; // Sunday is 1 in our tmElements structure
    te->Day = (uint8_t) timeinfo.tm_mday;
    te->Month = (uint8_t) timeinfo.tm_mon + 1; // January is 1 in our tmElements structure
    te->Year = (uint8_t) timeinfo.tm_year - 70; // tmElements starts at 1970
}


/**
 * Return true if the system time (in seconds) has changed from the last time
 * this function returned true. Each calling task must maintain its own prev_time
 * variable (it can initialize it to 0).  Set te to NULL if you don't need the time.
 */
bool time_changed(tmElements_t* te, time_t* prev_time)
{
	time_t now;
	struct tm timeinfo;
	
	// Get the time and check if it is different
	time(&now);
	if (now != *prev_time) {
		*prev_time = now;
		
		if (te != NULL) {
			// convert time into our simplified tmElements format
    		localtime_r(&now, &timeinfo);  // Get the unix formatted timeinfo
    		mktime(&timeinfo);             // Fill in the DOW and DOY fields
    		te->Millisecond = 0;
    		te->Second = (uint8_t) timeinfo.tm_sec;
    		te->Minute = (uint8_t) timeinfo.tm_min;
    		te->Hour = (uint8_t) timeinfo.tm_hour;
    		te->Wday = (uint8_t) timeinfo.tm_wday + 1; // Sunday is 1 in our tmElements structure
    		te->Day = (uint8_t) timeinfo.tm_mday;
    		te->Month = (uint8_t) timeinfo.tm_mon + 1; // January is 1 in our tmElements structure
    		te->Year = (uint8_t) timeinfo.tm_year - 70; // tmElements starts at 1970
    	}
    	
    	return true;
    } else {
    	return false;
    }
}


/**
 * Load buf with a time & date string for display.
 *
 *   "DOW MON DAY HH:MM:SS YEAR"
 *
 * buf must be at least 26 bytes long (to include null termination).
 */
void time_get_disp_string(tmElements_t te, char* buf)
{
	// Validate te to prevent illegal accesses to the constant string buffers
	if (te.Wday > 7) te.Wday = 0;
	if (te.Month > 12) te.Month = 0;
	
	// Build up the string
	sprintf(buf,"%s %s %2d %2d:%02d:%02d %4d", 
		day_strings[te.Wday],
		mon_strings[te.Month],
		te.Day,
		te.Hour,
		te.Minute,
		te.Second,
		te.Year + 1970);
}


/**
 * Load buf with a short time & date string with no spaces.  Useful for constructing
 * a file or directory name.
 *
 *   "YY_MM_DD_HH_MM_SS"
 *
 * buf must be at least 18 bytes long (to include null termination)
 */
void time_get_short_string(tmElements_t te, char* buf)
{
	// Always assume we are running post 2000
	te.Year = tmYearToY2k(te.Year);
	
	// Build up the string
	sprintf(buf, "%2d_%02d_%02d_%02d_%02d_%02d",
		te.Year, te.Month, te.Day, te.Hour, te.Minute, te.Second);
}


/**
 * Load buf with hours & minutes string for display
 *
 *   "HH:MM"
 *
 * buf must be at least 6 bytes long (to include null termination)
 */
void time_get_hhmm_string(tmElements_t te, char* buf)
{
	sprintf(buf, "%02d:%02d", te.Hour, te.Minute);
}



//
// Internal functions
//
time_t rtc_makeTime(const tmElements_t tm)
{  
	int i;
	uint32_t seconds;

	// seconds from 1970 till 1 jan 00:00:00 of the given year
	seconds= tm.Year*(SECS_PER_DAY * 365);
	for (i = 0; i < tm.Year; i++) {
		if (LEAP_YEAR(i)) {
			seconds +=  SECS_PER_DAY;   // add extra days for leap years
		}
	}
  
	// add days for this year, months start from 1
	for (i = 1; i < tm.Month; i++) {
		if ( (i == 2) && LEAP_YEAR(tm.Year)) { 
			seconds += SECS_PER_DAY * 29;
		} else {
			seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
		}
	}
	seconds+= (tm.Day-1) * SECS_PER_DAY;
	seconds+= tm.Hour * SECS_PER_HOUR;
	seconds+= tm.Minute * SECS_PER_MIN;
	seconds+= tm.Second;
	return (time_t)seconds; 
}

