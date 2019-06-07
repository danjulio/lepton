/*
 * Command Processor Header
 */
#ifndef CMD_H
#define CMD_H

// ERROR MESSAGE CODES
#define CMD_ERR_ILL_CMD   0
#define CMD_ERR_ILL_INDEX 1
#define CMD_ERR_NO_SET    2


// WARNING MESSAGE CODES
#define CMD_WARN_LOW_BATT  0
#define CMD_WARN_CRIT_BATT 1
#define CMD_WARN_USB_FAULT 2


// STATUS Byte Masks
//   Bit 7: USB Fault Detected
//   Bit 6: Reserved
//   Bit 5:4: Power Up Reason
//     00 - Alarm
//     01 - Button
//     10 - Battery Restart
//     11 - USB Power Applied
//   Bit 3: Battery Charging
//   Bit 2: USB Supplying Power
//   Bit 1: Battery Voltage Critical
//   Bit 0: Low Battery Detected
#define CMD_STATUS_USB_FAULT_MASK    0x80
#define CMD_STATUS_PWRUP_ALARM_MASK  0x00
#define CMD_STATUS_PWRUP_BUTTON_MASK 0x10
#define CMD_STATUS_PWRUP_BATT_MASK   0x20
#define CMD_STATUS_PWRUP_USB_MASK    0x30
#define CMD_STATUS_BATT_CHARGE_MASK  0x08
#define CMD_STATUS_USB_POWER_MASK    0x04
#define CMD_STATUS_CRIT_BATT_MASK    0x02
#define CMD_STATUS_LOW_BAT_MASK      0x01



// API
void CMD_Initialize(void);

void CMD_Tasks(void);

void CMD_SendWarnMsg(uint8_t n);

#endif // CMD_H
