/********************************************************************
 System Main Module
 *******************************************************************/

#include "system.h"
#include "system_config.h"
#include "adc.h"
#include "batt.h"
#include "button.h"
#include "chg_status.h"
#include "eeprom.h"
#include "rtc.h"
#include "ups.h"
#include "usb_power.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_cdc.h"



/** CONFIGURATION Bits **********************************************/
// PIC16F1459 configuration bit settings:
    // CONFIG1
    #pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
    #pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
    #pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
    #pragma config MCLRE = OFF      // MCLR Pin Function Select (MCLR/VPP pin function is digital input)
    #pragma config CP = ON          // Flash Program Memory Code Protection (Program memory code protection is disabled)
    #pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
    #pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
    #pragma config IESO = OFF       // Internal/External Switchover Mode (Internal/External Switchover Mode is disabled)
    #pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

    // CONFIG2
    #pragma config WRT = HALF       // Flash Memory Self-Write Protection (Write protection off)
    #pragma config CPUDIV = NOCLKDIV// CPU System Clock Selection Bit (NO CPU system divide)
    #pragma config USBLSCLK = 48MHz // USB Low Speed Clock Selection bit (System clock expects 48 MHz, FS/LS USB CLKENs divide-by is set to 8.)
    #pragma config PLLMULT = 3x     // PLL Multipler Selection Bit (3x Output Frequency Selected)
    #pragma config PLLEN = ENABLED  // PLL Enable Bit (3x or 4x PLL Enabled)
    #pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
    #pragma config BORV = HI        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), hi trip point selected.)
    #pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
    #pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)


/*** Forward Declarations for internal functions ************/
void SYS_Initialize(void);
void SYS_SetupWake(void);
void SYS_SetupPowerOn(void);
void SYS_SetupSleep(void);
void SYS_EvalTimer0(void);
void SYS_PowerEnable(uint8_t en);
void SYS_SetBeaglePowerButton(bool en);



/*** SYSTEM Evaluation State *********************************/
#define SYS_ST_WAKEUP_INIT      0
#define SYS_ST_WAKEUP_WAIT_BP   1
#define SYS_ST_WAKEUP_WAIT_BR   2
#define SYS_ST_NORMAL_OP        3
#define SYS_ST_NORMAL_WAIT_BR   4
#define SYS_ST_NORMAL_TGL_BGL   5
#define SYS_ST_NORMAL_WAIT_BGL  6
#define SYS_ST_NORMAL_HARD_OFF  7
#define SYS_ST_GOTO_SLEEP       8
uint8_t sysState;


/*** TIMER0 Related *****************************************/
// FOSC/4 = 12 MHz, Prescale 256:1 ==> 21.333 uS/count  ==> 234 counts for 4.992 mSec
#define TMR0_TO         (256-234)
// Fast tick is approx 20 mSec
#define TMR0_MSEC       5
#define FAST_TICK_MSEC  20
#define TMR0_SEC_COUNT  (1000 / TMR0_MSEC)

volatile bit TMR0tick;   // Set in the ISR, consumed by SYSTEM_Tasks
bit FastTick;   // Set in SYSTEM_Tasks for consumption in the main loop
bit SecTick;  // Set in SYSTEM_Tasks for consumption in main loop
uint8_t FastTickCount;
uint8_t SecTickCount;


/*** Power Control Related **********************************/
#define BUTTON_OFF_TIMEOUT (2000 / FAST_TICK_MSEC)
#define BUTTON_ON_TIMEOUT  (2000 / FAST_TICK_MSEC)
#define BEAGLE_NORM_BUT_PRESS (60 / FAST_TICK_MSEC)
#define BEAGLE_HARDOFF_BUT_PRESS (8500 / FAST_TICK_MSEC)
bit PowerEnabled;
uint8_t PowerDownCount;
bit PowerRestartEnable;

volatile bit beagle3v3good;
volatile bit prevBeagle3v3good;


/*** State Related */
bit sysCritBattAtWake;      // Temporary for use in SYSTEM_Tasks
uint8_t sysPowerUpReason;

volatile bit sysBoostEnabled;    // Set when power from Boost


/*** Watchdog timer */
uint16_t sysStepCount = 0;



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
void SYSTEM_Initialize( SYSTEM_STATE state )
{
    switch(state) {
        case SYSTEM_STATE_POWERON:
            SYS_Initialize();
            ADC_Initialize();
            CHGS_Initialize();
            RTC_Initialize();
            UPS_Initialize();
            USBPWR_Initialize();
            break;

        case SYSTEM_STATE_WAKEUP:
            SYS_SetupWake();
            ADC_SetupWake();
            BUTTON_SetupWake();
            UPS_SetupWake();
            break;
            
        case SYSTEM_STATE_POWERUP:
            BATT_SetupPowerup();
            SYS_SetupPowerOn();
            RTC_SetupPowerup();
            UPS_SetupPowerup();
            USBPWR_SetupPowerup();
            break;
            
        case SYSTEM_STATE_SLEEP:
            SYS_SetupSleep();
            ADC_SetupSleep();
            BUTTON_SetupSleep();
            RTC_SetupSleep();
            UPS_SetupSleep();
            USBPWR_SetupSleep();
            break;
    }
}


