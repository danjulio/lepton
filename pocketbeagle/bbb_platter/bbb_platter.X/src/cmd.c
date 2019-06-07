/*
 * Command Processor Module
 */
#include "system.h"
#include "cmd.h"
#include "adc.h"
#include "batt.h"
#include "chg_status.h"
#include "eeprom.h"
#include "rtc.h"
#include "system.h"
#include "ups.h"
#include "usb.h"
#include "usb_config.h"
#include "usb_device_cdc.h"
#include "usb_power.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


// Forward declarations for internal functions
void CMD_ProcessRxChar(uint8_t c);
bool CMD_IsValidCommand(uint8_t c);
bool CMD_IsValidDec(uint8_t c, uint8_t* v);
void CMD_Execute(void);
void CMD_ReturnDecVal(uint32_t v, bool includeIndex);
void CMD_ReturnFloatVal(float f);
uint8_t CMD_Load8bitDecVal(char* buf, uint8_t v);
//uint8_t CMD_GetHexChar(uint8_t v);
void CMD_ReturnError(uint8_t e);



// Command processor states
#define CMD_ST_IDLE  0
#define CMD_ST_CMD   1
#define CMD_ST_EQ    2
#define CMD_ST_DATA  3
#define CMD_ST_WAIT  4

// Command termination character
#define TERM_CHAR 0x0D

// Linefeed for responses
#define TERM_LF   0x0A

// Maximum warning buffer size
#define CMD_WARN_BUF_LEN 9


// Command USB variables
uint8_t cmdUSBrxBuffer[CDC_DATA_IN_EP_SIZE];  // RX Data from USB to process
uint8_t cmdUSBrxBufNum;    // Number of valid bytes in the RX Buffer
uint8_t cmdUSBrxBufIndex;  // Index to current byte being processed
uint8_t cmdUSBtxBuffer[CDC_DATA_OUT_EP_SIZE];  // TX Data to send out via USB
uint8_t cmdUSBtxBufIndex;  // Index to next available entry (also the count)
uint8_t cmdUSBwtxBuffer[CMD_WARN_BUF_LEN];
uint8_t cmdUSBwtxBufIndex; // Index to next available entry (also the count)


// Command processing variables
uint8_t cmdState;
char cmdOp;           // Command Op Code
uint8_t cmdIndex;     // Command entry index
bit cmdIsSet;         // =0 for query, =1 for set
uint32_t cmdData;     // Data value for set



// API Routines
void CMD_Initialize(void)
{
    CDCInitEP();

    cmdUSBrxBufNum = 0;
    cmdUSBtxBufIndex = 0;
    cmdUSBwtxBufIndex = 0;
    cmdState = CMD_ST_IDLE;
}


void CMD_Tasks(void)
{
    // Process a character otherwise look for new USB data
    if (cmdUSBrxBufNum != 0) {
        if (cmdUSBtxBufIndex == 0) {
            // Only process data from USB if we're not waiting to send
            CMD_ProcessRxChar(cmdUSBrxBuffer[cmdUSBrxBufIndex++]);
           cmdUSBrxBufNum--;
        }
    } else {
        cmdUSBrxBufNum = getsUSBUSART(cmdUSBrxBuffer, 64);
        cmdUSBrxBufIndex = 0;
    }

    // Attempt to return command response data if available
    if ((cmdUSBtxBufIndex != 0) && (USBUSARTIsTxTrfReady())) {
        putUSBUSART(&cmdUSBtxBuffer[0], cmdUSBtxBufIndex);
        cmdUSBtxBufIndex = 0;
    }

    // Attempt to return any warning string if available
    if ((cmdUSBwtxBufIndex != 0) && (USBUSARTIsTxTrfReady())) {
        putUSBUSART(&cmdUSBwtxBuffer[0], cmdUSBwtxBufIndex);
        cmdUSBwtxBufIndex = 0;
    }

    CDCTxService();
}


