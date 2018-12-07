#ifndef CCI_H
#define CCI_H

#include <stdint.h>

/* CCI constants */
#define CCI_WORD_LENGTH 0x02
#define CCI_ADDRESS 0x2A

/* CCI register locations */
#define CCI_REG_STATUS 0x0002
#define CCI_REG_COMMAND 0x0004
#define CCI_REG_DATA_LENGTH 0x0006
#define CCI_REG_DATA_0 0x0008

/* Commands */
#define CCI_CMD_SYS_RUN_FFC 0x0242
#define CCI_CMD_SYS_GET_UPTIME 0x020C
#define CCI_CMD_SYS_GET_TELEMETRY_ENABLE_STATE 0x0218
#define CCI_CMD_SYS_SET_TELEMETRY_ENABLE_STATE 0x0219
#define CCI_CMD_SYS_GET_TELEMETRY_LOCATION 0x021C
#define CCI_CMD_SYS_SET_TELEMETRY_LOCATION 0x021D

#define CCI_CMD_RAD_GET_RADIOMETRY_ENABLE_STATE 0x4E10
#define CCI_CMD_RAD_SET_RADIOMETRY_ENABLE_STATE 0x4E11
#define CCI_CMD_RAD_GET_RADIOMETRY_TLINEAR_ENABLE_STATE 0x4EC0
#define CCI_CMD_RAD_SET_RADIOMETRY_TLINEAR_ENABLE_STATE 0x4EC1

#define CCI_CMD_AGC_GET_AGC_ENABLE_STATE 0x0100
#define CCI_CMD_AGC_SET_AGC_ENABLE_STATE 0x0101
#define CCI_CMD_AGC_GET_CALC_ENABLE_STATE 0x0148
#define CCI_CMD_AGC_SET_CALC_ENABLE_STATE 0x0149

#define CCI_CMD_OEM_RUN_REBOOT 0x4842


#define WAIT_FOR_BUSY_DEASSERT() cci_wait_busy_clear(fd);

/* Telemetry Modes for use with CCI_CMD_SYS_SET_TELEMETRY_* */
typedef enum {
  CCI_TELEMETRY_DISABLED,
  CCI_TELEMETRY_ENABLED,
} cci_telemetry_enable_state_t;

typedef enum {
  CCI_TELEMETRY_LOCATION_HEADER,
  CCI_TELEMETRY_LOCATION_FOOTER,
} cci_telemetry_location_t;

/* Radiometry Modes for use with CCI_CMD_RAD_SET_RADIOMETRY* */
typedef enum {
  CCI_RADIOMETRY_DISABLED,
  CCI_RADIOMETRY_ENABLED,
} cci_radiometry_enable_state_t;

typedef enum {
  CCI_RADIOMETRY_TLINEAR_DISABLED,
  CCI_RADIOMETRY_TLINEAR_ENABLED,
} cci_radiometry_tlinear_enable_state_t;

/* AGC Modes for use with CCI_CMD_AGC_SET_AGC* */
typedef enum {
  CCI_AGC_DISABLED,
  CCI_AGC_ENABLED,
} cci_agc_enable_state_t;

/* Setup */
int cci_init(int fd);

/* Primative methods */
int cci_write_register(int fd, uint16_t reg, uint16_t value);
uint16_t cci_read_register(int fd, uint16_t reg);
void cci_wait_busy_clear(int fd);

/* Module: SYS */
void cci_run_ffc(int fd);
uint32_t cci_get_uptime(int fd);
void cci_set_telemetry_enable_state(int fd, cci_telemetry_enable_state_t state);
uint32_t cci_get_telemetry_enable_state(int fd);
void cci_set_telemetry_location(int fd, cci_telemetry_location_t location);
uint32_t cci_get_telemetry_location(int fd);

/* Module: RAD */
void cci_set_radiometry_enable_state(int fd, cci_radiometry_enable_state_t state);
uint32_t cci_get_radiometry_enable_state(int fd);
void cci_set_radiometry_tlinear_enable_state(int fd, cci_radiometry_tlinear_enable_state_t state);
uint32_t cci_get_radiometry_tlinear_enable_state(int fd);

/* Module: AGC */
void cci_set_agc_enable_state(int fd, cci_agc_enable_state_t state);
uint32_t cci_get_agc_enable_state(int fd);
void cci_set_agc_calc_enable_state(int fd, cci_agc_enable_state_t state);
uint32_t cci_get_agc_calc_enable_state(int fd);

/* Module: OEM */
void cc_run_oem_reboot(int fd);

#endif /* CCI_H */