/*********************************************************************
* Function: void SYSTEM_Tasks(void)
*
* Overview: Runs system level tasks that keep the system running
*
* PreCondition: System has been initialized with SYSTEM_Initialize()
*
* Input: None
*
* Output: None
*
********************************************************************/
void SYSTEM_Tasks(void)
{
    // Evaluate our tick timer first to set timing flags to be used during
    // this evaluation cycle
    SYS_EvalTimer0();

    switch (sysState) {
        case SYS_ST_WAKEUP_INIT:
            // Start by checking our battery
            BATT_ReadBatteryInternal();
            sysCritBattAtWake = BATT_UndervoltageDetected() ? 1 : 0;

            // Then look for conditions to power-up or go back to sleep
            if (RTC_SawAlarm()) {
                if (sysCritBattAtWake == 1) {
                    // Battery too low to power up so go back to sleep
                    sysState = SYS_ST_GOTO_SLEEP;
                } else {
                    // Battery OK to power up for alarm
                    sysPowerUpReason = SYSTEM_PWRUP_ALARM;
                    SYSTEM_PowerUp();
                }
            } else if (BUTTON_Down()) {
                if (sysCritBattAtWake == 1) {
                    // Battery too low so wait for the button to be released before
                    // going back to sleep
                    sysState = SYS_ST_WAKEUP_WAIT_BR;
                } else {
                    // Battery OK so see if they hold it long enough for a full
                    // power up
                    sysState = SYS_ST_WAKEUP_WAIT_BP;
                }
            } else {
                // Must have been only a RTC update so we check a couple of things:
                //   1. If we sense the Beaglebone 3.3V output indicating that it
                //      has been powered via the USB input.
                //   2. If we have been enabled to restart when the battery voltage
                //      reaches the restart threshold.
                // Otherwise just go back to sleep
                if ((prevBeagle3v3good == 0) && (beagle3v3good == 1)) {
                    sysPowerUpReason = SYSTEM_PWRUP_USB_PWR;
                    SYSTEM_PowerUp();
                } else if (PowerRestartEnable == 1) {
                    if (BATT_RestartvoltageDetected()) {
                        sysPowerUpReason = SYSTEM_PWRUP_RESTART;
                        SYSTEM_PowerUp();
                    } else {
                        sysState = SYS_ST_GOTO_SLEEP;
                    }
                } else {
                    sysState = SYS_ST_GOTO_SLEEP;
                }
            }
            break;

        case SYS_ST_WAKEUP_WAIT_BP:
            // Evaluate button state on fast periodic activities
            if (FastTick == 1) {
                BUTTON_Tasks();
                if (BUTTON_Down()) {
                    // We stay awake as long as the button is down
                    if (BUTTON_PressedDetected()) {
                        sysPowerUpReason = SYSTEM_PWRUP_BUTTON;
                        SYSTEM_PowerUp();
                    }
                } else {
                    // Button has been released before we saw a press detection
                    sysState = SYS_ST_GOTO_SLEEP;
                }
            }

            // Also allow the probably rare case where the alarm goes off
            // while the button is pressed
            if (RTC_SawAlarm()) {
                sysPowerUpReason = SYSTEM_PWRUP_ALARM;
                SYSTEM_PowerUp();
            }
            break;

        case SYS_ST_WAKEUP_WAIT_BR:
            // Wait for the button to be released and then go back to sleep
            if (FastTick == 1) {
                BUTTON_Tasks();
                if (!BUTTON_Down()) {
                    sysState = SYS_ST_GOTO_SLEEP;
                }
            }
            break;

        case SYS_ST_NORMAL_OP:
            // Look for case where beaglebone powered itself off (e.g. "shutdown")
            if ((prevBeagle3v3good == 1) && (beagle3v3good == 0)) {
                SYSTEM_PowerDown();
            }
            
            // Evaluate fast tick activities
            else if (FastTick == 1) {
                BUTTON_Tasks();
                if (BUTTON_PressedDetected()) {
                    // Button press while we are active starts a shut-down by
                    // initiating a short "press" of the Beaglebone power button
                    sysState = SYS_ST_NORMAL_TGL_BGL;
                    SYS_SetBeaglePowerButton(true);
                    sysStepCount = BEAGLE_NORM_BUT_PRESS;
                }

                UPS_Tasks();
                USBPWR_Tasks();
            }

            // Evaluate slow tick activities
            if (SecTick == 1) {
                BATT_Tasks();

                // Evaluate power-down if it is in-progress
                if (PowerDownCount != 0) {
                    if (--PowerDownCount == 0) {
                        SYSTEM_PowerDown();
                    }
                }
            }
            break;

        case SYS_ST_NORMAL_WAIT_BR:
            // Wait for the button to be released and then go back to sleep
            if (FastTick == 1) {
                BUTTON_Tasks();
                if (!BUTTON_Down()) {
                    sysState = SYS_ST_GOTO_SLEEP;
                }
            }
            break;

        case SYS_ST_NORMAL_TGL_BGL:
            if (FastTick == 1) {
                if (--sysStepCount == 0) {
                    // Release the Beaglebone power button input
                    SYS_SetBeaglePowerButton(false);
                
                    // Setup to shut down
                    sysState = SYS_ST_NORMAL_WAIT_BGL;
                    sysStepCount = SYSTEM_BB_PWRDN_MAX_TIME;
                }
            }
            break;
            
        case SYS_ST_NORMAL_WAIT_BGL:
            // Wait for the Beaglebone to shut down its 3.3V output or a timeout
            if (SecTick == 1) {
                sysStepCount = sysStepCount - 1;
            }
            if (beagle3v3good == 0) {
                // Beaglebone powered itself off normally, now we can kill all power
                SYSTEM_PowerDown();
                sysState = SYS_ST_NORMAL_WAIT_BR;   // Wait on button release
            } else if (sysStepCount == 0) {
                // Beaglebone didn't power itself off, so force its PMIC to shut it down
                // with a long "press" of its power button
                SYS_SetBeaglePowerButton(true);
                sysState = SYS_ST_NORMAL_HARD_OFF;
                sysStepCount = BEAGLE_HARDOFF_BUT_PRESS;
            }
            break;
            
        case SYS_ST_NORMAL_HARD_OFF:
            if (FastTick == 1) {
                if (--sysStepCount == 0) {
                    // Release the Beaglebone power button input - by this time
                    // its PMIC should have killed power on the board
                    SYS_SetBeaglePowerButton(false);
                
                    // Shut the rest of the system down
                    SYSTEM_PowerDown();
                    sysState = SYS_ST_NORMAL_WAIT_BR;
                }
            }
            break;
            
        case SYS_ST_GOTO_SLEEP:
            SYSTEM_Initialize(SYSTEM_STATE_SLEEP);
            SLEEP();  // Control halts here until a wake condition
            SYSTEM_Initialize(SYSTEM_STATE_WAKEUP);
            break;

        default:
            sysState = SYS_ST_GOTO_SLEEP;
            break;
    }
}