void CMD_SendWarnMsg(uint8_t n)
{
    // Message format: "WARN N<CR><LF>"
    cmdUSBwtxBuffer[0] = 'W';
    cmdUSBwtxBuffer[1] = 'A';
    cmdUSBwtxBuffer[2] = 'R';
    cmdUSBwtxBuffer[3] = 'N';
    cmdUSBwtxBuffer[4] = ' ';
    cmdUSBwtxBuffer[5] = '0' + n;
    cmdUSBwtxBuffer[6] = TERM_CHAR;
    cmdUSBwtxBuffer[7] = TERM_LF;
    cmdUSBwtxBuffer[8] = 0;
    cmdUSBwtxBufIndex = 8;
}



// Internal Routines
void CMD_ProcessRxChar(uint8_t c)
{
    uint8_t t;

    switch (cmdState) {
        case CMD_ST_IDLE:
            if (CMD_IsValidCommand(c)) {
                cmdOp = c;
                cmdIsSet = 0;  // Will be set for a "set" command
                cmdIndex = 0;
                cmdData = 0;
                cmdState = CMD_ST_CMD;
            }
            break;
        case CMD_ST_CMD:
            if (c == TERM_CHAR) {
                CMD_Execute();
                cmdState = CMD_ST_IDLE;
            } else if (c == '=') {
                // Command will be a "set"
                cmdIsSet = 1;
                cmdState = CMD_ST_EQ;
            } else if (CMD_IsValidDec(c, &t)) {
                cmdIndex = (cmdIndex * 10) + t;
            }
            break;
        case CMD_ST_EQ:
            if (c == TERM_CHAR) {
                CMD_Execute();
                cmdState = CMD_ST_IDLE;
            } else if (CMD_IsValidDec(c, &t)) {
                cmdData = t;
                cmdState = CMD_ST_DATA;
            } else {
                cmdState = CMD_ST_WAIT;
            }
            break;
        case CMD_ST_DATA:
            if (c == TERM_CHAR) {
                CMD_Execute();
                cmdState = CMD_ST_IDLE;
            } else if (CMD_IsValidDec(c, &t)) {
                cmdData = (cmdData * 10) + t;
            } else {
                cmdState = CMD_ST_WAIT;
            }
            break;
        case CMD_ST_WAIT:
            // Invalid command: just wait until TERM_CHAR
            if (c == TERM_CHAR) {
                cmdState = CMD_ST_IDLE;
            }
            break;
        default:
            cmdState = CMD_ST_IDLE;
    }
}


bool CMD_IsValidCommand(uint8_t c)
{
    return ((c >= 'A') && (c <= 'Z'));
}


bool CMD_IsValidDec(uint8_t c, uint8_t* v)
{
    if ((c >= '0') && (c <= '9')) {
        *v = c - '0';
        return(true);
    }
    return(false);
}


