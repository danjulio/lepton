/********************************************************************
 System Main Module Header
 *******************************************************************/

#ifndef SYSTEM_H
#define SYSTEM_H

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#include "fixed_address_memory.h"


/*** Components of the 32-bit response for the Version Command ******/
// Board Type - Bits 23:16 - Designed to identify different boards that might
// present a similar or related command interface
#define BOARD_TYPE 0x02
#define BOARD_TYPE_MASK 0x00020000


// Firmware version (must be BCD encoded for use in the USB Descriptor)
//   Major - Bits 15:8 - functionality changes
//   Minor - Bits  7:0 - bug fixes
#define FW_MAJOR 0x01
#define FW_MAJOR_MASK  0x00000100
#define FW_MINOR 0x00
#define FW_MINOR_MASK  0x00000000



// Power up reasons
#define SYSTEM_PWRUP_ALARM    0
#define SYSTEM_PWRUP_BUTTON   1
#define SYSTEM_PWRUP_RESTART  2
#define SYSTEM_PWRUP_USB_PWR  3


// Watchdog reset powerdown period (in seconds)
#define SYSTEM_WATCHDOG_PWRDN_TIME 5

// Beaglebone power-down watchdog timeout (in seconds)
// We kill the power if the beaglebone hasn't shut down its 3.3V output in this period
#define SYSTEM_BB_PWRDN_MAX_TIME 30


/*** System States **************************************************/
typedef enum
{
    SYSTEM_STATE_POWERON,
    SYSTEM_STATE_WAKEUP,
    SYSTEM_STATE_POWERUP,
    SYSTEM_STATE_SLEEP
} SYSTEM_STATE;


/*** Externs */
extern uint8_t PowerDownCount;
extern uint8_t sysPowerUpReason;
extern bit PowerEnabled;
extern bit PowerRestartEnable;
extern uint16_t sysWatchdogCount;
extern volatile bit sysBoostEnabled;
extern bit sysUsbEnabled;



/*********************************************************************
* Function: void SYSTEM_Initialize( SYSTEM_STATE state )
*
* Overview: Initializes the system.
*
* PreCondition: None
*
* Input:  SYSTEM_STATE - the state to initialize the system into
*
* Output: None
*
********************************************************************/
void SYSTEM_Initialize(SYSTEM_STATE state);


/*********************************************************************
* Function: void SYSTEM_Tasks(void)
*
* Overview: Runs system level tasks that keep the system running
*
* PreCondition: System has been initalized with SYSTEM_Initialize()
*
* Input: None
*
* Output: None
*
********************************************************************/
void SYSTEM_Tasks(void);

void SYSTEM_PowerUp(void);

void SYSTEM_PowerDown(void);

void SYSTEM_BoostEnable(void);

void SYSTEM_BoostDisable(void);

#define SYSTEM_SchedulePowerDown(t) (PowerDownCount = t)
#define SYSTEM_GetPowerDownTime() (PowerDownCount)
#define SYSTEM_GetPowerUpReason() (sysPowerUpReason)
#define SYSTEM_SystemPowered() (PowerEnabled == 1)
#define SYSTEM_SetRestartEnable(e) (PowerRestartEnable = (e ? 1 : 0))
#define SYSTEM_GetRestartEnable() ((PowerRestartEnable == 1) ? 1 : 0)
#define SYSTEM_SetWatchdogTimeout(t) (sysWatchdogCount = t)
#define SYSTEM_GetWatchdogTimeout() (sysWatchdogCount)
#define SYSTEM_BoostEnabled() (sysBoostEnabled == 1)


#endif //SYSTEM_H
