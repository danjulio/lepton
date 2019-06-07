/********************************************************************
 * Beaglebone control program for Rev 01 and 02 boards (with USB Power
 * Bypass).  Modification of Pi Platter firmware.
 * 
 * Beaglebone connections to Pi Platter hardware
 *   - Beaglebone VI (USB Pwr) to Pi Platter USB V+
 *   - Beaglebone VIN (AC Pwr) to Pi Platter 5V
 *   - Beaglebone GND to Pi Platter Ground
 *   - USB
 *     - Beaglebone USB1 to Pi Platter D-/D+
 *     - Beaglebone ID grounded
 *  - Beaglebone +3.3V through 10 k resistor to Pi Platter A2
 * - Beaglebone PB to Pi Platter PWM1
 * 
 * PWM2 drives active-high Charge LED.
 * 
 * Copyright 2016-2019 (c) danjuliodesigns, LLC.
 * All Rights Reserved
 *
 *******************************************************************/

/** INCLUDES *******************************************************/
#include "system.h"
#include "system_config.h"

#include "cmd.h"

#include "usb.h"
#include "usb_device.h"
#include "usb_device_cdc.h"



void main(void)
{
    // We initialize enough to go to sleep on power-up or a hard reset
    SYSTEM_Initialize(SYSTEM_STATE_POWERON);
    SYSTEM_Initialize(SYSTEM_STATE_SLEEP);
    SLEEP();  // Control halts here until a wake condition

    // Start the main loop after first wake-up
    SYSTEM_Initialize(SYSTEM_STATE_WAKEUP);
  
    while(1) {
        // SYSTEM_Tasks() should be executed first and include code that always
        // runs
        SYSTEM_Tasks();

        if (SYSTEM_SystemPowered()) {
            // Evaluate polled USB tasks
            USBDeviceTasks();

            // Evaluate the command processor if we are actively connected to a host
            if ((USBGetDeviceState() == CONFIGURED_STATE) &&
                    (USBIsDeviceSuspended() == false)) {
                CMD_Tasks();
            }
        }
    }
}




bool USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, uint16_t size)
{
    switch( (int) event )
    {
        case EVENT_TRANSFER:
            break;

        case EVENT_SOF:
            break;

        case EVENT_SUSPEND:
            break;

        case EVENT_RESUME:
            break;

        case EVENT_CONFIGURED:
            // Initialize the command processor when the device is configured
            CMD_Initialize();
            break;

        case EVENT_SET_DESCRIPTOR:
            break;

        case EVENT_EP0_REQUEST:
            /* We have received a non-standard USB request.  The HID driver
             * needs to check to see if the request was for it. */
            USBCheckCDCRequest();
            break;

        case EVENT_BUS_ERROR:
            break;

        case EVENT_TRANSFER_TERMINATED:
            break;

        default:
            break;
    }
    return true;
}
