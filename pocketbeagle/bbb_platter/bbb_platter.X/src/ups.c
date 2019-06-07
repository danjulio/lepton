/* 
 * USB Power Switching Module header
 */
#include "ups.h"
#include "system.h"
#include "system_config.h"


// Variables
volatile uint8_t upsPowerEvent;  // Indicates change with the USB power input
volatile uint8_t upsEventTimer;  // Task eval count to validate changes
bit sysUsbEnabled;               // Set when power from USB


// API Routines
void UPS_Initialize()
{
    // Initialize the comparator input
    USB_PWR_SNS_TRIS = 1;
    USB_PWR_SNS_ANSEL = 0;
    
    // Initialize the USB Bypass Enable output
    USB_PWREN_TRIS = 0;
    USB_PWREN_ANSEL = 0;
    UPS_BypassDisable();
}


// Should be called on fast-tick
void UPS_Tasks()
{
    // Look for USB Power becoming available
    if ((upsPowerEvent == USB_PWR_IDLE) && (UPS_USBPowerAvailable()) &&
        (SYSTEM_BoostEnabled())) {
        // Setup possible switch to USB Power
        UPS_SetPowerEvent(USB_PWR_AVAIL);
        UPS_InitEventTimer(UPS_TIMER_100MS);
    }
    
    // Event Timer is used to verify that a condition legitimately exists
    if (upsEventTimer != 0) {
        if (--upsEventTimer == 0) {
            switch (upsPowerEvent) {
                case USB_PWR_FAIL:
                    // Triggered by the ISR edge detector because we think we
                    // saw USB Power fail and switched on the Boost converter.
                    // Disable the bypass and then setup to wait before allowing
                    // ourself to check USB power to let the USB side voltage
                    // decay
                    UPS_BypassDisable();
                    UPS_SetPowerEvent(USB_PWR_DSCH);
                    UPS_InitEventTimer(UPS_TIMER_100MS);
                    break;

                case USB_PWR_AVAIL:
                    // We detected USB Power available again, verify it's still present
                    if (UPS_USBPowerAvailable()) {
                        // Still available so assume that it's good and we can switch
                        // on the bypass and then switch off the boost converter
                        UPS_BypassEnable();
                    
                        // Short delay to allow the bypass transistor to switch on
                        // before turning off the boost converter:
                        //   10 uSec @ 83.3 nSec/cycle = 120 cycles
                        _delay(120);
                        SYSTEM_BoostDisable();
                    }
                    UPS_SetPowerEvent(USB_PWR_IDLE);
                    break;

                case USB_PWR_DSCH:
                    // Done waiting for the voltage on the USB power input side
                    // to have decayed enough so we don't accidently switch
                    // back to USB.
                    UPS_SetPowerEvent(USB_PWR_IDLE);
                    break;
            }
        }
    }
}



void UPS_SetupWake()
{
    // Re-initialize the ports (just to be safe)
    UPS_Initialize();

    // (Re)initialize the comparator
    CM1CON0 = 0x86;   // Comparator on, non-inverted, internal, normal w/ hysteresis
    CM1CON1 = 0xA2;   // Pos edge, + input = FVR, - input = C1IN2-
    
    // Clear the interrupt bit (which may be set when the comparator is enabled)
    PIR2bits.C1IF = 0;
    
    // Enable the FVR
    FVRCONbits.FVREN = 1;
    FVRCONbits.CDAFVR = 0x02;  // 2.048 volts
    
    // Setup state
    upsPowerEvent = USB_PWR_IDLE;
    sysUsbEnabled = 0;
}


void UPS_SetupPowerup()
{
    // Enable comparator interrupts
    PIR2bits.C1IF = 0;
    PIE2bits.C1IE = 1;
    INTCONbits.PEIE = 1;
}


void UPS_SetupSleep()
{
    // Disable interrupts
    PIE2bits.C1IE = 0;
    
    // Disable the comparator
    CM1CON0bits.C1ON = 0;
    
    // Disable the FVR
    FVRCONbits.FVREN = 0;
}


uint8_t UPS_USBPowerAvailable()
{
    // Comparator output low indicates USB Power (C1- > C1+)
    if (CMOUTbits.MC1OUT == 0) {
        return 1;
    } else {
        return 0;
    }
}


void UPS_BypassEnable()
{
    USB_PWREN_PIN = 1;
    sysUsbEnabled = 1;
}


void UPS_BypassDisable()
{
    USB_PWREN_PIN = 0;
    sysUsbEnabled = 0;
}