void SYSTEM_PowerUp(void)
{
    sysState = SYS_ST_NORMAL_OP;
    SYS_PowerEnable(1);
    SYSTEM_Initialize(SYSTEM_STATE_POWERUP);
}


void SYSTEM_PowerDown(void)
{
    sysState = SYS_ST_GOTO_SLEEP;
    SYS_PowerEnable(0);
}




void SYSTEM_BoostEnable(void)
{
    PWREN_PIN = 1;
    sysBoostEnabled = 1;
}


void SYSTEM_BoostDisable(void)
{
    PWREN_PIN = 0;
    sysBoostEnabled = 0;
}


/*** Internal Functions  *****************************************/



/*********************************************************************
* Function: void SYS_InterruptHigh(void)
*
* Overview: Handle or dispatch 
*
* PreCondition: System has been initialized with SYSTEM_Initialize()
*
* Input: None
*
* Output: None
*
********************************************************************/
void interrupt SYS_InterruptHigh(void)
{
    // Highest priority - look for USB Power Sense Comparator edge interrupts
    if ((PIE2bits.C1IE == 1) && (PIR2bits.C1IF == 1)) {
        // Rising edge of comparator output when we are in a powered state:
        //   USB Power not good --> Boost ON!!!
        if ((PowerEnabled == 1) && (sysBoostEnabled == 0)) {
            PWREN_PIN = 1;
            sysBoostEnabled = 1;
            UPS_InitEventTimer(UPS_TIMER_20MS);
            UPS_SetPowerEvent(USB_PWR_FAIL);
        }
        PIR2bits.C1IF = 0;
    }

    // TIMER0
    if (INTCONbits.T0IF == 1) {
        // Reload the timer and note we saw a 5 mSec tick
        TMR0 = TMR0_TO;
        INTCONbits.T0IF = 0;   // Clear the interrupt
        TMR0tick = 1;
    }

    // TIMER1 - RTC - always running
    if (PIR1bits.TMR1IF == 1) {
        RTC_ISR_EvaluateState();
        CHGS_UpdateOutput();
        prevBeagle3v3good = beagle3v3good;
        beagle3v3good = BEAGLE_PWR_SNS_PIN;
    }

    // Button IOC input
    if (INTCONbits.IOCIF == 1) {
        BUTTON_SetIocDetected();
        //BUTTON_IOCF = 0;
        // Clear all possible IOC sources, just in case, even though we only
        // expect the one associated with the button
        IOCAF = 0;
        IOCBF = 0;
    }
}


