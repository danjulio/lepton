/*
 * Persistent storage module header
 */
#ifndef EEPROM_H
#define EEPROM_H

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>


// Number of EEPROM items
#define EEP_NUM_ITEMS        12

// EEPROM item index values
#define EEP_BATT_WARN_I      0
#define EEP_BATT_CRIT_I      1
#define EEP_BATT_STARTUV_I   2
#define EEP_BATT_RESTART_I   3
#define EEP_BATT_TURNOFF_I   4
#define EEP_BATT_WARN_MSG_I  5
#define EEP_BATT_CRIT_MSG_I  6
#define EEP_RESTART_EN_I     7
#define EEP_USB_FAULT_MSG_I  8
#define EEP_USB_1_PWR_EN_I   9
#define EEP_USB_2_PWR_EN_I   10
#define EEP_AREF_1_I         11


// Number of EEPROM bytes
#define EEP_NUM_BYTES        16

// EEPROM HEF page offsets for defined storage locations
#define EEP_BATT_WARN_HI     0
#define EEP_BATT_WARN_LO     1
#define EEP_BATT_CRIT_HI     2
#define EEP_BATT_CRIT_LO     3
#define EEP_BATT_START_UV_HI 4
#define EEP_BATT_START_UV_LO 5
#define EEP_BATT_RESTART_HI  6
#define EEP_BATT_RESTART_LO  7
#define EEP_BATT_TURNOFF_SEC 8
#define EEP_BATT_WARN_MSG_EN 9
#define EEP_BATT_CRIT_MSG_EN 10
#define EEP_RESTART_EN       11
#define EEP_USB_FAULT_MSG_EN 12
#define EEP_USB_1_PWR_EN     13
#define EEP_USB_2_PWR_EN     14
#define EEP_AREF_1           15



// API
void EEP_WriteIndexedItem(uint8_t i, uint16_t d);
uint16_t EEP_ReadIndexedItem(uint8_t i);

uint16_t EEP_GetBattWarnAdc();
uint16_t EEP_GetBattCritAdc();
uint16_t EEP_GetBattStartUvAdc();
uint16_t EEP_GetBattRestartAdc();
uint8_t EEP_GetBattCritTimeout();
bool EEP_GetBattWarnMsgEn();
bool EEP_GetBattCritMsgEn();
bool EEP_GetRestartEn();
bool EEP_GetUsbFaultMsgEn();
bool EEP_GetUsbPowerEn(uint8_t i);
uint8_t EEP_GetArefCfg();


#endif // EEPROM_H
