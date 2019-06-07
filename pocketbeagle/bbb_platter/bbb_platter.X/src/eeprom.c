/*
 * Persistent storage module
 */
#include "eeprom.h"
#include "adc.h"
#include "batt.h"
#include "usb_power.h"

#include "FLash.h"
#include "HEFLash.h"

// set the last High Endurance Row in this device with our default values
//  Note: We fill the entire row with data to keep the compiler from putting
//  anything else there (which will be destroyed when we update flash)
const uint8_t eepInitData[FLASH_ROWSIZE] @ (HEFLASH_END - FLASH_ROWSIZE + 1) = {
        BATT_DEF_LB_WARN_ADC_VAL >> 8,
        BATT_DEF_LB_WARN_ADC_VAL & 0xFF,
        BATT_DEF_CRIT_ADC_VAL >> 8,
        BATT_DEF_CRIT_ADC_VAL & 0xFF,
        BATT_UV_ADC_VAL >> 8,
        BATT_UV_ADC_VAL & 0xFF,
        BATT_RESTART_ADC_VAL >> 8,
        BATT_RESTART_ADC_VAL & 0xFF,
        BATT_CRIT_TURNOFF_SEC,
        BATT_WARN_MSG_EN,
        BATT_CRIT_MSG_EN,
        BATT_RESTART_EN,
        USBPWR_FAULT_MSG_EN,
        USBPWR_DEF_EN_1,
        USBPWR_DEF_EN_2,
        ADC_DEF_REF_1_CFG,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0
};


// Internal variables
char hef_row_buffer[FLASH_ROWSIZE];  // Used to RMW our eep data



// Internal function forward declarations
void EEP_WriteByte(uint8_t offset, uint8_t d);
void EEP_WriteWord(uint8_t offset, uint16_t d);
uint16_t EEP_GetWord(uint8_t offset);



// API
void EEP_WriteIndexedItem(uint8_t i, uint16_t d)
{
    switch (i) {
        case EEP_BATT_WARN_I:
            EEP_WriteWord(EEP_BATT_WARN_HI, d);
            break;
        case EEP_BATT_CRIT_I:
            EEP_WriteWord(EEP_BATT_CRIT_HI, d);
            break;
        case EEP_BATT_STARTUV_I:
            EEP_WriteWord(EEP_BATT_START_UV_HI, d);
            break;
        case EEP_BATT_RESTART_I:
            EEP_WriteWord(EEP_BATT_RESTART_HI, d);
            break;
        case EEP_BATT_TURNOFF_I:
            EEP_WriteByte(EEP_BATT_TURNOFF_SEC, d);
            break;
        case EEP_BATT_WARN_MSG_I:
            EEP_WriteByte(EEP_BATT_WARN_MSG_EN, d);
            break;
        case EEP_BATT_CRIT_MSG_I:
            EEP_WriteByte(EEP_BATT_CRIT_MSG_EN, d);
            break;
        case EEP_RESTART_EN_I:
            EEP_WriteByte(EEP_RESTART_EN, d);
            break;
        case EEP_USB_FAULT_MSG_I:
            EEP_WriteByte(EEP_USB_FAULT_MSG_EN, d);
            break;
        case EEP_USB_1_PWR_EN_I:
            EEP_WriteByte(EEP_USB_1_PWR_EN, d);
            break;
        case EEP_USB_2_PWR_EN_I:
            EEP_WriteByte(EEP_USB_2_PWR_EN, d);
            break;
        case EEP_AREF_1_I:
            EEP_WriteByte(EEP_AREF_1, d);
            break;
    }
}