void CMD_Execute(void)
{
    uint8_t t;

    switch (cmdOp) {
        case 'A':
            if (cmdIsSet == 0) {
                if (cmdIndex == 1) {
                    CMD_ReturnDecVal(ADC_Read(cmdIndex), true);
                } else {
                    CMD_ReturnError(CMD_ERR_ILL_INDEX);
                }
            } else {
                CMD_ReturnError(CMD_ERR_NO_SET);
            }
            break;
        case 'B':
            if (cmdIsSet == 0) {
                CMD_ReturnFloatVal(BATT_GetBattVoltage());
            } else {
                CMD_ReturnError(CMD_ERR_NO_SET);
            }
            break;
        case 'C':
            if (cmdIsSet == 0) {
                switch (cmdIndex) {
                    case 0: // Alarm Enable
                        CMD_ReturnDecVal(RTC_GetAlarmEnable(), true);
                        break;
                    case 1: // Low Battery Warning Enable
                        CMD_ReturnDecVal(BATT_GetLowBattWarnEn(), true);
                        break;
                    case 2: // Critical Battery Warning Enable
                        CMD_ReturnDecVal(BATT_GetCritBattWarnEn(), true);
                        break;
                    case 3: // USB Fault Warning Enable
                        CMD_ReturnDecVal(USBPWR_GetFaultWarnEn(), true);
                        break;
                    case 4: // Analog 1 Vref
                        CMD_ReturnDecVal(ADC_GetRefType(), true);
                        break;
                    case 5: // Reserved (unused by this firmware)
                        CMD_ReturnDecVal(0, true);
                        break;
                    case 6: // Reserved (unused by this firmware)
                        CMD_ReturnDecVal(0, true);
                        break;
                    case 7: // Restart Enable
                        CMD_ReturnDecVal(SYSTEM_GetRestartEnable(), true);
                        break;
                    case 8: // Reserved (unused by this firmware)
                        CMD_ReturnDecVal(0, true);
                        break;
                    default:
                        CMD_ReturnError(CMD_ERR_ILL_INDEX);
                }
            } else {
                switch (cmdIndex) {
                    case 0: // Alarm Enable
                        RTC_SetAlarmEnable(cmdData);
                        break;
                    case 1: // Low Battery Warning Enable
                        BATT_SetLowBattWarnEn(cmdData);
                        break;
                    case 2: // Critical Battery Warning Enable
                        BATT_SetCritBattWarnEn(cmdData);
                        break;
                    case 3: // USB Fault Warning Enable
                        USBPWR_SetFaultWarnEn(cmdData);
                        break;
                    case 4: // Analog 1 Vref
                        ADC_SetRefType(cmdData);
                        break;
                    case 5: // Reserved (unused by this firmware)
                        break;
                    case 6: // Reserved (unused by this firmware)
                        break;
                    case 7: // Restart Enable
                        SYSTEM_SetRestartEnable(cmdData);
                        break;
                    case 8: // Reserved (unused by this firmware)
                        break;
                    default:
                        CMD_ReturnError(CMD_ERR_ILL_INDEX);
                }
            }
            break;
        case 'E':
            if (cmdIndex < EEP_NUM_ITEMS) {
                if (cmdIsSet == 0) {
                    CMD_ReturnDecVal(EEP_ReadIndexedItem(cmdIndex), true);
                } else {
                    EEP_WriteIndexedItem(cmdIndex, cmdData);
                }
            } else {
                CMD_ReturnError(CMD_ERR_ILL_INDEX);
            }
            break;
        case 'O':
            if (cmdIsSet == 0) {
                CMD_ReturnDecVal(SYSTEM_GetPowerDownTime(), false);
            } else {
                SYSTEM_SchedulePowerDown(cmdData);
            }
            break;
        case 'R':
            if (cmdIsSet == 0) {
                CMD_ReturnDecVal(RTC_GetRepeatTime(), false);
            } else {
                RTC_SetRepeatTime(cmdData);
            }
            break;
        case 'S':
            if (cmdIsSet == 0) {
                t = BATT_LowDetected() ? CMD_STATUS_LOW_BAT_MASK : 0;
                t |= BATT_CriticalDetected() ? CMD_STATUS_CRIT_BATT_MASK : 0;
                t |= UPS_UsbPowerEnabled() ? CMD_STATUS_USB_POWER_MASK : 0;
                t |= CHGS_ChargeStatus() ? CMD_STATUS_BATT_CHARGE_MASK : 0;
                switch (SYSTEM_GetPowerUpReason()) {
                    case SYSTEM_PWRUP_BUTTON:
                        t |= CMD_STATUS_PWRUP_BUTTON_MASK;
                        break;
                    case SYSTEM_PWRUP_RESTART:
                        t |= CMD_STATUS_PWRUP_BATT_MASK;
                        break;
                    case SYSTEM_PWRUP_USB_PWR:
                        t |= CMD_STATUS_PWRUP_USB_MASK;
                        break;
                    default:
                        t |= CMD_STATUS_PWRUP_ALARM_MASK;
                }
                t |= USBPWR_FaultDetected() ? CMD_STATUS_USB_FAULT_MASK : 0;

                CMD_ReturnDecVal(t, false);
            } else {
                CMD_ReturnError(CMD_ERR_NO_SET);
            }
            break;
        case 'T':
            if (cmdIsSet == 0) {
                CMD_ReturnDecVal(RTC_GetTime(), false);
            } else {
                RTC_SetTime(cmdData);
            }
            break;
        case 'U':
            if ((cmdIndex >= 1) && (cmdIndex <= USBPWR_NUM)) {
                if (cmdIsSet == 0) {
                    CMD_ReturnDecVal(USBPWR_GetEnable(cmdIndex), true);
                } else {
                    USBPWR_SetEnable(cmdIndex, cmdData);
                }
            } else {
                CMD_ReturnError(CMD_ERR_ILL_INDEX);
            }
            break;
        case 'V':
            if (cmdIsSet == 0) {
                CMD_ReturnDecVal(BOARD_TYPE_MASK | FW_MAJOR_MASK | FW_MINOR_MASK, false);
            } else {
                CMD_ReturnError(CMD_ERR_NO_SET);
            }
            break;
        case 'W':
            if (cmdIsSet == 0) {
                CMD_ReturnDecVal(RTC_GetAlarmTime(), false);
            } else {
                RTC_SetAlarmTime(cmdData);
            }
            break;
        default:
            CMD_ReturnError(CMD_ERR_ILL_CMD);
    }
}


