/*
 * Common header for both PRUs
 */

/* Shared Memory Layout */
#define SMEM_BASE_PHYS_ADDR      0x10000
#define SMEM_LEN                 (12 * 1024)
#define SMEM_P0_EN_OFFSET        0
#define SMEM_P1_CMD_OFFSET       1
#define SMEM_BUF_START_OFFSET    2
#define SMEM_BUF_END_OFFSET      (SMEM_LEN - 1)

/* Shared Memory Addresses */
#define SMEM_EN_REG      (volatile uint8_t*) (SMEM_BASE_PHYS_ADDR + SMEM_P0_EN_OFFSET)
#define SMEM_CMD_REG     (volatile uint8_t*) (SMEM_BASE_PHYS_ADDR + SMEM_P1_CMD_OFFSET)
#define SMEM_BUF_START   (volatile uint8_t*) (SMEM_BASE_PHYS_ADDR + SMEM_BUF_START_OFFSET)
#define SMEM_BUF_END     (volatile uint8_t*) (SMEM_BASE_PHYS_ADDR + SMEM_BUF_END_OFFSET)

/* P0 Enable values - Set by P1 to enable/disable P0 */
#define P0_DISABLE               0
#define P0_ENABLE                1

/* P1_CMD values - command from PRU0 to PRU1 to tell it how to proceed   */
/* PRU1 clears the command back to IDLE when it is finished executing it */
#define P1_CMD_IDLE              0
#define P1_CMD_IN_FRAME          1
#define P1_CMD_ABORT             2

