/*
 * USB Port Power Enable Module header
 */
#ifndef USB_POWER_H
#define USB_POWER_H

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

// Default USB Power Fault Message Enable
#define USBPWR_FAULT_MSG_EN  1


// Number of USB enables
#define USBPWR_NUM        2


// Default Power Enable values
#define USBPWR_DEF_EN_1   1
#define USBPWR_DEF_EN_2   1


// Externs
extern bit USBPWR_FaultWarnMsgEn;
extern bit USBPWR_FaultDetected;


// API
void USBPWR_Initialize();
void USBPWR_SetupPowerup();
void USBPWR_SetupSleep();
void USBPWR_Tasks();
void USBPWR_SetEnable(uint8_t c, uint8_t v);
uint8_t USBPWR_GetEnable(uint8_t c);

// API Macros
#define USBPWR_FaultDetected() (USBPWR_FaultDetected == 1)
#define USBPWR_SetFaultWarnEn(e) (USBPWR_FaultWarnMsgEn = (e == 0) ? 0 : 1)
#define USBPWR_GetFaultWarnEn()  ((USBPWR_FaultWarnMsgEn == 1) ? 1 : 0)


#endif // USB_POWER_H