void SYS_Initialize(void)
{
    // Power Enable output
    PWREN_TRIS = 0;
    SYSTEM_BoostDisable();
    UPS_BypassDisable();
    
    // Beaglebone power sense input
    BEAGLE_PWR_SNS_TRIS = 1;
    BEAGLE_PWR_SNS_ANSEL = 0;
    beagle3v3good = 0;
    prevBeagle3v3good = 0;
    
    // Beaglebone power button output
    BEAGLE_PWR_BUT_TRIS = 1;  // Normally tri-stated
    //BEAGLE_PWR_BUT_PIN = 0;   // Output, when driven, is low

    // Configure the processor
    OPTION_REG = 0x07;  // Pre-scaler to TIMER0, Int Clock, 1:256
    INTCONbits.GIE = 1;

    // Turn on active clock tuning for USB full speed operation
    OSCCON = 0xFC;  // HFINTOSC @ 16MHz, 3X PLL, PLL enabled
    
    // Wait for oscillator to be stable @ 48 MHz
    //  (6:PLLRDY, 4:HFIOFR, 0:HFIOFS)
    //while ((OSCSTAT & 0x51) != 0x51);

    // Get Restart Enable from EEPROM when we are initially powered up
    PowerRestartEnable = EEP_GetRestartEn() ? 1 : 0;
}


void SYS_SetupWake(void)
{
    // Configure TIMER0 as our system tick generator
    TMR0 = TMR0_TO;
    SecTickCount = TMR0_SEC_COUNT;

    // Enable TIMER0 interrupts
    INTCONbits.TMR0IE = 1;

    // Setup our state
    sysState = SYS_ST_WAKEUP_INIT;

    // Configure button timeout for "turn-on" detection
    BUTTON_SetPressEvalCount(BUTTON_ON_TIMEOUT);
}


void SYS_SetupPowerOn(void)
{
    // Restart second timer on power-up boundary
    SecTickCount = TMR0_SEC_COUNT;

    // Active clock tuning enabled for USB
    ACTCON = 0x90;

    // Initialize the USB subsystem
    USBDeviceInit();
    USBDeviceAttach();

    // Configure button timeout for "turn-off" detection
    BUTTON_SetPressEvalCount(BUTTON_OFF_TIMEOUT);

    // Reset the Restart Enable from EEPROM when power up the system
    // (allows system software to set a one-time condition)
    PowerRestartEnable = EEP_GetRestartEn() ? 1 : 0;
}


void SYS_SetupSleep(void)
{
    // Disable TIMER0 interrupts
    INTCONbits.TMR0IE = 0;

    // Disable active clock tuning since USB will be gone
    ACTCON = 0x00;

    // Disable USB
    UCONbits.SUSPND = 1;
    UCONbits.USBEN = 0;
    UCFGbits.UPUEN = 0;
}


void SYS_EvalTimer0(void)
{
    // Tick bits will be set if necessary
    FastTick = 0;
    SecTick = 0;

    if (TMR0tick == 1) {
        TMR0tick = 0;
        if ((++FastTickCount & 0x03) == 0x00) {
            FastTick = 1;
        }
        if (--SecTickCount == 0) {
            SecTickCount = TMR0_SEC_COUNT;
            SecTick = 1;
        }
    }
}


void SYS_PowerEnable(uint8_t en)
{
    if (en) {
        PowerDownCount = 0;
        sysStepCount = 0;
        PowerEnabled = 1;
        if (UPS_USBPowerAvailable()) {
            UPS_BypassEnable();
        } else {
            SYSTEM_BoostEnable();
        }
    } else {
        PowerEnabled = 0;
        SYSTEM_BoostDisable();
        UPS_BypassDisable();
    }
}


void SYS_SetBeaglePowerButton(bool en)
{
    if (en) {
        // Drive active-low output
        BEAGLE_PWR_BUT_PIN = 0;
        BEAGLE_PWR_BUT_TRIS = 0;
    } else {
        // Pull-high then tri-state
        BEAGLE_PWR_BUT_PIN = 1;
        BEAGLE_PWR_BUT_TRIS = 1;
        //BEAGLE_PWR_BUT_PIN = 0;
    }
}
