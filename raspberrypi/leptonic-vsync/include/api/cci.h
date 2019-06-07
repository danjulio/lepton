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

#define CCI_CMD_OEM_GET_GPIO_MODE 0x4854
#define CCI_CMD_OEM_SET_GPIO_MODE 0x4855

#define WAIT_FOR_BUSY_DEASSERT() while (cci_read_register(fd, CCI_REG_STATUS) & 0x01) ;

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

/* GPIO Modes for use with CCI_CMD_OEM_SET_GPIO_MODE* */
typedef enum {
   LEP_OEM_GPIO_MODE_GPIO = 0,
   LEP_OEM_GPIO_MODE_I2C_MASTER = 1,
   LEP_OEM_GPIO_MODE_SPI_MASTER_VLB_DATA = 2,
   LEP_OEM_GPIO_MODE_SPIO_MASTER_REG_DATA = 3,
   LEP_OEM_GPIO_MODE_SPI_SLAVE_VLB_DATA = 4,
   LEP_OEM_GPIO_MODE_VSYNC = 5
} cci_gpio_mode_t;

/* Setup */
int cci_init(int fd);

/* Primative methods */
int cci_write_register(int fd, uint16_t reg, uint16_t value);
uint16_t cci_read_register(int fd, uint16_t reg);

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

/* Module: OEM */
uint32_t cci_get_gpio_mode(int fd);
void cci_set_gpio_mode(int fd, cci_gpio_mode_t mode);

#endif /* CCI_H */
