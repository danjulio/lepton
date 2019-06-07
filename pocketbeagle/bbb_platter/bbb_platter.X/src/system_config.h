#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include "usb_config.h"


// Analog input port/pins
#define AN1_TRIS TRISCbits.TRISC0
#define AN1_ANSEL ANSELCbits.ANSC0
#define AN1_CH 4
#define BATT_TRIS TRISCbits.TRISC3
#define BATT_ANSEL ANSELCbits.ANSC3
#define BATT_CH 7

// Button port/pin
#define BUTTON_PIN PORTAbits.RA3
// RA3 has no TRIS bit - it is input only
#define BUTTON_IOCP IOCAPbits.IOCAP3
#define BUTTON_IOCN IOCANbits.IOCAN3
#define BUTTON_IOCF IOCAFbits.IOCAF3

// Beaglebone power button control output
#define BEAGLE_PWR_BUT_PIN LATCbits.LATC5
#define BEAGLE_PWR_BUT_TRIS TRISCbits.TRISC5

// Beaglebone 3.3V sense input
#define BEAGLE_PWR_SNS_PIN PORTCbits.RC1
#define BEAGLE_PWR_SNS_TRIS TRISCbits.TRISC1
#define BEAGLE_PWR_SNS_ANSEL ANSELCbits.ANSC1

// Charge Status input port/pin
#define CHARGE_N_PIN PORTCbits.RC4
#define CHARGE_N_TRIS TRISCbits.TRISC4

// Charge Status LED port/pin
#define CHARGE_LED_PIN LATCbits.LATC6
#define CHARGE_LED_TRIS TRISCbits.TRISC6

// USB Power Input sense (C1IN2-)
#define USB_PWR_SNS_PIN PORTCbits.RC2
#define USB_PWR_SNS_TRIS TRISCbits.TRISC2
#define USB_PWR_SNS_ANSEL ANSELCbits.ANSC2

// Boost Converter Power Enable port/pin
#define PWREN_PIN LATBbits.LATB7
#define PWREN_TRIS TRISBbits.TRISB7

// USB Power Bypass Enable port/pin
#define USB_PWREN_PIN LATCbits.LATC7
#define USB_PWREN_TRIS TRISCbits.TRISC7
#define USB_PWREN_ANSEL ANSELCbits.ANSC7

// USB power enable and fault detection port/pins
#define USB_FLT_N_PIN PORTBbits.RB6
#define USB_FLT_N_TRIS TRISBbits.TRISB6
#define USB_FLT_N_WPU WPUBbits.WPUB6
#define USB_PWR_1_PIN LATBbits.LATB5
#define USB_PWR_1_TRIS TRISBbits.TRISB5
#define USB_PWR_1_ANSEL ANSELBbits.ANSB5
#define USB_PWR_2_PIN LATBbits.LATB4
#define USB_PWR_2_TRIS TRISBbits.TRISB4
#define USB_PWR_2_ANSEL ANSELBbits.ANSB4


#endif // SYSTEM_CONFIG_H