void CMD_ReturnDecVal(uint32_t v, bool includeIndex)
{
    uint8_t i;

    // Clear the buffer so we can determine where the string ends
    for (i=0; i<CDC_DATA_OUT_EP_SIZE; i++) {
        cmdUSBtxBuffer[i] = 0;
    }

    // Load the original command
    cmdUSBtxBuffer[0] = cmdOp;

    // Load the index if specified
    i = 1;
    if (includeIndex) {
        i += CMD_Load8bitDecVal(&cmdUSBtxBuffer[1], cmdIndex);
    }

    cmdUSBtxBuffer[i++] = '=';

    // Convert the unsigned long to a string in our buffer
    (void) ultoa(&cmdUSBtxBuffer[i], v, 10);

    // Determine how many characters are loaded
    for (i=0; i<CDC_DATA_OUT_EP_SIZE; i++) {
        if (cmdUSBtxBuffer[i] == 0) break;
    }

    // Terminate the string
    cmdUSBtxBuffer[i++] = TERM_CHAR;
    cmdUSBtxBuffer[i] = TERM_LF;
    cmdUSBtxBufIndex = i+1;
}


void CMD_ReturnFloatVal(float f)
{
    uint8_t i;
    uint32_t l, rem;

    // Clear the buffer so we can determine where the string ends
    for (i=0; i<CDC_DATA_OUT_EP_SIZE; i++) {
        cmdUSBtxBuffer[i] = 0;
    }

    // Load the original command
    cmdUSBtxBuffer[0] = cmdOp;
    cmdUSBtxBuffer[1] = '=';

    // Convert the floating point number into a string with the form N.NN
    l = (uint32_t) f;
    f -= (float) l;
    rem = (uint32_t)round(f*1e2);
    if (rem >= 100) {
        l = l + 1;
        rem = rem - 100;
    }
    sprintf(&cmdUSBtxBuffer[2], "%lu.%2.2lu",l, rem);

    // Determine how many characters are loaded
    for (i=0; i<CDC_DATA_OUT_EP_SIZE; i++) {
        if (cmdUSBtxBuffer[i] == 0) break;
    }

    // Terminate the string
    cmdUSBtxBuffer[i++] = TERM_CHAR;
    cmdUSBtxBuffer[i] = TERM_LF;
    cmdUSBtxBufIndex = i+1;

}


uint8_t CMD_Load8bitDecVal(char* buf, uint8_t v)
{
    sprintf(buf, "%1u", v);

    if (v < 10) {
        return 1;
    } else {
        return 2;
    }
}

/*
uint8_t CMD_GetHexChar(uint8_t v)
{
    v &= 0x0F;

    if (v <= 9) {
        return(v + '0');
    } else {
        return((v-10) + 'A');
    }
}
*/


void CMD_ReturnError(uint8_t e)
{
    cmdUSBtxBuffer[0] = 'E';
    cmdUSBtxBuffer[1] = 'R';
    cmdUSBtxBuffer[2] = 'R';
    cmdUSBtxBuffer[3] = ' ';
    if (CMD_Load8bitDecVal(&cmdUSBtxBuffer[4], e) == 1) {
        cmdUSBtxBuffer[5] = TERM_CHAR;
        cmdUSBtxBuffer[6] = TERM_LF;
        cmdUSBtxBufIndex = 7;
    } else {
        cmdUSBtxBuffer[6] = TERM_CHAR;
        cmdUSBtxBuffer[7] = TERM_LF;
        cmdUSBtxBufIndex = 8;
    }
}