uint16_t EEP_ReadIndexedItem(uint8_t i)
{
    switch (i) {
        case EEP_BATT_WARN_I:
            return(EEP_GetWord(EEP_BATT_WARN_HI));
        case EEP_BATT_CRIT_I:
            return(EEP_GetWord(EEP_BATT_CRIT_HI));
        case EEP_BATT_STARTUV_I:
            return(EEP_GetWord(EEP_BATT_START_UV_HI));
        case EEP_BATT_RESTART_I:
            return(EEP_GetWord(EEP_BATT_RESTART_HI));
        case EEP_BATT_TURNOFF_I:
            return(HEFLASH_readByte(HEFLASH_MAXROWS-1, EEP_BATT_TURNOFF_SEC));
        case EEP_BATT_WARN_MSG_I:
            return(HEFLASH_readByte(HEFLASH_MAXROWS-1, EEP_BATT_WARN_MSG_EN));
        case EEP_BATT_CRIT_MSG_I:
            return(HEFLASH_readByte(HEFLASH_MAXROWS-1, EEP_BATT_CRIT_MSG_EN));
        case EEP_RESTART_EN_I:
            return(HEFLASH_readByte(HEFLASH_MAXROWS-1, EEP_RESTART_EN));
        case EEP_USB_FAULT_MSG_I:
            return(HEFLASH_readByte(HEFLASH_MAXROWS-1, EEP_USB_FAULT_MSG_EN));
        case EEP_USB_1_PWR_EN_I:
            return(HEFLASH_readByte(HEFLASH_MAXROWS-1, EEP_USB_1_PWR_EN));
        case EEP_USB_2_PWR_EN_I:
            return(HEFLASH_readByte(HEFLASH_MAXROWS-1, EEP_USB_2_PWR_EN));
        case EEP_AREF_1_I:
            return(HEFLASH_readByte(HEFLASH_MAXROWS-1, EEP_AREF_1));
        default:
            return(0);
    }
}


uint16_t EEP_GetBattWarnAdc()
{
    return(EEP_GetWord(EEP_BATT_WARN_HI));
}


uint16_t EEP_GetBattCritAdc()
{
    return(EEP_GetWord(EEP_BATT_CRIT_HI));
}


uint16_t EEP_GetBattStartUvAdc()
{
    return(EEP_GetWord(EEP_BATT_START_UV_HI));
}


uint16_t EEP_GetBattRestartAdc()
{
    return(EEP_GetWord(EEP_BATT_RESTART_HI));
}


uint8_t EEP_GetBattCritTimeout()
{
    return(HEFLASH_readByte(HEFLASH_MAXROWS-1, EEP_BATT_TURNOFF_SEC));
}


bool EEP_GetBattWarnMsgEn()
{
    return(HEFLASH_readByte(HEFLASH_MAXROWS-1, EEP_BATT_WARN_MSG_EN) ? 1 : 0);
}


bool EEP_GetBattCritMsgEn()
{
    return(HEFLASH_readByte(HEFLASH_MAXROWS-1, EEP_BATT_CRIT_MSG_EN) ? 1 : 0);
}


bool EEP_GetRestartEn()
{
    return(HEFLASH_readByte(HEFLASH_MAXROWS-1, EEP_RESTART_EN) ? 1 : 0);
}


bool EEP_GetUsbFaultMsgEn()
{
    return(HEFLASH_readByte(HEFLASH_MAXROWS-1, EEP_USB_FAULT_MSG_EN) ? 1 : 0);
}


// i is 1-based
bool EEP_GetUsbPowerEn(uint8_t i)
{
    return(HEFLASH_readByte(HEFLASH_MAXROWS-1, EEP_USB_1_PWR_EN + (i-1)) ? 1 : 0);
}


uint8_t EEP_GetArefCfg()
{
    return(HEFLASH_readByte(HEFLASH_MAXROWS-1, EEP_AREF_1));
}


// Internal functions
void EEP_WriteByte(uint8_t offset, uint8_t d)
{
    if (HEFLASH_readBlock(&hef_row_buffer[0], HEFLASH_MAXROWS-1, EEP_NUM_BYTES) == 0) {
        // Successfully loaded our row data
        //
        // Update the specified byte offset
        hef_row_buffer[offset] = d;

        // Write the buffer back to flash (we assume there will be no write errors)
        (void) HEFLASH_writeBlock(HEFLASH_MAXROWS-1, &hef_row_buffer[0], EEP_NUM_BYTES);
    }
}


void EEP_WriteWord(uint8_t offset, uint16_t d)
{
    if (HEFLASH_readBlock(&hef_row_buffer[0], HEFLASH_MAXROWS-1, EEP_NUM_BYTES) == 0) {
        // Successfully loaded our row data
        //
        // Update the specified byte offset
        hef_row_buffer[offset] = d >> 8;
        hef_row_buffer[offset+1] = d & 0xFF;

        // Write the buffer back to flash (we assume there will be no write errors)
        (void) HEFLASH_writeBlock(HEFLASH_MAXROWS-1, &hef_row_buffer[0], EEP_NUM_BYTES);
    }
}


uint16_t EEP_GetWord(uint8_t offset)
{
    return(((uint16_t) HEFLASH_readByte(HEFLASH_MAXROWS-1, offset) << 8) |
            HEFLASH_readByte(HEFLASH_MAXROWS-1, offset+1));
}
