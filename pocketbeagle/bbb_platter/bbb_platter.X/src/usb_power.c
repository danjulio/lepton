/*
 * USB Port Power Enable Module 
 */
#include "usb_power.h"
#include "cmd.h"
#include "eeprom.h"
#include "system_config.h"


// Enable flags
bit USBPWR_En1;
bit USBPWR_En2;

// Fault detection state
bit USBPWR_CurFault;
bit USBPWR_FaultDetected;
bit USBPWR_PrevFault;

// Fault message enable
bit USBPWR_FaultWarnMsgEn;


// API
void USBPWR_Initialize()
{
    // Configure our IO
    USB_FLT_N_TRIS = 1;  // Fault input
    USB_PWR_1_TRIS = 0;  // USB Power outputs initialized off
    USB_PWR_2_TRIS = 0;
    USB_PWR_1_ANSEL = 0; // Digital
    USB_PWR_2_ANSEL = 0;
    USB_PWR_1_PIN = 0;
    USB_PWR_2_PIN = 0;
}


void USBPWR_SetupPowerup()
{
    // Reinitialize IO prots
    USBPWR_Initialize();

    // Initialize variables
    USBPWR_FaultDetected = 0;
    USBPWR_PrevFault = 0;

    // Configure default fault messages
    USBPWR_FaultWarnMsgEn = (EEP_GetUsbFaultMsgEn()) ? 1 : 0;

    // Initialize our ports
    USB_FLT_N_WPU = 1;   // Fault input pull-up enabled
    USBPWR_SetEnable(1, EEP_GetUsbPowerEn(1));
    USBPWR_SetEnable(2, EEP_GetUsbPowerEn(2));
}


void USBPWR_SetupSleep()
{
    // Set power enables to low
    USB_FLT_N_WPU = 0;   // Fault input pull-up disabled
    USBPWR_SetEnable(1, 0);
    USBPWR_SetEnable(2, 0);
}


// Designed to be called every 10 or 20 mSec to debounce fault input
void USBPWR_Tasks()
{
    // Sample fault input
    USBPWR_CurFault = ~USB_FLT_N_PIN;

    // Detect initial fault indication
    if ((USBPWR_CurFault == 1) && (USBPWR_PrevFault == 1) && (USBPWR_FaultDetected == 0)) {
        if (USBPWR_FaultWarnMsgEn == 1) {
            CMD_SendWarnMsg(CMD_WARN_USB_FAULT);
        }
        USBPWR_FaultDetected = 1;
    }
    if ((USBPWR_CurFault == 0) && (USBPWR_PrevFault == 0)) {
        USBPWR_FaultDetected = 0;
    }
    USBPWR_PrevFault = USBPWR_CurFault;
}


void USBPWR_SetEnable(uint8_t c, uint8_t v)
{
    switch (c) {
        case 1:
            USBPWR_En1 = (v == 0) ? 0 : 1;
            USB_PWR_1_PIN = USBPWR_En1;
            break;
        case 2:
            USBPWR_En2 = (v == 0) ? 0 : 1;
            USB_PWR_2_PIN = USBPWR_En2;
            break;
    }
}


uint8_t USBPWR_GetEnable(uint8_t c)
{
    uint8_t t = 0;

    switch (c) {
        case 1:
            t = (USBPWR_En1 == 1) ? 1 : 0;
            break;
        case 2:
            t = (USBPWR_En2 == 1) ? 1 : 0;
            break;
    }

    return(t);
}

