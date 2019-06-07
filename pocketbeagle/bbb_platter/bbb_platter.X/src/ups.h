/* 
 * USB Power Switching Module header
 */

#ifndef UPS_H
#define	UPS_H

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

// USB Power-related events
#define USB_PWR_IDLE  0
#define USB_PWR_AVAIL 1
#define USB_PWR_FAIL  2
#define USB_PWR_DSCH  3


// Number of fast-tick task evaluations required to wait to verify a power event
#define UPS_TIMER_20MS 2
#define UPS_TIMER_100MS 6


// Externals
extern volatile uint8_t upsEventTimer;
extern volatile uint8_t upsPowerEvent;
extern bit sysUsbEnabled;


// API
void UPS_Initialize();
void UPS_Tasks();
void UPS_SetupWake();
void UPS_SetupPowerup();
void UPS_SetupSleep();

uint8_t UPS_USBPowerAvailable();
void UPS_BypassEnable();
void UPS_BypassDisable();

#define UPS_InitEventTimer(t) (upsEventTimer = t)
#define UPS_SetPowerEvent(e) (upsPowerEvent = e)
#define UPS_UsbPowerEnabled() (sysUsbEnabled == 1)
#endif	/* UPS_H */

