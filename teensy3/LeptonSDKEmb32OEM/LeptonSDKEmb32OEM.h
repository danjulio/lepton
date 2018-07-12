/*******************************************************************************
**
**   File NAME: LeptonSDKEmb32OEM.h
**
**   CREATED: 6/12/2018
**
**   DESCRIPTION: Lepton SDK Module ported to Arduino
**
**   HISTORY: 6/12/2018 DJD - Initial Draft
**
**   Based on FLIR Systems Inc source code.  Their license is copied below
**   and forms the license for this code.
**
**      Copyright 2011,2012,2013,2014 FLIR Systems - Commercial
**      Vision Systems.  All rights reserved.
**
**      Proprietary - PROPRIETARY - FLIR Systems Inc.
**
**      This document is controlled to FLIR Technology Level 2.
**      The information contained in this document pertains to a
**      dual use product Controlled for export by the Export
**      Administration Regulations (EAR). Diversion contrary to
**      US law is prohibited.  US Department of Commerce
**      authorization is not required prior to export or
**      transfer to foreign persons or parties unless otherwise
**      prohibited.
**
**      Redistribution and use in source and binary forms, with
**      or without modification, are permitted provided that the
**      following conditions are met:
**
**      Redistributions of source code must retain the above
**      copyright notice, this list of conditions and the
**      following disclaimer.
**
**      Redistributions in binary form must reproduce the above
**      copyright notice, this list of conditions and the
**      following disclaimer in the documentation and/or other
**      materials provided with the distribution.
**
**      Neither the name of the FLIR Systems Corporation nor the
**      names of its contributors may be used to endorse or
**      promote products derived from this software without
**      specific prior written permission.
**
**      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
**      CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
**      WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
**      WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
**      PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
**      COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
**      DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
**      CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
**      PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
**      USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
**      CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
**      CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
**      NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
**      USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
**      OF SUCH DAMAGE.
**
*******************************************************************************/
#ifndef _LEPTON_SDK_H_
#define _LEPTON_SDK_H_

/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>


/******************************************************************************/
/** EXPORTED DEFINES                                                         **/
/******************************************************************************/

/* Types
*/
#define     LEP_FAILURE             -1
#define     LEP_SUCCESS             0
#define     LEP_TRUE                1
#define     LEP_FALSE               0
#define LEP_NULL            0


/* SDK Config
*/
#define USE_DEPRECATED_SERIAL_NUMBER_INTERFACE  0
#define USE_DEPRECATED_PART_NUMBER_INTERFACE    0
#define USE_DEPRECATED_ASICID_INTERFACE         0
#define USE_DEPRECATED_HOUSING_TCP_INTERFACE    0
#define USE_BORESIGHT_MEASUREMENT_FUNCTIONS     1


/* SDK Constants
*/
     #define LEP_JOVA_I2C
     #define LEP_SDK_VERSION_MAJOR         3
     #define LEP_SDK_VERSION_MINOR         3
     #define LEP_SDK_VERSION_BUILD         13

    /* SDK Module Command IDs
    */
    #define LEP_SDK_MODULE_BASE            0x0000

    #define LEP_SDK_ENABLE_STATE           (LEP_SDK_MODULE_BASE + 0x0000 )

/* Macros
*/

    #ifndef MIN
        #define MIN(a, b) ((a) < (b)? (a): (b))
    #endif
    #ifndef MAX
        #define MAX(a, b) ((a) > (b)? (a): (b))
    #endif

    #ifndef LOW_WORD
        #define LOW_WORD(longVariable) ((LEP_UINT16)longVariable)
    #endif
    #ifndef HIGH_WORD
        #define HIGH_WORD(longVariable) ((LEP_UINT16)(longVariable>>16))
    #endif
    #ifndef LOW_BYTE
        #define LOW_BYTE(w)           ((LEP_UINT8)(w))
    #endif
    #ifndef HIGH_BYTE
        #define HIGH_BYTE(w)           ((LEP_UINT8)(((w) >> 8) & 0xFF))
    #endif

    #ifndef LOW_NIBBLE
        #define LOW_NIBBLE(w)           ((LEP_UINT8)(w) & 0x0F)
    #endif
    #ifndef HIGH_NIBBLE
        #define HIGH_NIBBLE(w)           ((LEP_UINT8)(((w) >> 4) & 0x0F))
    #endif

    #define CLR_BIT(_port,_bit)         ((_port) & ~(_bit))

    #define REVERSE_ENDIENESS_UINT16(uint16Var) \
           ( ( ((LEP_UINT16)LOW_BYTE(uint16Var))<<8) + (LEP_UINT16)HIGH_BYTE(uint16Var))

    #define REVERSE_ENDIENESS_UINT32(uint32Var) \
           ( ((LEP_UINT32)REVERSE_ENDIENESS_UINT16(LOW_WORD(uint32Var)) << 16) + \
             (LEP_UINT32)REVERSE_ENDIENESS_UINT16(HIGH_WORD(uint32Var) ) )

    #define REVERSE_NIBBLE_UINT8(uint8Var) \
           ( ( ((LEP_UINT8)LOW_NIBBLE(uint8Var))<<4) + (LEP_UINT8)HIGH_NIBBLE(uint8Var))

    #define REVERSE_BYTEORDER_UINT32(uint32Var) \
           ( (((LEP_UINT32)LOW_BYTE(uint32Var))<<24) + (((LEP_UINT32)HIGH_BYTE(uint32Var))<<16) + \
             (((LEP_UINT32)LOW_BYTE(HIGH_WORD(uint32Var)))<<8) + (LEP_UINT32)HIGH_BYTE(HIGH_WORD(uint32Var)) )

    #define WORD_SWAP_16(uint32Var)  \
            ( ((LEP_UINT16)LOW_WORD(uint32Var) << 16) + ((LEP_UINT16)HIGH_WORD(uint32Var)) )


    #ifndef NUM_ELEMENTS_IN_ARRAY
        #define NUM_ELEMENTS_IN_ARRAY(array) (sizeof (array) / sizeof ((array) [0]))
    #endif /* NUM_ELEMENTS_IN_ARRAY */

    #ifndef NELEMENTS
        #define NELEMENTS(array)      /* number of elements in an array */ \
              (sizeof (array) / sizeof ((array) [0]))
    #endif /* NELEMENTS */


/* AGC Module Command IDs
*/
   #define LEP_AGC_MODULE_BASE                     0x0100

   #define LEP_CID_AGC_ENABLE_STATE                (LEP_AGC_MODULE_BASE + 0x0000 )
   #define LEP_CID_AGC_POLICY                      (LEP_AGC_MODULE_BASE + 0x0004 )
   #define LEP_CID_AGC_ROI                         (LEP_AGC_MODULE_BASE + 0x0008 )
   #define LEP_CID_AGC_STATISTICS                  (LEP_AGC_MODULE_BASE + 0x000C )
   #define LEP_CID_AGC_HISTOGRAM_CLIP_PERCENT      (LEP_AGC_MODULE_BASE + 0x0010 )
   #define LEP_CID_AGC_HISTOGRAM_TAIL_SIZE         (LEP_AGC_MODULE_BASE + 0x0014 )
   #define LEP_CID_AGC_LINEAR_MAX_GAIN             (LEP_AGC_MODULE_BASE + 0x0018 )
   #define LEP_CID_AGC_LINEAR_MIDPOINT             (LEP_AGC_MODULE_BASE + 0x001C )
   #define LEP_CID_AGC_LINEAR_DAMPENING_FACTOR     (LEP_AGC_MODULE_BASE + 0x0020 )
   #define LEP_CID_AGC_HEQ_DAMPENING_FACTOR        (LEP_AGC_MODULE_BASE + 0x0024 )
   #define LEP_CID_AGC_HEQ_MAX_GAIN                (LEP_AGC_MODULE_BASE + 0x0028 )
   #define LEP_CID_AGC_HEQ_CLIP_LIMIT_HIGH         (LEP_AGC_MODULE_BASE + 0x002C )
   #define LEP_CID_AGC_HEQ_CLIP_LIMIT_LOW          (LEP_AGC_MODULE_BASE + 0x0030 )
   #define LEP_CID_AGC_HEQ_BIN_EXTENSION           (LEP_AGC_MODULE_BASE + 0x0034 )
   #define LEP_CID_AGC_HEQ_MIDPOINT                (LEP_AGC_MODULE_BASE + 0x0038 )
   #define LEP_CID_AGC_HEQ_EMPTY_COUNTS            (LEP_AGC_MODULE_BASE + 0x003C )
   #define LEP_CID_AGC_HEQ_NORMALIZATION_FACTOR    (LEP_AGC_MODULE_BASE + 0x0040 )
   #define LEP_CID_AGC_HEQ_SCALE_FACTOR            (LEP_AGC_MODULE_BASE + 0x0044 )
   #define LEP_CID_AGC_CALC_ENABLE_STATE           (LEP_AGC_MODULE_BASE + 0x0048 )
   #define LEP_CID_AGC_HEQ_LINEAR_PERCENT          (LEP_AGC_MODULE_BASE + 0x004C )


/* AGC Module Attribute Scaling and Module Attribute Limits
*/
/* Linear
*/
   #define LEP_AGC_MAX_HISTOGRAM_CLIP_PERCENT      100         /* Scale is 10x  100 == 10.0%  */
   #define LEP_AGC_MAX_HISTOGRAM_TAIL_SIZE         (80*60)     /* Scale is 1x  units: pixels  */
   #define LEP_AGC_MIN_LINEAR_MAX_GAIN              1          /* Scale is 1x    */
   #define LEP_AGC_MAX_LINEAR_MAX_GAIN              4          /* Scale is 1x    */
   #define LEP_AGC_MIN_LINEAR_MIDPOINT              1          /* Scale is 1x    */
   #define LEP_AGC_MAX_LINEAR_MIDPOINT              254        /* Scale is 1x    */
   #define LEP_AGC_MAX_LINEAR_DAMPENING_FACTOR      100        /* Scale is 1x  units: percent  */

/* Histogram Equalization
*/
   #define LEP_AGC_MAX_HEQ_DAMPENING_FACTOR         100        /* Scale is 1x  units: percent  */
   #define LEP_AGC_MIN_HEQ_MAX_GAIN                 1          /* Scale is 1x    */
   #define LEP_AGC_MAX_HEQ_MAX_GAIN                 4          /* Scale is 1x    */

   #define LEP_AGC_HEQ_CLIP_LIMIT_HIGH             (80*60)     /* Scale is 1x  units: pixels  */
   #define LEP_AGC_HEQ_CLIP_LIMIT_LOW              (80*60)     /* Scale is 1x  units: pixels  */
   #define LEP_AGC_HEQ_MAX_BIN_EXTENSION            16         /* Scale is 1x  units: bins  */

   #define LEP_AGC_MIN_HEQ_MIDPOINT                127         /* Scale is 1x    */
   #define LEP_AGC_MAX_HEQ_MIDPOINT                65534       /* Scale is 1x    */

/* ROI
*/
   #define LEP_AGC_MAX_COL                         79
   #define LEP_AGC_MAX_ROW                         59
   #define LEP_AGC_MIN_COL                         0
   #define LEP_AGC_MIN_ROW                         0

/* Timeout count to wait for I2C command to complete
*/
    #define LEPTON_I2C_COMMAND_BUSY_WAIT_COUNT              1000

/* DEVICE ADDRESSES
*/
/* The Lepton camera's device address
*/
    #define LEP_I2C_DEVICE_ADDRESS              0x2A

/* Block Data Buffers
*/
    #define LEP_DATA_BUFFER_0_BASE_ADDR         0xF800
    #define LEP_DATA_BUFFER_1_BASE_ADDR         0xFC00


/* The Lepton I2C Registers Sub-Addresses
*/
    #define LEP_I2C_REG_BASE_ADDR               0x0000

    /* Host On Switch when camera is in stand by of off
    */
    #define LEP_I2C_POWER_REG                  (LEP_I2C_REG_BASE_ADDR + 0x0000 )

    /* Host Command Interface over I2C
    */
    #define LEP_I2C_STATUS_REG                 (LEP_I2C_REG_BASE_ADDR + 0x0002 )
    #define LEP_I2C_COMMAND_REG                (LEP_I2C_REG_BASE_ADDR + 0x0004 )
    #define LEP_I2C_DATA_LENGTH_REG            (LEP_I2C_REG_BASE_ADDR + 0x0006 )
    #define LEP_I2C_DATA_0_REG                 (LEP_I2C_REG_BASE_ADDR + 0x0008 )
    #define LEP_I2C_DATA_1_REG                 (LEP_I2C_REG_BASE_ADDR + 0x000A )
    #define LEP_I2C_DATA_2_REG                 (LEP_I2C_REG_BASE_ADDR + 0x000C )
    #define LEP_I2C_DATA_3_REG                 (LEP_I2C_REG_BASE_ADDR + 0x000E )
    #define LEP_I2C_DATA_4_REG                 (LEP_I2C_REG_BASE_ADDR + 0x0010 )
    #define LEP_I2C_DATA_5_REG                 (LEP_I2C_REG_BASE_ADDR + 0x0012 )
    #define LEP_I2C_DATA_6_REG                 (LEP_I2C_REG_BASE_ADDR + 0x0014 )
    #define LEP_I2C_DATA_7_REG                 (LEP_I2C_REG_BASE_ADDR + 0x0016 )
    #define LEP_I2C_DATA_8_REG                 (LEP_I2C_REG_BASE_ADDR + 0x0018 )
    #define LEP_I2C_DATA_9_REG                 (LEP_I2C_REG_BASE_ADDR + 0x001A )
    #define LEP_I2C_DATA_10_REG                (LEP_I2C_REG_BASE_ADDR + 0x001C )
    #define LEP_I2C_DATA_11_REG                (LEP_I2C_REG_BASE_ADDR + 0x001E )
    #define LEP_I2C_DATA_12_REG                (LEP_I2C_REG_BASE_ADDR + 0x0020 )
    #define LEP_I2C_DATA_13_REG                (LEP_I2C_REG_BASE_ADDR + 0x0022 )
    #define LEP_I2C_DATA_14_REG                (LEP_I2C_REG_BASE_ADDR + 0x0024 )
    #define LEP_I2C_DATA_15_REG                (LEP_I2C_REG_BASE_ADDR + 0x0026 )

    #define LEP_I2C_DATA_CRC_REG               (LEP_I2C_REG_BASE_ADDR + 0x0028 )

    #define LEP_I2C_DATA_BUFFER_0              (LEP_DATA_BUFFER_0_BASE_ADDR )
    #define LEP_I2C_DATA_BUFFER_0_END          (LEP_DATA_BUFFER_0_BASE_ADDR + 0x03FF )
    #define LEP_I2C_DATA_BUFFER_0_LENGTH        0x400

    #define LEP_I2C_DATA_BUFFER_1              (LEP_DATA_BUFFER_1_BASE_ADDR )
    #define LEP_I2C_DATA_BUFFER_1_END          (LEP_DATA_BUFFER_1_BASE_ADDR + 0x03FF )
    #define LEP_I2C_DATA_BUFFER_1_LENGTH        0x400

    #define LEP_I2C_STATUS_BUSY_BIT_MASK        0x0001   /* Bit 0 is the Busy Bit */

/* OEM Module Command IDs
*/
   #define LEP_OEM_MODULE_BASE                     0x0800

   #define LEP_CID_OEM_POWER_DOWN                  (LEP_OEM_MODULE_BASE + 0x4000 )
   #define LEP_CID_OEM_STANDBY                     (LEP_OEM_MODULE_BASE + 0x4004 )
   #define LEP_CID_OEM_LOW_POWER_MODE_1            (LEP_OEM_MODULE_BASE + 0x4008 )
   #define LEP_CID_OEM_LOW_POWER_MODE_2            (LEP_OEM_MODULE_BASE + 0x400C )
   #define LEP_CID_OEM_BIT_TEST                    (LEP_OEM_MODULE_BASE + 0x4010 )
   #define LEP_CID_OEM_MASK_REVISION               (LEP_OEM_MODULE_BASE + 0x4014 )
//#define LEP_CID_OEM_MASTER_ID                   (LEP_OEM_MODULE_BASE + 0x4018 )
   #define LEP_CID_OEM_FLIR_PART_NUMBER            (LEP_OEM_MODULE_BASE + 0x401C )
   #define LEP_CID_OEM_SOFTWARE_VERSION            (LEP_OEM_MODULE_BASE + 0x4020 )
   #define LEP_CID_OEM_VIDEO_OUTPUT_ENABLE         (LEP_OEM_MODULE_BASE + 0x4024 )
   #define LEP_CID_OEM_VIDEO_OUTPUT_FORMAT         (LEP_OEM_MODULE_BASE + 0x4028 )
   #define LEP_CID_OEM_VIDEO_OUTPUT_SOURCE         (LEP_OEM_MODULE_BASE + 0x402C )
   #define LEP_CID_OEM_VIDEO_OUTPUT_CHANNEL        (LEP_OEM_MODULE_BASE + 0x4030 )
   #define LEP_CID_OEM_VIDEO_GAMMA_ENABLE          (LEP_OEM_MODULE_BASE + 0x4034 )
   #define LEP_CID_OEM_CUST_PART_NUMBER            (LEP_OEM_MODULE_BASE + 0x4038 )
   #define LEP_CID_OEM_VIDEO_OUTPUT_CONSTANT       (LEP_OEM_MODULE_BASE + 0x403C )
   #define LEP_CID_OEM_REBOOT                      (LEP_OEM_MODULE_BASE + 0x4040 )
   #define LEP_CID_OEM_FFC_NORMALIZATION_TARGET    (LEP_OEM_MODULE_BASE + 0x4044 )
   #define LEP_CID_OEM_STATUS                      (LEP_OEM_MODULE_BASE + 0x4048 )
   #define LEP_CID_OEM_SCENE_MEAN_VALUE            (LEP_OEM_MODULE_BASE + 0x404C )
   #define LEP_CID_OEM_POWER_MODE                  (LEP_OEM_MODULE_BASE + 0x4050 )

   #define LEP_CID_OEM_GPIO_MODE_SELECT            (LEP_OEM_MODULE_BASE + 0x4054 )
   #define LEP_CID_OEM_GPIO_VSYNC_PHASE_DELAY      (LEP_OEM_MODULE_BASE + 0x4058 )

   #define LEP_CID_OEM_USER_DEFAULTS               (LEP_OEM_MODULE_BASE + 0x405C )
   #define LEP_CID_OEM_USER_DEFAULTS_RESTORE       (LEP_OEM_MODULE_BASE + 0x4060 )
   #define LEP_CID_OEM_SHUTTER_PROFILE_OBJ         (LEP_OEM_MODULE_BASE + 0x4064 )
   #define LEP_CID_OEM_THERMAL_SHUTDOWN_ENABLE_STATE (LEP_OEM_MODULE_BASE + 0x4068 )
   #define LEP_CID_OEM_BAD_PIXEL_REPLACE_CONTROL   (LEP_OEM_MODULE_BASE + 0x406C )
   #define LEP_CID_OEM_TEMPORAL_FILTER_CONTROL     (LEP_OEM_MODULE_BASE + 0x4070 )
   #define LEP_CID_OEM_COLUMN_NOISE_ESTIMATE_CONTROL (LEP_OEM_MODULE_BASE + 0x4074 )
   #define LEP_CID_OEM_PIXEL_NOISE_ESTIMATE_CONTROL (LEP_OEM_MODULE_BASE + 0x4078 )

   #define LEP_OEM_MAX_PART_NUMBER_CHAR_SIZE       32

/* RAD
*/
   #define LEP_RAD_TEMPERATURE_SCALE_FACTOR         100         // All temperatures in degrees C are scaled by 100 1.20 is 120
//
   #define LEP_RAD_RBFO_SCALE_SHIFT                 13          // 2^13 = 8192

   #define LEP_RAD_LUT128_ENTRIES                   128
   #define LEP_RAD_LUT256_ENTRIES                   256


   #define LEP_RAD_MODULE_BASE                     0x4E00   // includes the OEM Bit set 0x4000

   #define LEP_CID_RAD_RBFO_INTERNAL               (LEP_RAD_MODULE_BASE + 0x0000 )  /* High Gain */
   #define LEP_CID_RAD_RBFO_EXTERNAL               (LEP_RAD_MODULE_BASE + 0x0004 )  /* High Gain */
   #define LEP_CID_RAD_DEBUG_TEMP                  (LEP_RAD_MODULE_BASE + 0x0008 )
   #define LEP_CID_RAD_DEBUG_FLUX                  (LEP_RAD_MODULE_BASE + 0x000C )
   #define LEP_CID_RAD_ENABLE_STATE                (LEP_RAD_MODULE_BASE + 0x0010 )
   #define LEP_CID_RAD_GLOBAL_OFFSET               (LEP_RAD_MODULE_BASE + 0x0014 )
   #define LEP_CID_RAD_TFPA_CTS_MODE               (LEP_RAD_MODULE_BASE + 0x0018 )
   #define LEP_CID_RAD_TFPA_CTS                    (LEP_RAD_MODULE_BASE + 0x001C )
   #define LEP_CID_RAD_TEQ_SHUTTER_LUT             (LEP_RAD_MODULE_BASE + 0x0020 )
   #define LEP_CID_RAD_TSHUTTER_MODE               (LEP_RAD_MODULE_BASE + 0x0024 )
   #define LEP_CID_RAD_TSHUTTER                    (LEP_RAD_MODULE_BASE + 0x0028 )
   #define LEP_CID_RAD_RUN_FFC                     (LEP_RAD_MODULE_BASE + 0x002C )
   #define LEP_CID_RAD_RUN_STATUS                  (LEP_RAD_MODULE_BASE + 0x0030 )
   #define LEP_CID_RAD_RESPONSIVITY_SHIFT          (LEP_RAD_MODULE_BASE + 0x0034 )
   #define LEP_CID_RAD_F_NUMBER                    (LEP_RAD_MODULE_BASE + 0x0038 )
   #define LEP_CID_RAD_TAU_LENS                    (LEP_RAD_MODULE_BASE + 0x003C )
   #define LEP_CID_RAD_RESPONSIVITY_VALUE_LUT      (LEP_RAD_MODULE_BASE + 0x0040 )
   #define LEP_CID_RAD_GLOBAL_GAIN                 (LEP_RAD_MODULE_BASE + 0x0044 )
   #define LEP_CID_RAD_RADIOMETRY_FILTER           (LEP_RAD_MODULE_BASE + 0x0048 )
   #define LEP_CID_RAD_TFPA_LUT                    (LEP_RAD_MODULE_BASE + 0x004C )
   #define LEP_CID_RAD_TAUX_LUT                    (LEP_RAD_MODULE_BASE + 0x0050 )
   #define LEP_CID_RAD_TAUX_CTS_MODE               (LEP_RAD_MODULE_BASE + 0x0054 )
   #define LEP_CID_RAD_TAUX_CTS                    (LEP_RAD_MODULE_BASE + 0x0058 )
   #define LEP_CID_RAD_TEQ_SHUTTER_FLUX            (LEP_RAD_MODULE_BASE + 0x005C )
   #define LEP_CID_RAD_MFFC_FLUX                   (LEP_RAD_MODULE_BASE + 0x0060 )
   #define LEP_CID_RAD_FRAME_MEDIAN_VALUE          (LEP_RAD_MODULE_BASE + 0x007C )
   #define LEP_CID_RAD_MLG_LUT                     (LEP_RAD_MODULE_BASE + 0x0088 )
   #define LEP_CID_RAD_THOUSING_TCP                (LEP_RAD_MODULE_BASE + 0x008C )
   #define LEP_CID_RAD_HOUSING_TCP                 (LEP_RAD_MODULE_BASE + 0x008C )
   #define LEP_CID_RAD_SHUTTER_TCP                 (LEP_RAD_MODULE_BASE + 0x0090 )
   #define LEP_CID_RAD_LENS_TCP                    (LEP_RAD_MODULE_BASE + 0x0094 )
   #define LEP_CID_RAD_PREVIOUS_GLOBAL_OFFSET      (LEP_RAD_MODULE_BASE + 0x0098 )
   #define LEP_CID_RAD_PREVIOUS_GLOBAL_GAIN        (LEP_RAD_MODULE_BASE + 0x009C )
   #define LEP_CID_RAD_GLOBAL_GAIN_FFC             (LEP_RAD_MODULE_BASE + 0x00A0 )
   #define LEP_CID_RAD_CNF_SCALE_FACTOR            (LEP_RAD_MODULE_BASE + 0x00A4 )
   #define LEP_CID_RAD_TNF_SCALE_FACTOR            (LEP_RAD_MODULE_BASE + 0x00A8 )
   #define LEP_CID_RAD_SNF_SCALE_FACTOR            (LEP_RAD_MODULE_BASE + 0x00AC )
   #define LEP_CID_RAD_ARBITRARY_OFFSET            (LEP_RAD_MODULE_BASE + 0x00B8 )

   #define LEP_CID_RAD_FLUX_LINEAR_PARAMS          (LEP_RAD_MODULE_BASE + 0x00BC)
   #define LEP_CID_RAD_TLINEAR_ENABLE_STATE        (LEP_RAD_MODULE_BASE + 0x00C0)
   #define LEP_CID_RAD_TLINEAR_RESOLUTION          (LEP_RAD_MODULE_BASE + 0x00C4)
   #define LEP_CID_RAD_TLINEAR_AUTO_RESOLUTION     (LEP_RAD_MODULE_BASE + 0x00C8)
   #define LEP_CID_RAD_SPOTMETER_ROI               (LEP_RAD_MODULE_BASE + 0x00CC)
   #define LEP_CID_RAD_SPOTMETER_OBJ_KELVIN        (LEP_RAD_MODULE_BASE + 0x00D0)

   #define LEP_CID_RAD_RBFO_INTERNAL_LG            (LEP_RAD_MODULE_BASE + 0x00D4 )  /* Low Gain */
   #define LEP_CID_RAD_RBFO_EXTERNAL_LG            (LEP_RAD_MODULE_BASE + 0x00D8 )  /* Low Gain */

   #define LEP_CID_RAD_ARBITRARY_OFFSET_MODE       (LEP_RAD_MODULE_BASE + 0x00DC)
   #define LEP_CID_RAD_ARBITRARY_OFFSET_PARAMS     (LEP_RAD_MODULE_BASE + 0x00E0)

   #define LEP_CID_RAD_RADIO_CAL_VALUES            (LEP_RAD_MODULE_BASE + 0x00E4)


/* SYS Module Command IDs
*/
   #define LEP_SYS_MODULE_BASE                     0x0200

   #define LEP_CID_SYS_PING                        (LEP_SYS_MODULE_BASE + 0x0000 )
   #define LEP_CID_SYS_CAM_STATUS                  (LEP_SYS_MODULE_BASE + 0x0004 )
   #define LEP_CID_SYS_FLIR_SERIAL_NUMBER          (LEP_SYS_MODULE_BASE + 0x0008 )
   #define LEP_CID_SYS_CAM_UPTIME                  (LEP_SYS_MODULE_BASE + 0x000C )
   #define LEP_CID_SYS_AUX_TEMPERATURE_KELVIN      (LEP_SYS_MODULE_BASE + 0x0010 )
   #define LEP_CID_SYS_FPA_TEMPERATURE_KELVIN      (LEP_SYS_MODULE_BASE + 0x0014 )
   #define LEP_CID_SYS_TELEMETRY_ENABLE_STATE      (LEP_SYS_MODULE_BASE + 0x0018 )
   #define LEP_CID_SYS_TELEMETRY_LOCATION          (LEP_SYS_MODULE_BASE + 0x001C )
   #define LEP_CID_SYS_EXECTUE_FRAME_AVERAGE       (LEP_SYS_MODULE_BASE + 0x0020 )
   #define LEP_CID_SYS_NUM_FRAMES_TO_AVERAGE       (LEP_SYS_MODULE_BASE + 0x0024 )
   #define LEP_CID_SYS_CUST_SERIAL_NUMBER          (LEP_SYS_MODULE_BASE + 0x0028 )
   #define LEP_CID_SYS_SCENE_STATISTICS            (LEP_SYS_MODULE_BASE + 0x002C )
   #define LEP_CID_SYS_SCENE_ROI                   (LEP_SYS_MODULE_BASE + 0x0030 )
   #define LEP_CID_SYS_THERMAL_SHUTDOWN_COUNT      (LEP_SYS_MODULE_BASE + 0x0034 )
   #define LEP_CID_SYS_SHUTTER_POSITION            (LEP_SYS_MODULE_BASE + 0x0038 )
   #define LEP_CID_SYS_FFC_SHUTTER_MODE_OBJ        (LEP_SYS_MODULE_BASE + 0x003C )
   #define FLR_CID_SYS_RUN_FFC                     (LEP_SYS_MODULE_BASE + 0x0042 )
   #define LEP_CID_SYS_FFC_STATUS                  (LEP_SYS_MODULE_BASE + 0x0044 )
   #define LEP_CID_SYS_GAIN_MODE                   (LEP_SYS_MODULE_BASE + 0x0048 )
   #define LEP_CID_SYS_FFC_STATE                   (LEP_SYS_MODULE_BASE + 0x004C )
   #define LEP_CID_SYS_GAIN_MODE_OBJ               (LEP_SYS_MODULE_BASE + 0x0050 )
   #define LEP_CID_SYS_GAIN_MODE_DESIRED_FLAG      (LEP_SYS_MODULE_BASE + 0x0054 )
   #define LEP_CID_SYS_BORESIGHT_VALUES            (LEP_SYS_MODULE_BASE + 0x0058 )

/* SYS Module Attribute Limits
*/

   #define LEP_SYS_MAX_FRAMES_TO_AVERAGE               128          /* Scale is 1x    */
   #define LEP_SYS_MAX_SERIAL_NUMBER_CHAR_SIZE         32


   /* VID Module Command IDs
   */
   #define LEP_VID_MODULE_BASE                  0x0300

   #define LEP_CID_VID_POLARITY_SELECT         (LEP_VID_MODULE_BASE + 0x0000 )
   #define LEP_CID_VID_LUT_SELECT              (LEP_VID_MODULE_BASE + 0x0004 )
   #define LEP_CID_VID_LUT_TRANSFER            (LEP_VID_MODULE_BASE + 0x0008 )
   #define LEP_CID_VID_FOCUS_CALC_ENABLE       (LEP_VID_MODULE_BASE + 0x000C )
   #define LEP_CID_VID_FOCUS_ROI               (LEP_VID_MODULE_BASE + 0x0010 )
   #define LEP_CID_VID_FOCUS_THRESHOLD         (LEP_VID_MODULE_BASE + 0x0014 )
   #define LEP_CID_VID_FOCUS_METRIC            (LEP_VID_MODULE_BASE + 0x0018 )
   #define LEP_CID_VID_SBNUC_ENABLE            (LEP_VID_MODULE_BASE + 0x001C )
   #define LEP_CID_VID_GAMMA_SELECT            (LEP_VID_MODULE_BASE + 0x0020 )
   #define LEP_CID_VID_FREEZE_ENABLE           (LEP_VID_MODULE_BASE + 0x0024 )
   #define LEP_CID_VID_BORESIGHT_CALC_ENABLE   (LEP_VID_MODULE_BASE + 0x0028 )
   #define LEP_CID_VID_BORESIGHT_COORDINATES   (LEP_VID_MODULE_BASE + 0x002C )
   #define LEP_CID_VID_VIDEO_OUTPUT_FORMAT     (LEP_VID_MODULE_BASE + 0x0030 )
   #define LEP_CID_VID_LOW_GAIN_COLOR_LUT      (LEP_VID_MODULE_BASE + 0x0034 )

   /* VID Module Attribute Limits
   */


   /* VID Module Object Sizes
   */
   #define LEPTON_COLOR_LUT_SIZE               256     /* 8-bit LUT as 256 x 8-bits */



/******************************************************************************/
/******************************************************************************/
/**                               THE CLASS ITSELF                           **/
/******************************************************************************/
/******************************************************************************/
/*
class LeptonSDKEmb32OEM
{
public:
*/

/******************************************************************************/
/** EXPORTED TYPE DEFINITIONS                                                **/
/******************************************************************************/
/* Types
*/
    #ifdef _STDINT_H
    typedef uint8_t             LEP_UINT8;
    typedef uint8_t             LEP_UCHAR;
    typedef int8_t              LEP_INT8;
    typedef char                LEP_CHAR8;

    typedef uint16_t            LEP_UINT16;
    typedef uint16_t            LEP_USHORT;
    typedef int16_t             LEP_INT16;
    typedef short               LEP_SHORT;

    typedef uint32_t            LEP_UINT32;
    typedef uint32_t            LEP_UINT;
    typedef int32_t             LEP_INT32;
    typedef int                 LEP_INT;

    typedef uint64_t            LEP_UINT64;
    typedef uint64_t            LEP_ULONG64;
    typedef uint32_t            LEP_ULONG32;
    typedef uint32_t            LEP_ULONG;
    typedef int64_t             LEP_LONG32;
    typedef long                LEP_LONG;

    typedef float               LEP_FLOAT32;
    typedef double              LEP_FLOAT64;
    #else
    typedef unsigned char       LEP_UINT8;
    typedef unsigned char       LEP_UCHAR;
    typedef signed char         LEP_INT8;
    typedef char                LEP_CHAR8;

    typedef unsigned short      LEP_UINT16;
    typedef unsigned short      LEP_USHORT;
    typedef signed short        LEP_INT16;
    typedef short               LEP_SHORT;

    typedef unsigned int        LEP_UINT32;
    typedef unsigned int        LEP_UINT;
    typedef signed int          LEP_INT32;
    typedef int                 LEP_INT;

    typedef unsigned long long  LEP_UINT64;
    typedef unsigned long long  LEP_ULONG64;
    typedef unsigned long       LEP_ULONG32;
    typedef unsigned long       LEP_ULONG;
    typedef signed long         LEP_LONG32;
    typedef long                LEP_LONG;

    typedef float               LEP_FLOAT32;
    typedef double              LEP_FLOAT64;
    #endif

    #ifdef _STDBOOL_H
    typedef bool                LEP_BOOL, *LEP_BOOL_PTR;
    #else
    typedef unsigned char       LEP_BOOL, *LEP_BOOL_PTR;
    #endif

    /* NULL
    */
    #ifndef NULL
        #define NULL '\0'
    #endif


    typedef LEP_UINT16          LEP_COMMAND_ID;
    typedef LEP_UINT16          LEP_ATTRIBUTE_T,*LEP_ATTRIBUTE_T_PTR;

    #define LEP_GET_TYPE        0x0000
    #define LEP_SET_TYPE        0x0001
    #define LEP_RUN_TYPE        0x0002

    typedef enum
    {
        LEP_LSB_FIRST=0,
        LEP_MSB_FIRST

    }LEP_BYTE_ORDER_T, *LEP_BYTE_ORDER_T_PTR;

    typedef enum
    {
        LEP_READY = 0,
        LEP_BUSY,
        LEP_WAITING

    }LEP_OPERATION_STATE;

    typedef enum
    {
        LEP_DISABLED = 0,
        LEP_ENABLED

    }LEP_ENABLE_STATE;

    typedef enum
    {
        LEP_OFF = 0,
        LEP_ON

    }LEP_ON_STATE;


    /* Lepton physical tranport interfaces
    */
    typedef enum LEP_CAMERA_PORT_E_TAG
    {
        LEP_CCI_TWI=0,
        LEP_CCI_SPI,
        LEP_END_CCI_PORTS
    }LEP_CAMERA_PORT_E, *LEP_CAMERA_PORT_E_PTR;

    /* Device entries
    */
    typedef enum LEP_PROTOCOL_DEVICE_E_TAG
    {
        /* I2C Devices */
        AARDVARK_I2C = 0,
        DEV_BOARD_FTDI_V2,
        //C232HM_DDHSL_0,
                TCP_IP,

        /* SPI Devices */

        LEP_END_PROTOCOL_DEVICE,
    } LEP_PROTOCOL_DEVICE_E, *LEP_PROTOCOL_DEVICE_E_PTR;

    /* Lepton supported TWI  clock rates
    */
    typedef enum LEP_TWI_CLOCK_RATE_T_TAG
    {
        LEP_TWI_CLOCK_100KHZ=0,
        LEP_TWI_CLOCK_400KHZ,
        LEP_TWI_CLOCK_1MHZ,
        LEP_END_TWI_CLOCK_RATE

    }LEP_TWI_CLOCK_RATE_T, *LEP_TWI_CLOCK_RATE_T_PTR;

    /* Lepton supported SPI  clock rates
    */
    typedef enum LEP_SPI_CLOCK_RATE_T_TAG
    {
        LEP_SPI_CLOCK_2MHZ=0,
        LEP_SPI_CLOCK_10MHZ,
        LEP_SPI_CLOCK_20MHZ,
        LEP_END_SPI_CLOCK_RATE

    }LEP_SPI_CLOCK_RATE_T, *LEP_SPI_CLOCK_RATE_T_PTR;

    /* Communications Port Descriptor Type
    */
    typedef struct  LEP_CAMERA_PORT_DESC_T_TAG
    {
        LEP_UINT16  portID;
        LEP_CAMERA_PORT_E   portType;
        LEP_UINT16  portBaudRate;
        LEP_UINT8 deviceAddress;
    }LEP_CAMERA_PORT_DESC_T, *LEP_CAMERA_PORT_DESC_T_PTR;



/* SDK
*/
   typedef struct LEP_SDK_VERSION_TAG
   {
      LEP_UINT8   major;
      LEP_UINT8   minor;
      LEP_UINT8   build;
      LEP_UINT8   reserved;

   }LEP_SDK_VERSION_T, *LEP_SDK_VERSION_T_PTR;

   typedef enum LEP_SDK_BOOT_STATUS_E_TAG
   {
      LEP_BOOT_STATUS_NOT_BOOTED = 0,
      LEP_BOOT_STATUS_BOOTED = 1,

      LEP_END_BOOT_STATUS,
   }LEP_SDK_BOOT_STATUS_E, *LEP_SDK_BOOT_STATUS_E_PTR;


/*
 * Represents the different result codes the camera can return.
 */
typedef enum Result
{
   LEP_OK                            = 0,     /* Camera ok */
   LEP_COMM_OK                       = LEP_OK, /* Camera comm ok (same as LEP_OK) */

   LEP_ERROR                         = -1,    /* Camera general error */
   LEP_NOT_READY                     = -2,    /* Camera not ready error */
   LEP_RANGE_ERROR                   = -3,    /* Camera range error */
   LEP_CHECKSUM_ERROR                = -4,    /* Camera checksum error */
   LEP_BAD_ARG_POINTER_ERROR         = -5,    /* Camera Bad argument  error */
   LEP_DATA_SIZE_ERROR               = -6,    /* Camera byte count error */
   LEP_UNDEFINED_FUNCTION_ERROR      = -7,    /* Camera undefined function error */
   LEP_FUNCTION_NOT_SUPPORTED        = -8,    /* Camera function not yet supported error */
   LEP_DATA_OUT_OF_RANGE_ERROR       = -9,    /* Camera input DATA is out of valid range error */
   LEP_COMMAND_NOT_ALLOWED           = -11,   /* Camera unable to execute command due to current camera state */

   /* OTP access errors */
   LEP_OTP_WRITE_ERROR               = -15,   /*!< Camera OTP write error */
   LEP_OTP_READ_ERROR                               = -16,   /* double bit error detected (uncorrectible) */

   LEP_OTP_NOT_PROGRAMMED_ERROR      = -18,   /* Flag read as non-zero */

   /* I2C Errors */
   LEP_ERROR_I2C_BUS_NOT_READY       = -20,   /* I2C Bus Error - Bus Not Avaialble */
   LEP_ERROR_I2C_BUFFER_OVERFLOW     = -22,   /* I2C Bus Error - Buffer Overflow */
   LEP_ERROR_I2C_ARBITRATION_LOST    = -23,   /* I2C Bus Error - Bus Arbitration Lost */
   LEP_ERROR_I2C_BUS_ERROR           = -24,   /* I2C Bus Error - General Bus Error */
   LEP_ERROR_I2C_NACK_RECEIVED       = -25,   /* I2C Bus Error - NACK Received */
   LEP_ERROR_I2C_FAIL                = -26,   /* I2C Bus Error - General Failure */

   /* Processing Errors */
   LEP_DIV_ZERO_ERROR                = -80,   /* Attempted div by zero */

   /* Comm Errors */
   LEP_COMM_PORT_NOT_OPEN            = -101,  /* Comm port not open */
   LEP_COMM_INVALID_PORT_ERROR       = -102,  /* Comm port no such port error */
   LEP_COMM_RANGE_ERROR              = -103,  /* Comm port range error */
   LEP_ERROR_CREATING_COMM           = -104,  /* Error creating comm */
   LEP_ERROR_STARTING_COMM           = -105,  /* Error starting comm */
   LEP_ERROR_CLOSING_COMM            = -106,  /* Error closing comm */
   LEP_COMM_CHECKSUM_ERROR           = -107,  /* Comm checksum error */
   LEP_COMM_NO_DEV                   = -108,  /* No comm device */
   LEP_TIMEOUT_ERROR                 = -109,  /* Comm timeout error */
   LEP_COMM_ERROR_WRITING_COMM       = -110,  /* Error writing comm */
   LEP_COMM_ERROR_READING_COMM       = -111,  /* Error reading comm */
   LEP_COMM_COUNT_ERROR              = -112,  /* Comm byte count error */

   /* Other Errors */
   LEP_OPERATION_CANCELED            = -126,  /* Camera operation canceled */
   LEP_UNDEFINED_ERROR_CODE          = -127   /* Undefined error */

} LEP_RESULT;


typedef LEP_UINT16  LEP_AGC_HEQ_EMPTY_COUNT_T, *LEP_AGC_HEQ_EMPTY_COUNT_T_PTR;
typedef LEP_UINT16  LEP_AGC_HEQ_NORMALIZATION_FACTOR_T, *LEP_AGC_HEQ_NORMALIZATION_FACTOR_T_PTR;

/* AGC Enable Enum
*/
typedef enum LEP_AGC_ENABLE_TAG
{
   LEP_AGC_DISABLE = 0,
   LEP_AGC_ENABLE,
   LEP_END_AGC_ENABLE

}LEP_AGC_ENABLE_E, *LEP_AGC_ENABLE_E_PTR;

/* AGC Policy Enum
*/
typedef enum LEP_AGC_POLICY_TAG
{
   LEP_AGC_LINEAR = 0,
   LEP_AGC_HEQ,
   LEP_END_AGC_POLICY

}LEP_AGC_POLICY_E, *LEP_AGC_POLICY_E_PTR;


/* AGC ROI Structure
*/
typedef struct LEP_AGC_ROI_TAG
{
   LEP_UINT16 startCol;
   LEP_UINT16 startRow;
   LEP_UINT16 endCol;
   LEP_UINT16 endRow;

}LEP_AGC_ROI_T, *LEP_AGC_ROI_T_PTR;

/* AGC Histogram Statistics Structure
*/
typedef struct LEP_AGC_HISTOGRAM_STATISTICS_TAG
{
   LEP_UINT16  minIntensity;
   LEP_UINT16  maxIntensity;
   LEP_UINT16  meanIntensity;
   LEP_UINT16  numPixels;

}LEP_AGC_HISTOGRAM_STATISTICS_T, *LEP_AGC_HISTOGRAM_STATISTICS_T_PTR;

/* AGC Output Scale Factor Structure
*/
typedef enum LEP_AGC_SCALE_FACTOR_E_TAG
{
   LEP_AGC_SCALE_TO_8_BITS = 0,
   LEP_AGC_SCALE_TO_14_BITS,

   LEP_AGC_END_SCALE_TO
}LEP_AGC_HEQ_SCALE_FACTOR_E, *LEP_AGC_HEQ_SCALE_FACTOR_E_PTR;


/* OEM
*/

typedef enum LEP_I2C_COMMAND_STATUS_TAG
{
    LEP_I2C_COMMAND_NOT_BUSY = 0,
    LEP_I2C_COMMAND_IS_BUSY,
    LEP_I2C_END_COMMAND_STATUS

}LEP_I2C_COMMAND_STATUS_E, *LEP_I2C_COMMAND_STATUS_E_PTR;

/* Chip Mask Revision: An (8 bit depth) identifier for the chip
** mask revision located in bits 7-0 of the 16-bit word passed.
*/
typedef LEP_UINT16  LEP_OEM_MASK_REVISION_T,*LEP_OEM_MASK_REVISION_T_PTR;
typedef LEP_UINT16  LEP_OEM_FFC_NORMALIZATION_TARGET_T, *LEP_OEM_FFC_NORMALIZATION_TARGET_T_PTR;
typedef LEP_UINT16  LEP_OEM_FRAME_AVERAGE_T, *LEP_OEM_FRAME_AVERAGE_T_PTR;

//typedef LEP_UINT16  LEP_OEM_VENDORID_T;

/* Part Number: A (32 byte string) identifier unique to a
** specific configuration of module; essentially a module
** Configuration ID.
*/
   #if USE_DEPRECATED_PART_NUMBER_INTERFACE
typedef LEP_CHAR8 *LEP_OEM_PART_NUMBER_T, *LEP_OEM_PART_NUMBER_T_PTR;
   #else
typedef struct LEP_OEM_PART_NUMBER_T_TAG
{
   LEP_CHAR8 value[LEP_OEM_MAX_PART_NUMBER_CHAR_SIZE];
} LEP_OEM_PART_NUMBER_T, *LEP_OEM_PART_NUMBER_T_PTR;
   #endif
   #if 0
typedef struct
{
   LEP_CHAR8   value[32];    /* 32-byte string */

}
LEP_OEM_PART_NUMBER_T, *LEP_OEM_PART_NUMBER_T_PTR;
   #endif

/* Software Version ID: A (24 bit depth) identifier for
** the software version stored in OTP.
*/
typedef struct LEP_OEM_SW_VERSION_TAG
{
   LEP_UINT8   gpp_major;
   LEP_UINT8   gpp_minor;
   LEP_UINT8   gpp_build;
   LEP_UINT8   dsp_major;
   LEP_UINT8   dsp_minor;
   LEP_UINT8   dsp_build;
   LEP_UINT16  reserved;

}LEP_OEM_SW_VERSION_T, *LEP_OEM_SW_VERSION_T_PTR;

   #if 0
/* Chip Master ID: A (16 byte depth) identifier for the ISC1103
** signal processing ASIC.
*/
typedef struct LEP_OEM_MASTER_ID_T_TAG
{
   LEP_UINT8  value[16];        /* 16 byte value */

}
LEP_OEM_MASTER_ID_T, *LEP_OEM_MASTER_ID_T_PTR;
   #endif
/* Video Output Enable Enum
*/
typedef enum LEP_OEM_VIDEO_OUTPUT_ENABLE_TAG
{
   LEP_VIDEO_OUTPUT_DISABLE = 0,
   LEP_VIDEO_OUTPUT_ENABLE,
   LEP_END_VIDEO_OUTPUT_ENABLE

}LEP_OEM_VIDEO_OUTPUT_ENABLE_E, *LEP_OEM_VIDEO_OUTPUT_ENABLE_E_PTR;

/* Video Output Format Selection
*/
typedef enum LEP_OEM_VIDEO_OUTPUT_FORMAT_TAG
{
   LEP_VIDEO_OUTPUT_FORMAT_RAW8 = 0,          // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW10,             // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW12,             // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RGB888,            // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RGB666,            // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RGB565,            // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_YUV422_8BIT,       // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW14,             // SUPPORTED in this release
   LEP_VIDEO_OUTPUT_FORMAT_YUV422_10BIT,      // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_USER_DEFINED,      // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW8_2,            // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW8_3,            // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW8_4,            // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW8_5,            // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW8_6,            // To be supported in later release
   LEP_END_VIDEO_OUTPUT_FORMAT
}LEP_OEM_VIDEO_OUTPUT_FORMAT_E, *LEP_OEM_VIDEO_OUTPUT_FORMAT_E_PTR;

/* Video Output Source Selection
*/
typedef enum LEP_OEM_VIDEO_OUTPUT_SOURCE_TAG
{
   LEP_VIDEO_OUTPUT_SOURCE_RAW = 0,         /* Before video processing */
   LEP_VIDEO_OUTPUT_SOURCE_COOKED,        /* Post video processing - NORMAL MODE */
   LEP_VIDEO_OUTPUT_SOURCE_RAMP,          /* Software Ramp pattern - increase in X, Y */
   LEP_VIDEO_OUTPUT_SOURCE_CONSTANT,      /* Software Constant value pattern */
   LEP_VIDEO_OUTPUT_SOURCE_RAMP_H,        /* Software Ramp pattern - increase in X only */
   LEP_VIDEO_OUTPUT_SOURCE_RAMP_V,        /* Software Ramp pattern - increase in Y only */
   LEP_VIDEO_OUTPUT_SOURCE_RAMP_CUSTOM,   /* Software Ramp pattern - uses custom settings */

   /* Additions to support frame averaging, freeze frame, and data buffers
   */
   LEP_VIDEO_OUTPUT_SOURCE_FRAME_CAPTURE,  // Average, Capture frame
   LEP_VIDEO_OUTPUT_SOURCE_FRAME_FREEZE,   // Freeze-Frame Buffer

   /* RESERVED BUFFERS
   */
   LEP_VIDEO_OUTPUT_SOURCE_FRAME_0,        // Reserved DATA Buffer
   LEP_VIDEO_OUTPUT_SOURCE_FRAME_1,        // Reserved DATA Buffer
   LEP_VIDEO_OUTPUT_SOURCE_FRAME_2,        // Reserved DATA Buffer
   LEP_VIDEO_OUTPUT_SOURCE_FRAME_3,        // Reserved DATA Buffer
   LEP_VIDEO_OUTPUT_SOURCE_FRAME_4,        // Reserved DATA Buffer

   LEP_END_VIDEO_OUTPUT_SOURCE

}LEP_OEM_VIDEO_OUTPUT_SOURCE_E, *LEP_OEM_VIDEO_OUTPUT_SOURCE_E_PTR;

/* Video Output Channel Selection
*/
typedef enum LEP_OEM_VIDEO_OUTPUT_CHANNEL_TAG
{
   LEP_VIDEO_OUTPUT_CHANNEL_MIPI = 0,
   LEP_VIDEO_OUTPUT_CHANNEL_VOSPI,
   LEP_END_VIDEO_OUTPUT_CHANNEL

}LEP_OEM_VIDEO_OUTPUT_CHANNEL_E, *LEP_OEM_VIDEO_OUTPUT_CHANNEL_E_PTR;

/* Video Gamma Enable Enum
*/
typedef enum LEP_OEM_VIDEO_GAMMA_ENABLE_TAG
{
   LEP_VIDEO_GAMMA_DISABLE = 0,
   LEP_VIDEO_GAMMA_ENABLE,
   LEP_END_VIDEO_GAMMA_ENABLE

}LEP_OEM_VIDEO_GAMMA_ENABLE_E, *LEP_OEM_VIDEO_GAMMA_ENABLE_E_PTR;

typedef enum LEP_OEM_MEM_BUFFER_E_TAG
{
   LEP_OEM_MEM_OTP_ODAC = 0,
   LEP_OEM_MEM_OTP_GAIN,
   LEP_OEM_MEM_OTP_OFFSET_0,
   LEP_OEM_MEM_OTP_OFFSET_1,
   LEP_OEM_MEM_OTP_FFC,
   LEP_OEM_MEM_OTP_LG0,
   LEP_OEM_MEM_OTP_LG1,
   LEP_OEM_MEM_OTP_LG2,
   LEP_OEM_MEM_OTP_TFPA_LUT,
   LEP_OEM_MEM_OTP_TAUX_LUT,
   LEP_OEM_MEM_OTP_BAD_PIXEL_LIST,
   LEP_OEM_MEM_SRAM_ODAC,
   LEP_OEM_MEM_SRAM_BAD_PIXEL_LIST,
   LEP_OEM_MEM_SHARED_BUFFER_0,
   LEP_OEM_MEM_SHARED_BUFFER_1,
   LEP_OEM_MEM_SHARED_BUFFER_2,
   LEP_OEM_MEM_SHARED_BUFFER_3,
   LEP_OEM_MEM_SHARED_BUFFER_4,
   LEP_OEM_END_MEM_BUFFERS

}LEP_OEM_MEM_BUFFER_E,*LEP_OEM_MEM_BUFFER_E_PTR;

typedef enum
{
   LEP_OEM_STATUS_OTP_WRITE_ERROR = -2,
   LEP_OEM_STATUS_ERROR = -1,
   LEP_OEM_STATUS_READY = 0,
   LEP_OEM_STATUS_BUSY,
   LEP_OEM_FRAME_AVERAGE_COLLECTING_FRAMES,
   LEP_OEM_STATUS_END

} LEP_OEM_STATUS_E, *LEP_OEM_STATUS_E_PTR;

typedef enum LEP_OEM_STATE_E_TAG
{
   LEP_OEM_DISABLE = 0,
   LEP_OEM_ENABLE,
   LEP_OEM_END_STATE

}LEP_OEM_STATE_E,*LEP_OEM_STATE_E_PTR;

typedef enum LEP_OEM_POWER_STATE_E_TAG
{
   LEP_OEM_POWER_MODE_NORMAL = 0,
   LEP_OEM_POWER_MODE_LOW_POWER_1,
   LEP_OEM_POWER_MODE_LOW_POWER_2,
   LEP_OEM_END_POWER_MODE,

}LEP_OEM_POWER_STATE_E, *LEP_OEM_POWER_STATE_E_PTR;

typedef enum LEP_OEM_VSYNC_DELAY_E_TAG
{
   LEP_OEM_VSYNC_DELAY_MINUS_3 = -3,
   LEP_OEM_VSYNC_DELAY_MINUS_2 = -2,
   LEP_OEM_VSYNC_DELAY_MINUS_1 = -1,
   LEP_OEM_VSYNC_DELAY_NONE = 0,
   LEP_OEM_VSYNC_DELAY_PLUS_1 = 1,
   LEP_OEM_VSYNC_DELAY_PLUS_2 = 2,
   LEP_OEM_VSYNC_DELAY_PLUS_3 = 3,

   LEP_END_OEM_VSYNC_DELAY
} LEP_OEM_VSYNC_DELAY_E, *LEP_OEM_VSYNC_DELAY_E_PTR;

typedef enum LEP_OEM_GPIO_MODE_E_TAG
{
   LEP_OEM_GPIO_MODE_GPIO = 0,
   LEP_OEM_GPIO_MODE_I2C_MASTER = 1,
   LEP_OEM_GPIO_MODE_SPI_MASTER_VLB_DATA = 2,
   LEP_OEM_GPIO_MODE_SPIO_MASTER_REG_DATA = 3,
   LEP_OEM_GPIO_MODE_SPI_SLAVE_VLB_DATA = 4,
   LEP_OEM_GPIO_MODE_VSYNC = 5,

   LEP_OEM_END_GPIO_MODE,
}LEP_OEM_GPIO_MODE_E, *LEP_OEM_GPIO_MODE_E_PTR;

typedef enum LEP_OEM_USER_PARAMS_STATE_E_TAG
{
   LEP_OEM_USER_PARAMS_STATE_NOT_WRITTEN = 0,
   LEP_OEM_USER_PARAMS_STATE_WRITTEN,

   LEP_OEM_END_USER_PARAMS_STATE,

}LEP_OEM_USER_PARAMS_STATE_E, *LEP_OEM_USER_PARAMS_STATE_E_PTR;

/* Shutter Profile Object
*/
typedef struct LEP_OEM_SHUTTER_PROFILE_OBJ_T_TAG
{
   LEP_UINT16 closePeriodInFrames;           /* in frame counts x1 */
   LEP_UINT16 openPeriodInFrames;            /* in frame counts x1 */

}LEP_OEM_SHUTTER_PROFILE_OBJ_T, *LEP_OEM_SHUTTER_PROFILE_OBJ_T_PTR;

typedef struct LEP_OEM_BAD_PIXEL_REPLACE_CONTROL_T_TAG
{
   LEP_OEM_STATE_E oemBadPixelReplaceEnable;          // Bad Pixel Replacment in the video path

}LEP_OEM_BAD_PIXEL_REPLACE_CONTROL_T, *LEP_OEM_BAD_PIXEL_REPLACE_CONTROL_T_PTR;
typedef struct LEP_OEM_TEMPORAL_FILTER_CONTROL_T_TAG
{
   LEP_OEM_STATE_E oemTemporalFilterEnable;           // Temporal Filter in the video path

}LEP_OEM_TEMPORAL_FILTER_CONTROL_T, *LEP_OEM_TEMPORAL_FILTER_CONTROL_T_PTR;

typedef struct LEP_OEM_COLUMN_NOISE_ESTIMATE_CONTROL_T_TAG
{
   LEP_OEM_STATE_E oemColumnNoiseEstimateEnable;      // Column Noise Estimate in the video path

}LEP_OEM_COLUMN_NOISE_ESTIMATE_CONTROL_T, *LEP_OEM_COLUMN_NOISE_ESTIMATE_CONTROL_T_PTR;

typedef struct LEP_OEM_PIXEL_NOISE_SETTINGS_T_TAG
{
   LEP_OEM_STATE_E oemPixelNoiseEstimateEnable;         // Row Noise Estimate in the video path

}LEP_OEM_PIXEL_NOISE_SETTINGS_T, *LEP_OEM_PIXEL_NOISE_SETTINGS_T_PTR;

typedef struct LEP_OEM_THERMAL_SHUTDOWN_ENABLE_T_TAG
{
   LEP_OEM_STATE_E oemThermalShutdownEnable;

}LEP_OEM_THERMAL_SHUTDOWN_ENABLE_T, *LEP_OEM_THERMAL_SHUTDOWN_ENABLE_T_PTR;


/* RAD
*/
typedef LEP_UINT16   LEP_RAD_RS_T, *LEP_RAD_RS_T_PTR;
typedef LEP_UINT16   LEP_RAD_FNUMBER_T, *LEP_RAD_FNUMBER_T_PTR;
typedef LEP_UINT16   LEP_RAD_TAULENS_T, *LEP_RAD_TAULENS_T_PTR;

typedef LEP_UINT16   LEP_RAD_FNUM_SHUTTER_T, *LEP_RAD_FNUM_SHUTTER_T_PTR;
typedef LEP_UINT16   LEP_RAD_RADIOMETRY_FILTER_T, *LEP_RAD_RADIOMETRY_FILTER_T_PTR;
typedef LEP_UINT16   LEP_RAD_MEDIAN_VALUE_T, *LEP_RAD_MEDIAN_VALUE_T_PTR;
typedef LEP_UINT16   LEP_RAD_PARAMETER_SCALE_FACTOR_T, *LEP_RAD_PARAMETER_SCALE_FACTOR_T_PTR;
typedef LEP_INT16    LEP_RAD_ARBITRARY_OFFSET_T, *LEP_RAD_ARBITRARY_OFFSET_T_PTR;
typedef LEP_UINT16   LEP_RAD_SPOTMETER_KELVIN_T, *LEP_RAD_SPOTMETER_KELVIN_T_PTR;


/* TFpa and TAux counts
*/
typedef LEP_UINT16  LEP_RAD_TEMPERATURE_COUNTS_T, *LEP_RAD_TEMPERATURE_COUNTS_T_PTR;

/* Standard temperature type for temperatures in degrees C
*  These are scaled by 100 so: 100xDegC as [s15.0]
*/
typedef LEP_UINT16  LEP_RAD_KELVIN_T, *LEP_RAD_KELVIN_T_PTR;

/* Flux format: s32 1000x
*/
typedef LEP_INT32   LEP_RAD_FLUX_T, *LEP_RAD_FLUX_T_PTR;

/* Global Gain  [3.13]
*/
typedef LEP_UINT16  LEP_RAD_GLOBAL_GAIN_T, *LEP_RAD_GLOBAL_GAIN_T_PTR;


/* Global Offset  [14.0]
*/
typedef LEP_UINT16  LEP_RAD_GLOBAL_OFFSET_T, *LEP_RAD_GLOBAL_OFFSET_T_PTR;

/* LUTs
*/
typedef LEP_INT16   LEP_RAD_SIGNED_LUT128_T, *LEP_RAD_SIGNED_LUT128_T_PTR;
typedef LEP_UINT16  LEP_RAD_LUT128_T, *LEP_RAD_LUT128_T_PTR;
typedef LEP_UINT16  LEP_RAD_LUT256_T, *LEP_RAD_LUT256_T_PTR;

/* Alpha and Beta
*/
typedef LEP_INT16   LEP_RAD_ALPHA_T, *LEP_RAD_ALPHA_T_PTR;
typedef LEP_INT16   LEP_RAD_BETA_T, *LEP_RAD_BETA_T_PTR;

/* TShutter Mode
*/
typedef enum LEP_RAD_TS_MODE_E_TAG
{
   LEP_RAD_TS_USER_MODE = 0,
   LEP_RAD_TS_CAL_MODE,
   LEP_RAD_TS_FIXED_MODE,
   LEP_RAD_TS_END_TS_MODE

}LEP_RAD_TS_MODE_E, *LEP_RAD_TS_MODE_E_PTR;

/* RBFO
*/
typedef struct LEP_RBFO_T_TAG
{
   LEP_UINT32 RBFO_R;   // value is not scaled
   LEP_UINT32 RBFO_B;   // value is scaled by X  << n
   LEP_UINT32 RBFO_F;
   LEP_INT32  RBFO_O;

}LEP_RBFO_T, *LEP_RBFO_T_PTR;

/* Linear Temperature correction parameters
*/
typedef struct LEP_RAD_LINEAR_TEMP_CORRECTION_T_TAG
{
   LEP_INT16   offset;     // [s8.7]
   LEP_INT16   gainAux;    // [s2.13]
   LEP_INT16   gainShutter; // [s2.13]
   LEP_UINT16  pad;

}LEP_RAD_LINEAR_TEMP_CORRECTION_T, *LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR;

/* Radiometry Enable state
*/
typedef enum LEP_RAD_ENABLE_E_TAG
{
   LEP_RAD_DISABLE = 0,
   LEP_RAD_ENABLE,
   LEP_END_RAD_ENABLE

}LEP_RAD_ENABLE_E, *LEP_RAD_ENABLE_E_PTR;

/* Temperature TFpa and TAux counts Update Mode
*/
typedef enum LEP_RAD_TEMPERATURE_UPDATE_E_TAG
{
   LEP_RAD_NORMAL_UPDATE = 0,
   LEP_RAD_NO_UPDATE,         // Fixed to last value
   LEP_RAD_UPDATE_END

}LEP_RAD_TEMPERATURE_UPDATE_E, *LEP_RAD_TEMPERATURE_UPDATE_E_PTR;

/* Run operation status
*/
typedef enum
{
   LEP_RAD_STATUS_ERROR = -1,
   LEP_RAD_STATUS_READY = 0,
   LEP_RAD_STATUS_BUSY,
   LEP_RAD_FRAME_AVERAGE_COLLECTING_FRAMES,
   LEP_RAD_STATUS_END

} LEP_RAD_STATUS_E, *LEP_RAD_STATUS_E_PTR;
typedef struct LEP_RAD_FLUX_LINEAR_PARAMS_T_TAG
{
   /* Type     Field name              format   default  range       comment*/
   LEP_UINT16  sceneEmissivity;     /* 3.13     8192     [82, 8192] */
   LEP_UINT16  TBkgK;               /* 16.0     30000    [, ]        value in kelvin 100x*/
   LEP_UINT16  tauWindow;           /* 3.13     8192     [82, 8192] */
   LEP_UINT16  TWindowK;            /* 16.0     30000    [, ]        value in kelvin 100x*/
   LEP_UINT16  tauAtm;              /* 3.13     8192     [82, 8192] */
   LEP_UINT16  TAtmK;               /* 16.0     30000    [, ]        value in kelvin 100x*/
   LEP_UINT16  reflWindow;          /* 3.13     0        [0, 8192-tauWindow] */
   LEP_UINT16  TReflK;              /* 16.0     30000    [, ]        value in kelvin 100x*/

}LEP_RAD_FLUX_LINEAR_PARAMS_T, *LEP_RAD_FLUX_LINEAR_PARAMS_T_PTR;

typedef enum LEP_RAD_TLINEAR_RESOLUTION_E_TAG
{
   LEP_RAD_RESOLUTION_0_1 = 0,
   LEP_RAD_RESOLUTION_0_01,

   LEP_RAD_END_RESOLUTION,
} LEP_RAD_TLINEAR_RESOLUTION_E, *LEP_RAD_TLINEAR_RESOLUTION_E_PTR;

typedef struct LEP_RAD_ROI_T_TAG
{
   LEP_UINT16 startRow;
   LEP_UINT16 startCol;
   LEP_UINT16 endRow;
   LEP_UINT16 endCol;
} LEP_RAD_ROI_T, *LEP_RAD_ROI_T_PTR;

typedef enum LEP_RAD_ARBITRARY_OFFSET_MODE_E_TAG
{
   LEP_RAD_ARBITRARY_OFFSET_MODE_MANUAL = 0,
   LEP_RAD_ARBITRARY_OFFSET_MODE_AUTO,
   LEP_RAD_END_ARBITRARY_OFFSET_MODE,

} LEP_RAD_ARBITRARY_OFFSET_MODE_E, *LEP_RAD_ARBITRARY_OFFSET_MODE_E_PTR;

typedef struct LEP_RAD_ARBITRARY_OFFSET_PARAMS_T_TAG
{
   LEP_INT16 amplitude;
   LEP_UINT16 decay;

} LEP_RAD_ARBITRARY_OFFSET_PARAMS_T, *LEP_RAD_ARBITRARY_OFFSET_PARAMS_T_PTR;

typedef struct LEP_RAD_SPOTMETER_OBJ_KELVIN_T_TAG
{
    LEP_RAD_SPOTMETER_KELVIN_T  radSpotmeterValue;
    LEP_UINT16                  radSpotmeterMaxValue;
    LEP_UINT16                  radSpotmeterMinValue;
    LEP_UINT16                  radSpotmeterPopulation;

} LEP_RAD_SPOTMETER_OBJ_KELVIN_T, *LEP_RAD_SPOTMETER_OBJ_KELVIN_T_PTR;

typedef struct LEP_RAD_RADIO_CAL_VALUES_T_TAG
{

   LEP_RAD_TEMPERATURE_COUNTS_T  radTauxCounts;
   LEP_RAD_TEMPERATURE_COUNTS_T  radTfpaCounts;

   LEP_RAD_KELVIN_T              radTauxKelvin;
   LEP_RAD_KELVIN_T              radTfpaKelvin;

} LEP_RAD_RADIO_CAL_VALUES_T, *LEP_RAD_RADIO_CAL_VALUES_T_PTR;


/* SYS
*/
typedef LEP_UINT64  LEP_SYS_FLIR_SERIAL_NUMBER_T, *LEP_SYS_FLIR_SERIAL_NUMBER_T_PTR;
typedef LEP_UINT32  LEP_SYS_UPTIME_NUMBER_T, *LEP_SYS_UPTIME_NUMBER_T_PTR;
typedef LEP_FLOAT32 LEP_SYS_AUX_TEMPERATURE_CELCIUS_T, *LEP_SYS_AUX_TEMPERATURE_CELCIUS_T_PTR;
typedef LEP_FLOAT32 LEP_SYS_FPA_TEMPERATURE_CELCIUS_T, *LEP_SYS_FPA_TEMPERATURE_CELCIUS_T_PTR;
typedef LEP_UINT16  LEP_SYS_AUX_TEMPERATURE_KELVIN_T, *LEP_SYS_AUX_TEMPERATURE_KELVIN_T_PTR;
typedef LEP_UINT16  LEP_SYS_FPA_TEMPERATURE_KELVIN_T, *LEP_SYS_FPA_TEMPERATURE_KELVIN_T_PTR;
//typedef LEP_UINT8   LEP_SYS_NUM_AVERAGE_FRAMES_T, *LEP_SYS_NUM_AVERAGE_FRAMES_T_PTR;
typedef LEP_UINT16  LEP_SYS_THERMAL_SHUTDOWN_COUNTS_T, *LEP_SYS_THERMAL_SHUTDOWN_COUNTS_T_PTR;
typedef LEP_UINT16  LEP_SYS_THRESHOLD_T, *LEP_SYS_THRESHOLD_T_PTR;

   #if USE_DEPRECATED_SERIAL_NUMBER_INTERFACE
typedef LEP_CHAR8 *LEP_SYS_CUST_SERIAL_NUMBER_T, *LEP_SYS_CUST_SERIAL_NUMBER_T_PTR;
   #else
typedef struct LEP_SYS_CUST_SERIAL_NUMBER_T_TAG
{
   LEP_CHAR8 value[LEP_SYS_MAX_SERIAL_NUMBER_CHAR_SIZE];
} LEP_SYS_CUST_SERIAL_NUMBER_T, *LEP_SYS_CUST_SERIAL_NUMBER_T_PTR;
   #endif

/* SYS Camera System Status Enum
   Captures basic camera operation
*/
typedef enum LEP_SYSTEM_STATUS_STATES_E_TAG
{
   LEP_SYSTEM_READY = 0,
   LEP_SYSTEM_INITIALIZING,
   LEP_SYSTEM_IN_LOW_POWER_MODE,
   LEP_SYSTEM_GOING_INTO_STANDBY,
   LEP_SYSTEM_FLAT_FIELD_IN_PROCESS,
   LEP_SYSTEM_FLAT_FIELD_IMMINENT,
   LEP_SYSTEM_THERMAL_SHUTDOWN_IMMINENT,

   LEP_SYSTEM_END_STATES

}LEP_SYSTEM_STATUS_STATES_E, *LEP_SYSTEM_STATUS_STATES_E_PTR;

typedef struct LEP_STATUS_T_TAG
{
   LEP_SYSTEM_STATUS_STATES_E  camStatus;
   LEP_UINT16                  commandCount;
   LEP_UINT16                  reserved;

}LEP_STATUS_T, *LEP_STATUS_T_PTR;

typedef enum LEP_SYS_TELEMETRY_ENABLE_STATE_E_TAG
{
   LEP_TELEMETRY_DISABLED = 0,
   LEP_TELEMETRY_ENABLED,
   LEP_END_TELEMETRY_ENABLE_STATE

}LEP_SYS_TELEMETRY_ENABLE_STATE_E, *LEP_SYS_TELEMETRY_ENABLE_STATE_E_PTR;

typedef enum LEP_SYS_TELEMETRY_LOCATION_E_TAG
{
   LEP_TELEMETRY_LOCATION_HEADER = 0,
   LEP_TELEMETRY_LOCATION_FOOTER,
   LEP_END_TELEMETRY_LOCATION

}LEP_SYS_TELEMETRY_LOCATION_E, *LEP_SYS_TELEMETRY_LOCATION_E_PTR;

typedef enum LEP_SYS_FRAME_AVERAGE_DIVISOR_E_TAG
{
   LEP_SYS_FA_DIV_1 = 0,
   LEP_SYS_FA_DIV_2,
   LEP_SYS_FA_DIV_4,
   LEP_SYS_FA_DIV_8,
   LEP_SYS_FA_DIV_16,
   LEP_SYS_FA_DIV_32,
   LEP_SYS_FA_DIV_64,
   LEP_SYS_FA_DIV_128,
   LEP_SYS_END_FA_DIV

}LEP_SYS_FRAME_AVERAGE_DIVISOR_E, *LEP_SYS_FRAME_AVERAGE_DIVISOR_E_PTR;

typedef struct LEP_SYS_SCENE_STATISTICS_T_TAG
{
   LEP_UINT16   meanIntensity;
   LEP_UINT16   maxIntensity;
   LEP_UINT16   minIntensity;
   LEP_UINT16   numPixels;

} LEP_SYS_SCENE_STATISTICS_T, *LEP_SYS_SCENE_STATISTICS_T_PTR;

typedef struct LEP_SYS_BAD_PIXEL_T_TAG
{
   LEP_UINT8 row;
   LEP_UINT8 col;
   LEP_UINT8 value;
   LEP_UINT8 value2;

}LEP_SYS_BAD_PIXEL_T, *LEP_SYS_BAD_PIXEL_T_PTR;

typedef struct LEP_SYS_VIDEO_ROI_T_TAG
{
   LEP_UINT16 startCol;
   LEP_UINT16 startRow;
   LEP_UINT16 endCol;
   LEP_UINT16 endRow;
} LEP_SYS_VIDEO_ROI_T, *LEP_SYS_VIDEO_ROI_T_PTR;

typedef enum LEP_SYS_ENABLE_E_TAG
{
   LEP_SYS_DISABLE = 0,
   LEP_SYS_ENABLE,

   LEP_END_SYS_ENABLE

}LEP_SYS_ENABLE_E, *LEP_SYS_ENABLE_E_PTR;

typedef enum LEP_SYS_SHUTTER_POSITION_E_TAG
{
   LEP_SYS_SHUTTER_POSITION_UNKNOWN = -1,
   LEP_SYS_SHUTTER_POSITION_IDLE = 0,
   LEP_SYS_SHUTTER_POSITION_OPEN,
   LEP_SYS_SHUTTER_POSITION_CLOSED,
   LEP_SYS_SHUTTER_POSITION_BRAKE_ON,
   LEP_SYS_SHUTTER_POSITION_END

}LEP_SYS_SHUTTER_POSITION_E,  *LEP_SYS_SHUTTER_POSITION_E_PTR;

typedef enum LEP_SYS_FFC_SHUTTER_MODE_E_TAG
{
   LEP_SYS_FFC_SHUTTER_MODE_MANUAL = 0,
   LEP_SYS_FFC_SHUTTER_MODE_AUTO,
   LEP_SYS_FFC_SHUTTER_MODE_EXTERNAL,

   LEP_SYS_FFC_SHUTTER_MODE_END

}LEP_SYS_FFC_SHUTTER_MODE_E, *LEP_SYS_FFC_SHUTTER_MODE_E_PTR;

typedef enum LEP_SYS_SHUTTER_TEMP_LOCKOUT_STATE_E_TAG
{
   LEP_SYS_SHUTTER_LOCKOUT_INACTIVE = 0,  /* not locked out */
   LEP_SYS_SHUTTER_LOCKOUT_HIGH,    /* lockout due to high temp */
   LEP_SYS_SHUTTER_LOCKOUT_LOW,     /* lockout due to low temp */

}LEP_SYS_SHUTTER_TEMP_LOCKOUT_STATE_E,*LEP_SYS_SHUTTER_TEMP_LOCKOUT_STATE_E_PTR;

   #if 0
typedef struct LEP_SYS_FFC_SHUTTER_TEMP_LOCKOUT_T_TAG
{
   LEP_UINT16  lowTempThreshold;             /* in Kelvin */
   LEP_UINT16  highTempThreshold;            /* in Kelvin */
   LEP_SYS_SHUTTER_TEMP_LOCKOUT_STATE_E tempLockoutState;
}
LEP_SYS_FFC_SHUTTER_TEMP_LOCKOUT_T, *LEP_SYS_FFC_SHUTTER_TEMP_LOCKOUT_T_PTR;
   #endif

typedef struct LEP_SYS_FFC_SHUTTER_MODE_OBJ_T_TAG
{
   LEP_SYS_FFC_SHUTTER_MODE_E shutterMode;   /* defines current mode */

   LEP_SYS_SHUTTER_TEMP_LOCKOUT_STATE_E   tempLockoutState;
   LEP_SYS_ENABLE_E videoFreezeDuringFFC;
   LEP_SYS_ENABLE_E ffcDesired;              /* status of FFC desired */
   LEP_UINT32 elapsedTimeSinceLastFfc;       /* in milliseconds x1 */
   LEP_UINT32 desiredFfcPeriod;              /* in milliseconds x1 */
   LEP_BOOL       explicitCmdToOpen;             /* true or false */
   LEP_UINT16 desiredFfcTempDelta;           /* in Kelvin x100  */
   LEP_UINT16 imminentDelay;                 /* in frame counts x1 */


}LEP_SYS_FFC_SHUTTER_MODE_OBJ_T, *LEP_SYS_FFC_SHUTTER_MODE_OBJ_T_PTR;

/* SYS  Status Enum
   Captures the FFC operation status
*/
typedef enum LEP_SYS_STATUS_E_TAG
{
   LEP_SYS_STATUS_WRITE_ERROR = -2,
   LEP_SYS_STATUS_ERROR = -1,
   LEP_SYS_STATUS_READY = 0,
   LEP_SYS_STATUS_BUSY,
   LEP_SYS_FRAME_AVERAGE_COLLECTING_FRAMES,
   LEP_SYS_STATUS_END

} LEP_SYS_STATUS_E, *LEP_SYS_STATUS_E_PTR;

typedef enum LEP_SYS_GAIN_MODE_E_TAG
{
   LEP_SYS_GAIN_MODE_HIGH = 0,
   LEP_SYS_GAIN_MODE_LOW,
   LEP_SYS_GAIN_MODE_AUTO,

   LEP_SYS_END_GAIN_MODE,
} LEP_SYS_GAIN_MODE_E, *LEP_SYS_GAIN_MODE_E_PTR;


/* System Gain Mode ROI Structure
*/
typedef struct LEP_SYS_GAIN_MODE_ROI_T_TAG
{
   LEP_UINT16 startCol;
   LEP_UINT16 startRow;
   LEP_UINT16 endCol;
   LEP_UINT16 endRow;

}LEP_SYS_GAIN_MODE_ROI_T, *LEP_SYS_GAIN_MODE_ROI_T_PTR;

/* Gain Mode Support
*/
typedef struct LEP_SYS_GAIN_MODE_THRESHOLDS_T_TAG
{
   LEP_SYS_THRESHOLD_T sys_P_high_to_low;    /* Range: [0 <96> 100], percent   */
   LEP_SYS_THRESHOLD_T sys_P_low_to_high;    /* Range: [0 <96> 100], percent   */

   LEP_SYS_THRESHOLD_T sys_C_high_to_low;    /* Range: [0-600], degrees C   */
   LEP_SYS_THRESHOLD_T sys_C_low_to_high;    /* Range: [0-600], degrees C   */

   LEP_SYS_THRESHOLD_T sys_T_high_to_low;    /* Range: [0-900], Kelvin   */
   LEP_SYS_THRESHOLD_T sys_T_low_to_high;    /* Range: [0-900], Kelvin   */

}LEP_SYS_GAIN_MODE_THRESHOLDS_T, *LEP_SYS_GAIN_MODE_THRESHOLDS_T_PTR;

/* Gain Mode Object
*/
typedef struct LEP_SYS_GAIN_MODE_OBJ_T_TAG
{
   LEP_SYS_GAIN_MODE_ROI_T          sysGainModeROI;         /* Specified ROI to use for Gain Mode switching */
   LEP_SYS_GAIN_MODE_THRESHOLDS_T   sysGainModeThresholds;  /* Set of threshold triggers */
   LEP_UINT16                       sysGainRoiPopulation;   /* Population size in pixels within the ROI */
   LEP_UINT16                       sysGainModeTempEnabled; /* True if T-Linear is implemented */
   LEP_UINT16                       sysGainModeFluxThresholdLow;     /* calculated from desired temp */
   LEP_UINT16                       sysGainModeFluxThresholdHigh;    /* calculated from desired temp */

}LEP_SYS_GAIN_MODE_OBJ_T, *LEP_SYS_GAIN_MODE_OBJ_T_PTR;

/* SYS FFC States Enum
   Captures the current camera FFC operation state
*/
typedef enum LEP_SYS_FFC_STATES_E_TAG
{
   LEP_SYS_FFC_NEVER_COMMANDED = 0,
   LEP_SYS_FFC_IMMINENT,
   LEP_SYS_FFC_IN_PROCESS,
   LEP_SYS_FFC_DONE,

   LEP_SYS_END_FFC_STATES

}LEP_SYS_FFC_STATES_E, *LEP_SYS_FFC_STATES_E_PTR;

typedef struct LEP_SYS_BORESIGHT_VALUES_T_TAG
{
    LEP_UINT16  targetRow;
    LEP_UINT16  targetCol;
    LEP_INT16   targetRotation;
    LEP_INT16   fovX;
    LEP_INT16   fovY;

} LEP_SYS_BORESIGHT_VALUES_T, *LEP_SYS_BORESIGHT_VALUES_T_PTR;


/* VID
*/
   typedef LEP_UINT32 LEP_VID_FOCUS_METRIC_T, *LEP_VID_FOCUS_METRIC_T_PTR;
   typedef LEP_UINT32 LEP_VID_FOCUS_METRIC_THRESHOLD_T, *LEP_VID_FOCUS_METRIC_THRESHOLD_T_PTR;


   typedef enum LEP_POLARITY_E_TAG
   {
      LEP_VID_WHITE_HOT=0,
      LEP_VID_BLACK_HOT,
      LEP_VID_END_POLARITY

   }LEP_POLARITY_E, *LEP_POLARITY_E_PTR;


   /* Video Pseudo color LUT Enum
   */
   typedef enum LEP_PCOLOR_LUT_E_TAG
   {
      LEP_VID_WHEEL6_LUT=0,
      LEP_VID_FUSION_LUT,
      LEP_VID_RAINBOW_LUT,
      LEP_VID_GLOBOW_LUT,
      LEP_VID_SEPIA_LUT,
      LEP_VID_COLOR_LUT,
      LEP_VID_ICE_FIRE_LUT,
      LEP_VID_RAIN_LUT,
      LEP_VID_USER_LUT,
      LEP_VID_END_PCOLOR_LUT

   }LEP_PCOLOR_LUT_E, *LEP_PCOLOR_LUT_E_PTR;

   /* User-Defined color look-up table (LUT)
   */
   typedef struct LEP_VID_LUT_PIXEL_T_TAG
   {
      LEP_UINT8 reserved;
      LEP_UINT8 red;
      LEP_UINT8 green;
      LEP_UINT8 blue;
   } LEP_VID_LUT_PIXEL_T, *LEP_VID_LUT_PIXEL_T_PTR;
   typedef struct LEP_VID_LUT_BUFFER_T_TAG
   {
      LEP_VID_LUT_PIXEL_T bin[256];
   } LEP_VID_LUT_BUFFER_T, *LEP_VID_LUT_BUFFER_T_PTR;

#if 0
   typedef struct
   {
      LEP_UINT8 bin[LEPTON_COLOR_LUT_SIZE];

   }LEP_VID_LUT_BUFFER_T, *LEP_VID_LUT_BUFFER_T_PTR;
#endif

   /* Video Focus Metric Calculation Enable Enum
   */
   typedef enum LEP_VID_ENABLE_TAG
   {
      LEP_VID_FOCUS_CALC_DISABLE=0,
      LEP_VID_FOCUS_CALC_ENABLE,
      LEP_VID_END_FOCUS_CALC_ENABLE

   }LEP_VID_FOCUS_CALC_ENABLE_E, *LEP_VID_FOCUS_CALC_ENABLE_E_PTR;


   /* Video Focus ROI Structure
   */
   typedef struct LEP_VID_FOCUS_ROI_TAG
   {
      LEP_UINT16 startCol;
      LEP_UINT16 startRow;
      LEP_UINT16 endCol;
      LEP_UINT16 endRow;

   }LEP_VID_FOCUS_ROI_T, *LEP_VID_FOCUS_ROI_T_PTR;


   /* Video Scene-Based NUC Enable Enum
   */
   typedef enum LEP_VID_SBNUC_ENABLE_TAG
   {
      LEP_VID_SBNUC_DISABLE = 0,
      LEP_VID_SBNUC_ENABLE,
      LEP_VID_END_SBNUC_ENABLE

   }LEP_VID_SBNUC_ENABLE_E, *LEP_VID_SBNUC_ENABLE_E_PTR ;

   /* Video Freeze Output Enable Enum
   */
   typedef enum LEP_VID_FREEZE_ENABLE_TAG
   {
      LEP_VID_FREEZE_DISABLE = 0,
      LEP_VID_FREEZE_ENABLE,
      LEP_VID_END_FREEZE_ENABLE

   }LEP_VID_FREEZE_ENABLE_E, *LEP_VID_FREEZE_ENABLE_E_PTR ;

   typedef enum LEP_VID_BORESIGHT_CALC_ENABLE_STATE_E_TAG
   {
      LEP_VID_BORESIGHT_CALC_DISABLED = 0,
      LEP_VID_BORESIGHT_CALC_ENABLED,

      LEP_VID_END_BORESIGHT_CALC_ENABLE_STATE,
   } LEP_VID_BORESIGHT_CALC_ENABLE_STATE_E, *LEP_VID_BORESIGHT_CALC_ENABLE_STATE_E_PTR;

   typedef struct LEP_PIXEL_COORDINATE_T_TAG
   {
      LEP_UINT16  row;
      LEP_UINT16  col;

   }LEP_PIXEL_COORDINATE_T, *LEP_PIXEL_COORDINATE_T_PTR;

   typedef struct LEP_VID_BORESIGHT_COORDINATES_T_TAG
   {
      LEP_PIXEL_COORDINATE_T   top_0;        /* Top row 0 */
      LEP_PIXEL_COORDINATE_T   top_1;        /* Top row 1 */
      LEP_PIXEL_COORDINATE_T   bottom_0;     /* Bottom row 118 */
      LEP_PIXEL_COORDINATE_T   bottom_1;     /* Bottom row 119 */
      LEP_PIXEL_COORDINATE_T   left_0;       /* Left column 0 */
      LEP_PIXEL_COORDINATE_T   left_1;       /* Left column 1 */
      LEP_PIXEL_COORDINATE_T   right_0;      /* Right column 158 */
      LEP_PIXEL_COORDINATE_T   right_1;      /* Right column 159 */

   } LEP_VID_BORESIGHT_COORDINATES_T, *LEP_VID_BORESIGHT_COORDINATES_T_PTR;

   typedef struct LEP_VID_TARGET_POSITION_T_TAG
   {
      LEP_FLOAT32 row;
      LEP_FLOAT32 col;
      LEP_FLOAT32 rotation;

   }LEP_VID_TARGET_POSITION_T, *LEP_VID_TARGET_POSITION_T_PTR;

   typedef enum LEP_VID_VIDEO_OUTPUT_FORMAT_TAG
   {
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW8 = 0,          // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW10,             // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW12,             // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RGB888,            // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RGB666,            // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RGB565,            // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_YUV422_8BIT,       // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW14,             // SUPPORTED in this release
      LEP_VID_VIDEO_OUTPUT_FORMAT_YUV422_10BIT,      // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_USER_DEFINED,      // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW8_2,            // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW8_3,            // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW8_4,            // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW8_5,            // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW8_6,            // To be supported in later release
      LEP_END_VID_VIDEO_OUTPUT_FORMAT

   }LEP_VID_VIDEO_OUTPUT_FORMAT_E, *LEP_VID_VIDEO_OUTPUT_FORMAT_E_PTR;

/* Hardware Specific I2C
*/
        typedef enum
        {
                REG_READ = 0,
                REG_WRITE,

                END_REG
        } LEP_REG_DIRECTION_E;

        typedef struct
        {
                LEP_UINT8 data[1026];

                LEP_REG_DIRECTION_E readOrWrite;
                LEP_UINT16 bytesToTransfer;

                LEP_UINT8 deviceAddress;
                LEP_UINT8 reserved1;

        } LEP_CMD_PACKET_T;

        typedef struct
        {
                LEP_UINT8 data[1024];

                LEP_INT16  status;
                LEP_UINT16 bytesTransferred;

        } LEP_RESPONSE_PACKET_T;


/* Utility
*/
typedef unsigned short CRC16;


/******************************************************************************/
/** EXPORTED PUBLIC METHODS                                                  **/
/******************************************************************************/
class LeptonSDKEmb32OEM
{
public:

/* Constructor
*/
LeptonSDKEmb32OEM();

/* SDK
*/
    LEP_RESULT LEP_SelectDevice(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_PROTOCOL_DEVICE_E device);

    LEP_RESULT LEP_OpenPort(LEP_UINT16 portID,
                                   LEP_CAMERA_PORT_E portType,
                                   LEP_UINT16   portBaudRate,
                                   LEP_CAMERA_PORT_DESC_T_PTR portDescPtr);

    LEP_RESULT LEP_ClosePort(LEP_CAMERA_PORT_DESC_T_PTR pd);

    LEP_RESULT LEP_ResetPort(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr);

    LEP_RESULT LEP_GetPortStatus(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_UINT16 *status);

    LEP_RESULT LEP_GetDeviceAddress(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_UINT8* deviceAddress);

    LEP_RESULT LEP_GetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_COMMAND_ID commandID,
                                       LEP_ATTRIBUTE_T_PTR attributePtr,
                                       LEP_UINT16 attributeWordLength);

    LEP_RESULT LEP_SetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_COMMAND_ID commandID,
                                       LEP_ATTRIBUTE_T_PTR attributePtr,
                                       LEP_UINT16 attributeWordLength);

    LEP_RESULT LEP_RunCommand(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_COMMAND_ID commandID);

    LEP_RESULT LEP_DirectWriteBuffer(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_ATTRIBUTE_T_PTR attributePtr,
                                            LEP_UINT16 attributeWordLength);

    LEP_RESULT LEP_DirectWriteRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 registerAddress,
                                              LEP_UINT16 regValue);

    LEP_RESULT LEP_DirectReadRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_UINT16 registerAddress,
                                             LEP_UINT16* regValue);

    LEP_RESULT LEP_GetSDKVersion(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_SDK_VERSION_T_PTR sdkVersionPtr);

    LEP_RESULT LEP_GetCameraBootStatus(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_SDK_BOOT_STATUS_E_PTR bootStatusPtr);


/* General AGC Controls
*/

LEP_RESULT LEP_GetAgcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_AGC_ENABLE_E_PTR agcEnableStatePtr );
LEP_RESULT LEP_SetAgcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_AGC_ENABLE_E agcEnableState );

LEP_RESULT LEP_GetAgcPolicy( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_AGC_POLICY_E_PTR agcPolicyPtr );
LEP_RESULT LEP_SetAgcPolicy( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_AGC_POLICY_E agcPolicy );

LEP_RESULT LEP_GetAgcROI( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_AGC_ROI_T_PTR agcROIPtr );
LEP_RESULT LEP_SetAgcROI( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_AGC_ROI_T agcROI );

LEP_RESULT LEP_GetAgcLinearHistogramClipPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_UINT16 *agcLinearHistogramClipPercentPtr );

LEP_RESULT LEP_SetAgcLinearHistogramClipPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_UINT16 agcLinearHistogramClipPercent );

LEP_RESULT LEP_GetAgcHistogramStatistics( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_AGC_HISTOGRAM_STATISTICS_T_PTR *agcHistogramStatisticsPtr );

/* Linear AGC Policy Controls
*/
LEP_RESULT LEP_GetAgcLinearHistogramTailSize( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                     LEP_UINT16 *agcLinearHistogramTailSizePtr );
LEP_RESULT LEP_SetAgcLinearHistogramTailSize( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                     LEP_UINT16 agcLinearHistogramTailSize );

LEP_RESULT LEP_GetAgcHistogramClipPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_UINT16 *agcLinearClipPercentPtr );
LEP_RESULT LEP_SetAgcHistogramClipPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_UINT16 agcLinearClipPercent );

LEP_RESULT LEP_GetAgcLinearMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_UINT16 *agcLinearMaxGainPtr );
LEP_RESULT LEP_SetAgcLinearMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_UINT16 agcLinearMaxGain );

LEP_RESULT LEP_GetAgcLinearMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_UINT16 *agcLinearMidPointPtr );
LEP_RESULT LEP_SetAgcLinearMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_UINT16 agcLinearMidPoint );

LEP_RESULT LEP_GetAgcLinearDampeningFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_UINT16 *agcLinearDampeningFactorPtr );
LEP_RESULT LEP_SetAgcLinearDampeningFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_UINT16 agcLinearDampeningFactor );

/* Heq AGC Policy Controls
*/
LEP_RESULT LEP_GetAgcHeqDampingFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 *agcHeqDampingFactorPtr );
LEP_RESULT LEP_SetAgcHeqDampingFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 agcHeqDampingFactor );

LEP_RESULT LEP_GetAgcHeqMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_UINT16 *agcHeqMaxGainPtr );
LEP_RESULT LEP_SetAgcHeqMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_UINT16 agcHeqMaxGain );

LEP_RESULT LEP_GetAgcHeqClipLimitHigh( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 *agcHeqClipLimitHighPtr );
LEP_RESULT LEP_SetAgcHeqClipLimitHigh( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 agcHeqClipLimitHigh );

LEP_RESULT LEP_GetAgcHeqClipLimitLow( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_UINT16 *agcHeqClipLimitLowPtr );
LEP_RESULT LEP_SetAgcHeqClipLimitLow( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_UINT16 agcHeqClipLimitLow );

LEP_RESULT LEP_GetAgcHeqBinExtension( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_UINT16 *agcHeqBinExtensionPtr );
LEP_RESULT LEP_SetAgcHeqBinExtension( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_UINT16 agcHeqBinExtension );

LEP_RESULT LEP_GetAgcHeqMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_UINT16 *agcHeqMidPointPtr );
LEP_RESULT LEP_SetAgcHeqMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_UINT16 agcHeqMidPoint );

LEP_RESULT LEP_GetAgcHeqEmptyCount( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_AGC_HEQ_EMPTY_COUNT_T_PTR emptyCountPtr );
LEP_RESULT LEP_SetAgcHeqEmptyCount( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_AGC_HEQ_EMPTY_COUNT_T emptyCount );

LEP_RESULT LEP_GetAgcHeqNormalizationFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_AGC_HEQ_NORMALIZATION_FACTOR_T_PTR normalizationFactorPtr );
LEP_RESULT LEP_SetAgcHeqNormalizationFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_AGC_HEQ_NORMALIZATION_FACTOR_T normalizationFactor );

LEP_RESULT LEP_GetAgcHeqScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_AGC_HEQ_SCALE_FACTOR_E_PTR scaleFactorPtr );
LEP_RESULT LEP_SetAgcHeqScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_AGC_HEQ_SCALE_FACTOR_E scaleFactor );

LEP_RESULT LEP_GetAgcCalcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_AGC_ENABLE_E_PTR agcCalculationEnableStatePtr );

LEP_RESULT LEP_SetAgcCalcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_AGC_ENABLE_E agcCalculationEnableState );

LEP_RESULT LEP_GetAgcHeqLinearPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 *agcHeqLinearPercentPtr);

LEP_RESULT LEP_SetAgcHeqLinearPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 agcHeqLinearPercent);

/* I2C_Protocol
*/
    LEP_RESULT LEP_I2C_GetCommandBusyStatus(LEP_I2C_COMMAND_STATUS_E_PTR commandStatus);

    LEP_RESULT LEP_I2C_SetCommandRegister(LEP_COMMAND_ID commandID,
                                          LEP_UINT16 *transactionStatus);

    LEP_RESULT LEP_I2C_SelectDevice(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_PROTOCOL_DEVICE_E device);

    LEP_RESULT LEP_I2C_OpenPort(LEP_UINT16 portID,
                                       LEP_UINT16 *baudRateInkHz,
                                       LEP_UINT8 *deviceAddress);

    LEP_RESULT LEP_I2C_ClosePort(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr);

    LEP_RESULT LEP_I2C_ResetPort(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr);

    LEP_RESULT LEP_I2C_GetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_COMMAND_ID commandID,
                                           LEP_ATTRIBUTE_T_PTR attributePtr,
                                           LEP_UINT16 attributeWordLength);

    LEP_RESULT LEP_I2C_SetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_COMMAND_ID commandID,
                                           LEP_ATTRIBUTE_T_PTR attributePtr,
                                           LEP_UINT16 attributeWordLength);

    LEP_RESULT LEP_I2C_RunCommand(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_COMMAND_ID commandID);

    LEP_RESULT LEP_I2C_ReadData(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr);

    LEP_RESULT LEP_I2C_WriteData(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr);

    LEP_RESULT LEP_I2C_GetPortStatus(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr);

    LEP_RESULT LEP_I2C_GetDeviceAddress(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_UINT8* deviceAddress);

    LEP_RESULT LEP_I2C_DirectWriteRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_UINT16 regAddress,
                                                  LEP_UINT16 regValue);
    LEP_RESULT LEP_I2C_DirectWriteBuffer(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_ATTRIBUTE_T_PTR attributePtr,
                                                LEP_UINT16 attributeWordLength);
    LEP_RESULT LEP_I2C_DirectReadRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_UINT16 regAddress,
                                                 LEP_UINT16 *regValue);

/* I2C_Service
*/

    LEP_RESULT LEP_I2C_MasterSelectDevice(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_PROTOCOL_DEVICE_E device);

    LEP_RESULT LEP_I2C_MasterOpen(LEP_UINT16 portID,
                                         LEP_UINT16 *portBaudRate);

    LEP_RESULT LEP_I2C_MasterClose(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );

    LEP_RESULT LEP_I2C_MasterReset(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );

    LEP_RESULT LEP_I2C_MasterReadData(LEP_UINT16 portID,
                                             LEP_UINT8  deviceAddress,
                                             LEP_UINT16 subAddress,
                                             LEP_UINT16 *dataPtr,
                                             LEP_UINT16 dataLength);

    LEP_RESULT LEP_I2C_MasterWriteData(LEP_UINT16 portID,
                                              LEP_UINT8  deviceAddress,
                                              LEP_UINT16 subAddress,
                                              LEP_UINT16 *dataPtr,
                                              LEP_UINT16 dataLength);

    LEP_RESULT LEP_I2C_MasterReadRegister(LEP_UINT16 portID,
                                                 LEP_UINT8  deviceAddress,
                                                 LEP_UINT16 regAddress,
                                                 LEP_UINT16 *regValue);


    LEP_RESULT LEP_I2C_MasterWriteRegister(LEP_UINT16 portID,
                                                  LEP_UINT8  deviceAddress,
                                                  LEP_UINT16 regAddress,
                                                  LEP_UINT16 regValue);

    LEP_RESULT LEP_I2C_MasterStatus(LEP_UINT16 portID,
                                           LEP_UINT16 *portStatus );

/* OEM
*/
LEP_RESULT LEP_RunOemPowerDown( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
LEP_RESULT LEP_RunOemPowerOn( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
LEP_RESULT LEP_RunOemStandby( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
LEP_RESULT LEP_RunOemReboot( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
LEP_RESULT LEP_RunOemLowPowerMode1( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
LEP_RESULT LEP_RunOemLowPowerMode2( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
LEP_RESULT LEP_RunOemBit( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );

LEP_RESULT LEP_GetOemMaskRevision( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_OEM_MASK_REVISION_T *oemMaskRevisionPtr );
   #if 0
LEP_RESULT LEP_GetOemMasterID( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_OEM_MASTER_ID_T_PTR oemMasterIDPtr );
   #endif

   #if USE_DEPRECATED_PART_NUMBER_INTERFACE
/* Deprecated: use LEP_GetOemFlirPN instead */
LEP_RESULT LEP_GetOemFlirPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr );
/* Deprecated: use LEP_GetOemCustPN instead */
LEP_RESULT LEP_GetOemCustPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr );
   #else
LEP_RESULT LEP_GetOemFlirPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr );
LEP_RESULT LEP_GetOemCustPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr );
   #endif
LEP_RESULT LEP_GetOemSoftwareVersion( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_OEM_SW_VERSION_T *oemSoftwareVersionPtr );

   #if 0
LEP_RESULT LEP_GetOemVendorID(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_OEM_VENDORID_T *oemVendorIDPtr);
   #endif

LEP_RESULT LEP_GetOemVideoOutputEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_VIDEO_OUTPUT_ENABLE_E_PTR oemVideoOutputEnablePtr );
LEP_RESULT LEP_SetOemVideoOutputEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_VIDEO_OUTPUT_ENABLE_E oemVideoOutputEnable );

LEP_RESULT LEP_GetOemVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_VIDEO_OUTPUT_FORMAT_E_PTR oemVideoOutputFormatPtr );
LEP_RESULT LEP_SetOemVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_VIDEO_OUTPUT_FORMAT_E oemVideoOutputFormat );
LEP_RESULT LEP_GetOemVideoOutputSource( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_VIDEO_OUTPUT_SOURCE_E_PTR oemVideoOutputSourcePtr );
LEP_RESULT LEP_SetOemVideoOutputSource( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_VIDEO_OUTPUT_SOURCE_E oemVideoOutputSource );
LEP_RESULT LEP_GetOemVideoOutputSourceConstant( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                       LEP_UINT16 *oemVideoOutputSourceConstant );
LEP_RESULT LEP_SetOemVideoOutputSourceConstant( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                       LEP_UINT16 oemVideoOutputSourceConstant );


LEP_RESULT LEP_GetOemVideoOutputChannel( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_OEM_VIDEO_OUTPUT_CHANNEL_E_PTR oemVideoOutputChannelPtr );
LEP_RESULT LEP_SetOemVideoOutputChannel( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_OEM_VIDEO_OUTPUT_CHANNEL_E oemVideoOutputChannel );

LEP_RESULT LEP_GetOemVideoGammaEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_OEM_VIDEO_GAMMA_ENABLE_E_PTR oemVideoGammaEnablePtr );
LEP_RESULT LEP_SetOemVideoGammaEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_OEM_VIDEO_GAMMA_ENABLE_E oemVideoGammaEnable );

LEP_RESULT LEP_GetOemCalStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_OEM_STATUS_E_PTR calStatusPtr );

LEP_RESULT LEP_GetOemFFCNormalizationTarget( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_OEM_FFC_NORMALIZATION_TARGET_T_PTR ffcTargetPtr );
LEP_RESULT LEP_SetOemFFCNormalizationTarget( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_OEM_FFC_NORMALIZATION_TARGET_T ffcTarget );
LEP_RESULT LEP_RunOemFFCNormalization( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_OEM_FFC_NORMALIZATION_TARGET_T ffcTarget );

LEP_RESULT LEP_GetOemFrameMean( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_OEM_FRAME_AVERAGE_T_PTR frameAveragePtr );

LEP_RESULT LEP_GetOemPowerMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_OEM_POWER_STATE_E_PTR powerModePtr );
LEP_RESULT LEP_SetOemPowerMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_OEM_POWER_STATE_E powerMode );

LEP_RESULT LEP_RunOemFFC( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );


LEP_RESULT LEP_GetOemGpioMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_OEM_GPIO_MODE_E_PTR gpioModePtr );
LEP_RESULT LEP_SetOemGpioMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_OEM_GPIO_MODE_E gpioMode );
LEP_RESULT LEP_GetOemGpioVsyncPhaseDelay( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_OEM_VSYNC_DELAY_E_PTR numHsyncLinesPtr );
LEP_RESULT LEP_SetOemGpioVsyncPhaseDelay( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_OEM_VSYNC_DELAY_E numHsyncLines );


LEP_RESULT LEP_GetOemUserDefaultsState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_USER_PARAMS_STATE_E_PTR userParamsStatePtr );
LEP_RESULT LEP_RunOemUserDefaultsCopyToOtp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
LEP_RESULT LEP_RunOemUserDefaultsRestore( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );



LEP_RESULT LEP_GetOemThermalShutdownEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_OEM_THERMAL_SHUTDOWN_ENABLE_T_PTR ThermalShutdownEnableStatePtr );

LEP_RESULT LEP_SetOemThermalShutdownEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_OEM_THERMAL_SHUTDOWN_ENABLE_T ThermalShutdownEnableState );

LEP_RESULT LEP_GetOemShutterProfileObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_SHUTTER_PROFILE_OBJ_T_PTR ShutterProfileObjPtr );

LEP_RESULT LEP_SetOemShutterProfileObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_SHUTTER_PROFILE_OBJ_T ShutterProfileObj );

LEP_RESULT LEP_GetOemBadPixelReplaceControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_OEM_BAD_PIXEL_REPLACE_CONTROL_T_PTR BadPixelReplaceControlPtr );
LEP_RESULT LEP_SetOemBadPixelReplaceControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_OEM_BAD_PIXEL_REPLACE_CONTROL_T BadPixelReplaceControl );


LEP_RESULT LEP_GetOemTemporalFilterControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_OEM_TEMPORAL_FILTER_CONTROL_T_PTR TemporalFilterControlPtr );
LEP_RESULT LEP_SetOemTemporalFilterControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_OEM_TEMPORAL_FILTER_CONTROL_T TemporalFilterControl );


LEP_RESULT LEP_GetOemColumnNoiseEstimateControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                        LEP_OEM_COLUMN_NOISE_ESTIMATE_CONTROL_T_PTR ColumnNoiseEstimateControlPtr );
LEP_RESULT LEP_SetOemColumnNoiseEstimateControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                        LEP_OEM_COLUMN_NOISE_ESTIMATE_CONTROL_T ColumnNoiseEstimateControl );


LEP_RESULT LEP_GetOemPixelNoiseSettings( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_OEM_PIXEL_NOISE_SETTINGS_T_PTR PixelNoiseEstimateControlPtr );
LEP_RESULT LEP_SetOemPixelNoiseSettings( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_OEM_PIXEL_NOISE_SETTINGS_T PixelNoiseEstimateControl );


/* RAD
*/
LEP_RESULT LEP_GetRadTShutterMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_TS_MODE_E_PTR radTShutterModePtr );

LEP_RESULT LEP_SetRadTShutterMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_TS_MODE_E radTShutterMode );

LEP_RESULT LEP_GetRadTShutter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_KELVIN_T_PTR radTShutterPtr );

LEP_RESULT LEP_SetRadTShutter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_KELVIN_T radTShutter );

LEP_RESULT LEP_RunRadFFC( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );

LEP_RESULT LEP_GetRadRBFOInternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr );

LEP_RESULT LEP_SetRadRBFOInternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr );

LEP_RESULT LEP_GetRadRBFOExternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr );

LEP_RESULT LEP_SetRadRBFOExternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr );

LEP_RESULT LEP_GetRadResponsivityShift( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_RAD_RS_T_PTR radResponsivityShiftPtr );

LEP_RESULT LEP_SetRadResponsivityShift( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_RAD_RS_T radResponsivityShift );

LEP_RESULT LEP_GetRadFNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_FNUMBER_T_PTR radFNumberPtr );

LEP_RESULT LEP_SetRadFNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_FNUMBER_T radFNumber );

LEP_RESULT LEP_GetRadTauLens( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_TAULENS_T_PTR radTauLensPtr );

LEP_RESULT LEP_SetRadTauLens( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_TAULENS_T radTauLens );

LEP_RESULT LEP_GetRadRadometryFilter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_RAD_RADIOMETRY_FILTER_T_PTR radRadiometryFilterPtr );

LEP_RESULT LEP_SetRadRadometryFilter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_RAD_RADIOMETRY_FILTER_T radRadiometryFilter );

/* Deprecated: Use LEP_GetRadTFpaLut */
LEP_RESULT LEP_GetRadTFpaCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_LUT256_T_PTR radTFpaCLutPtr );
/* Deprecated: Use LEP_SetRadTFpaLut */
LEP_RESULT LEP_SetRadTFpaCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_LUT256_T_PTR radTFpaCLutPtr );
/* Deprecated: Use LEP_GetRadTAuxLut */
LEP_RESULT LEP_GetRadTAuxCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_LUT256_T_PTR radTAuxCLutPtr );
/* Deprecated: Use LEP_SetRadTAuxLut */
LEP_RESULT LEP_SetRadTAuxCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_LUT256_T_PTR radTAuxCLutPtr );

LEP_RESULT LEP_GetRadTFpaLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_LUT256_T_PTR radTFpaLutPtr );

LEP_RESULT LEP_SetRadTFpaLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_LUT256_T_PTR radTFpaLutPtr );

LEP_RESULT LEP_GetRadTAuxLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_LUT256_T_PTR radTAuxLutPtr );

LEP_RESULT LEP_SetRadTAuxLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_LUT256_T_PTR radTAuxLutPtr );

LEP_RESULT LEP_GetRadResponsivityValueLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_RAD_LUT128_T_PTR radResponsivityValueLutPtr );

LEP_RESULT LEP_SetRadResponsivityValueLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_RAD_LUT128_T_PTR radResponsivityValueLutPtr );

LEP_RESULT LEP_GetRadDebugTemp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_RAD_KELVIN_T_PTR radDebugTempPtr );

LEP_RESULT LEP_SetRadDebugTemp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_RAD_KELVIN_T radDebugTemp );

LEP_RESULT LEP_GetRadDebugFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_RAD_FLUX_T_PTR radDebugFluxPtr );

LEP_RESULT LEP_SetRadDebugFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_RAD_FLUX_T radDebugFlux );

LEP_RESULT LEP_GetRadEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_ENABLE_E_PTR radEnableStatePtr );

LEP_RESULT LEP_SetRadEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_ENABLE_E radEnableState );

LEP_RESULT LEP_GetRadGlobalGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_GLOBAL_GAIN_T_PTR radGlobalGainPtr );

LEP_RESULT LEP_SetRadGlobalGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_GLOBAL_GAIN_T radGlobalGain );

LEP_RESULT LEP_GetRadGlobalOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_GLOBAL_OFFSET_T_PTR radGlobalOffsetPtr );

LEP_RESULT LEP_SetRadGlobalOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_GLOBAL_OFFSET_T radGlobalOffset );

LEP_RESULT LEP_GetRadTFpaCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_TEMPERATURE_UPDATE_E_PTR radTFpaCtsModePtr );

LEP_RESULT LEP_SetRadTFpaCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_TEMPERATURE_UPDATE_E radTFpaCtsMode );

LEP_RESULT LEP_GetRadTAuxCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_TEMPERATURE_UPDATE_E_PTR radTAuxCtsModePtr );

LEP_RESULT LEP_SetRadTAuxCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_TEMPERATURE_UPDATE_E radTAuxCtsMode );

LEP_RESULT LEP_GetRadTFpaCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_TEMPERATURE_COUNTS_T_PTR radTFpaCtsPtr );

LEP_RESULT LEP_SetRadTFpaCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_TEMPERATURE_COUNTS_T radTFpaCts );

LEP_RESULT LEP_GetRadTAuxCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_TEMPERATURE_COUNTS_T_PTR radTAuxCtsPtr );

LEP_RESULT LEP_SetRadTAuxCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_TEMPERATURE_COUNTS_T radTAuxCts );

LEP_RESULT LEP_GetRadTEqShutterLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RAD_LUT128_T_PTR radTEqShutterLutPtr );

LEP_RESULT LEP_SetRadTEqShutterLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RAD_LUT128_T_PTR radTEqShutterLutPtr );

LEP_RESULT LEP_GetRadRunStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_RAD_STATUS_E_PTR radStatusPtr );

LEP_RESULT LEP_GetRadTEqShutterFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_FLUX_T_PTR radTEqShutterFluxPtr );

LEP_RESULT LEP_SetRadTEqShutterFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_FLUX_T radTEqShutterFlux );

LEP_RESULT LEP_GetRadMffcFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_FLUX_T_PTR radRadMffcFluxPtr  );

LEP_RESULT LEP_SetRadMffcFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_FLUX_T radRadMffcFlux );

LEP_RESULT LEP_GetRadFrameMedianPixelValue( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_RAD_MEDIAN_VALUE_T_PTR frameMedianPtr );

LEP_RESULT LEP_GetRadMLGLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RAD_SIGNED_LUT128_T_PTR radMLGLutPtr );

LEP_RESULT LEP_SetRadMLGLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RAD_SIGNED_LUT128_T_PTR radMLGLutPtr );

   #if USE_DEPRECATED_HOUSING_TCP_INTERFACE
LEP_RESULT LEP_GetRadTHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR radHousingTcp );
LEP_RESULT LEP_SetRadTHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_LINEAR_TEMP_CORRECTION_T radHousingTcp );
   #else
LEP_RESULT LEP_GetRadHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR radHousingTcp );
LEP_RESULT LEP_SetRadHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_LINEAR_TEMP_CORRECTION_T radHousingTcp );
   #endif


LEP_RESULT LEP_GetRadShutterTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR radShutterTcp );

LEP_RESULT LEP_SetRadShutterTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_LINEAR_TEMP_CORRECTION_T radShutterTcp );

LEP_RESULT LEP_GetRadLensTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR radLensTcp );

LEP_RESULT LEP_SetRadLensTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_LINEAR_TEMP_CORRECTION_T radLensTcp );

LEP_RESULT LEP_GetRadPreviousGlobalOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_RAD_GLOBAL_OFFSET_T_PTR globalOffsetPtr );

LEP_RESULT LEP_GetRadPreviousGlobalGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_RAD_GLOBAL_GAIN_T_PTR globalGainPtr );

LEP_RESULT LEP_GetGlobalGainFFC( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_GLOBAL_GAIN_T_PTR globalGainFfcPtr );

LEP_RESULT LEP_GetRadCnfScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_PARAMETER_SCALE_FACTOR_T_PTR scaleFactorPtr );
LEP_RESULT LEP_GetRadTnfScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_PARAMETER_SCALE_FACTOR_T_PTR scaleFactorPtr );
LEP_RESULT LEP_GetRadSnfScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_PARAMETER_SCALE_FACTOR_T_PTR scaleFactorPtr );

LEP_RESULT LEP_GetRadArbitraryOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_RAD_ARBITRARY_OFFSET_T_PTR arbitraryOffsetPtr );
LEP_RESULT LEP_SetRadArbitraryOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_RAD_ARBITRARY_OFFSET_T arbitraryOffset );

LEP_RESULT LEP_GetRadFluxLinearParams( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_RAD_FLUX_LINEAR_PARAMS_T_PTR fluxParamsPtr );

LEP_RESULT LEP_SetRadFluxLinearParams( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_RAD_FLUX_LINEAR_PARAMS_T fluxParams );

LEP_RESULT LEP_GetRadTLinearEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_RAD_ENABLE_E_PTR enableStatePtr );

LEP_RESULT LEP_SetRadTLinearEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_RAD_ENABLE_E enableState );

LEP_RESULT LEP_GetRadTLinearResolution( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_RAD_TLINEAR_RESOLUTION_E_PTR resolutionPtr );

LEP_RESULT LEP_SetRadTLinearResolution( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_RAD_TLINEAR_RESOLUTION_E resolution );

LEP_RESULT LEP_GetRadTLinearAutoResolution( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_RAD_ENABLE_E_PTR enableStatePtr );

LEP_RESULT LEP_SetRadTLinearAutoResolution( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_RAD_ENABLE_E enableState );

LEP_RESULT LEP_GetRadSpotmeterRoi( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_ROI_T_PTR spotmeterRoiPtr );

LEP_RESULT LEP_SetRadSpotmeterRoi( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_ROI_T spotmeterRoi );

LEP_RESULT LEP_GetRadSpotmeterObjInKelvinX100( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                      LEP_RAD_SPOTMETER_OBJ_KELVIN_T_PTR kelvinPtr );

LEP_RESULT LEP_GetRadArbitraryOffsetMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_RAD_ARBITRARY_OFFSET_MODE_E_PTR arbitraryOffsetModePtr );

LEP_RESULT LEP_SetRadArbitraryOffsetMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_RAD_ARBITRARY_OFFSET_MODE_E arbitraryOffsetMode );

LEP_RESULT LEP_GetRadArbitraryOffsetParams( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_RAD_ARBITRARY_OFFSET_PARAMS_T_PTR arbitraryOffsetParamsPtr);

LEP_RESULT LEP_SetRadArbitraryOffsetParams( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_RAD_ARBITRARY_OFFSET_PARAMS_T arbitraryOffsetParams);

LEP_RESULT LEP_GetRadInternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_RBFO_T_PTR radRBFOPtr );

LEP_RESULT LEP_SetRadInternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_RBFO_T_PTR radRBFOPtr );

LEP_RESULT LEP_GetRadExternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_RBFO_T_PTR radRBFOPtr );

LEP_RESULT LEP_SetRadExternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_RBFO_T_PTR radRBFOPtr );

LEP_RESULT LEP_GetRadInternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_RBFO_T_PTR radRBFOPtr );

LEP_RESULT LEP_SetRadInternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_RBFO_T_PTR radRBFOPtr );

LEP_RESULT LEP_GetRadExternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_RBFO_T_PTR radRBFOPtr );

LEP_RESULT LEP_SetRadExternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_RBFO_T_PTR radRBFOPtr );

LEP_RESULT LEP_GetRadRadioCalValues( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_RADIO_CAL_VALUES_T_PTR radRadioCalValuesPtr);

LEP_RESULT LEP_SetRadRadioCalValues( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_RADIO_CAL_VALUES_T radRadioCalValues );

/* SYS
*/
LEP_RESULT LEP_RunSysPing( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
LEP_RESULT LEP_GetSysStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_STATUS_T_PTR sysStatusPtr );
LEP_RESULT LEP_GetSysFlirSerialNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_SYS_FLIR_SERIAL_NUMBER_T_PTR sysSerialNumberBufPtr );

/* Deprecated: Use LEP_GetSysCustSN instead */
   #if LEP_SYS_CUST_SERIAL_NUMBER_T
LEP_RESULT LEP_GetSysCustSerialNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_SYS_CUST_SERIAL_NUMBER_T_PTR sysSerialNumberPtr );
   #else
LEP_RESULT LEP_GetSysCustSerialNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_SYS_CUST_SERIAL_NUMBER_T_PTR sysCustSNPtr );
   #endif
LEP_RESULT LEP_GetSysCameraUpTime( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_SYS_UPTIME_NUMBER_T_PTR sysCameraUpTimePtr );

LEP_RESULT LEP_GetSysAuxTemperatureCelcius( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_SYS_AUX_TEMPERATURE_CELCIUS_T_PTR auxTemperaturePtr );

LEP_RESULT LEP_GetSysFpaTemperatureCelcius( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_SYS_FPA_TEMPERATURE_CELCIUS_T_PTR fpaTemperaturePtr );

LEP_RESULT LEP_GetSysAuxTemperatureKelvin( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_SYS_AUX_TEMPERATURE_KELVIN_T_PTR auxTemperaturePtr );

LEP_RESULT LEP_GetSysFpaTemperatureKelvin( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_SYS_FPA_TEMPERATURE_KELVIN_T_PTR fpaTemperaturePtr );

LEP_RESULT LEP_GetSysTelemetryEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_SYS_TELEMETRY_ENABLE_STATE_E_PTR enableStatePtr );
LEP_RESULT LEP_SetSysTelemetryEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_SYS_TELEMETRY_ENABLE_STATE_E enableState );

LEP_RESULT LEP_GetSysTelemetryLocation( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_SYS_TELEMETRY_LOCATION_E_PTR telemetryLocationPtr );
LEP_RESULT LEP_SetSysTelemetryLocation( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_SYS_TELEMETRY_LOCATION_E telemetryLocation );


LEP_RESULT LEP_RunSysAverageFrames( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_SYS_FRAME_AVERAGE_DIVISOR_E numFrameToAverage );
LEP_RESULT LEP_GetSysFramesToAverage( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_SYS_FRAME_AVERAGE_DIVISOR_E_PTR numFrameToAveragePtr );
LEP_RESULT LEP_SetSysFramesToAverage( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_SYS_FRAME_AVERAGE_DIVISOR_E numFrameToAverage );

LEP_RESULT LEP_GetSysSceneStatistics( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_SYS_SCENE_STATISTICS_T_PTR sceneStatisticsPtr );

LEP_RESULT LEP_RunFrameAverage( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );

LEP_RESULT LEP_GetSysSceneRoi( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_VIDEO_ROI_T_PTR sceneRoiPtr );
LEP_RESULT LEP_SetSysSceneRoi( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_VIDEO_ROI_T sceneRoi );

LEP_RESULT LEP_GetSysThermalShutdownCount( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_SYS_THERMAL_SHUTDOWN_COUNTS_T_PTR thermalCountsPtr );

LEP_RESULT LEP_GetSysShutterPosition( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_SYS_SHUTTER_POSITION_E_PTR shutterPositionPtr );

LEP_RESULT LEP_SetSysShutterPosition( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_SYS_SHUTTER_POSITION_E shutterPosition );

LEP_RESULT LEP_GetSysFfcShutterModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_SYS_FFC_SHUTTER_MODE_OBJ_T_PTR shutterModeObjPtr );

LEP_RESULT LEP_SetSysFfcShutterModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_SYS_FFC_SHUTTER_MODE_OBJ_T shutterModeObj );

LEP_RESULT LEP_GetSysFFCStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_SYS_STATUS_E_PTR ffcStatusPtr );

LEP_RESULT LEP_RunSysFFCNormalization( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );

LEP_RESULT LEP_GetSysGainMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_GAIN_MODE_E_PTR gainModePtr );
LEP_RESULT LEP_SetSysGainMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_GAIN_MODE_E gainMode );

LEP_RESULT LEP_GetSysFFCStates( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_SYS_FFC_STATES_E_PTR ffcStatePtr );

LEP_RESULT LEP_GetSysGainModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_SYS_GAIN_MODE_OBJ_T_PTR gainModeObjPtr );
LEP_RESULT LEP_SetSysGainModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_SYS_GAIN_MODE_OBJ_T gainModeObj );

LEP_RESULT LEP_GetSysBoresightValues( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_SYS_BORESIGHT_VALUES_T_PTR boresightValuesPtr);


/* VID
*/
   LEP_RESULT LEP_GetVidPolarity(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_POLARITY_E_PTR vidPolarityPtr);
   LEP_RESULT LEP_SetVidPolarity(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_POLARITY_E vidPolarity);

   LEP_RESULT LEP_GetVidPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_PCOLOR_LUT_E_PTR vidPcolorLutPtr);
   LEP_RESULT LEP_SetVidPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_PCOLOR_LUT_E vidPcolorLut);

   LEP_RESULT LEP_GetVidLowGainPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_PCOLOR_LUT_E_PTR vidPcolorLutPtr);
   LEP_RESULT LEP_SetVidLowGainPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_PCOLOR_LUT_E vidPcolorLut);

   LEP_RESULT LEP_GetVidUserLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_VID_LUT_BUFFER_T_PTR vidUserLutBufPtr);

   LEP_RESULT LEP_SetVidUserLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_VID_LUT_BUFFER_T_PTR vidUserLutBufPtr);

   LEP_RESULT LEP_GetVidFocusCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_VID_FOCUS_CALC_ENABLE_E_PTR vidEnableFocusCalcStatePtr);
   LEP_RESULT LEP_SetVidFocusCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_VID_FOCUS_CALC_ENABLE_E vidFocusCalcEnableState);

   LEP_RESULT LEP_GetVidBoresightCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                        LEP_VID_BORESIGHT_CALC_ENABLE_STATE_E_PTR boresightCalcEnableStatePtr);
   LEP_RESULT LEP_SetVidBoresightCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                        LEP_VID_BORESIGHT_CALC_ENABLE_STATE_E boresightCalcEnableState);
   LEP_RESULT LEP_GetVidBoresightCoordinates(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_VID_BORESIGHT_COORDINATES_T_PTR boresightCoordinatesPtr);
   LEP_RESULT LEP_GetVidTargetPosition(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_VID_TARGET_POSITION_T_PTR targetPositionPtr);

   LEP_RESULT LEP_GetVidROI(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_VID_FOCUS_ROI_T_PTR vidFocusROIPtr);
   LEP_RESULT LEP_SetVidROI(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_VID_FOCUS_ROI_T vidFocusROI);

   LEP_RESULT LEP_GetVidFocusMetric(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_VID_FOCUS_METRIC_T_PTR vidFocusMetricPtr);

   LEP_RESULT LEP_GetVidFocusMetricThreshold(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_VID_FOCUS_METRIC_THRESHOLD_T_PTR vidFocusMetricThresholdPtr);
   LEP_RESULT LEP_SetVidFocusMetricThreshold(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_VID_FOCUS_METRIC_THRESHOLD_T vidFocusMetricThreshold);

   LEP_RESULT LEP_GetVidSbNucEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_VID_SBNUC_ENABLE_E_PTR vidSbNucEnableStatePtr);
   LEP_RESULT LEP_SetVidSbNucEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_VID_SBNUC_ENABLE_E vidSbNucEnableState);

   LEP_RESULT LEP_GetVidFreezeEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_VID_FREEZE_ENABLE_E_PTR vidFreezeEnableStatePtr);

   LEP_RESULT LEP_SetVidFreezeEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_VID_FREEZE_ENABLE_E vidFreezeEnableState);

   LEP_RESULT LEP_GetVidVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_VID_VIDEO_OUTPUT_FORMAT_E_PTR vidVideoOutputFormatPtr );

   LEP_RESULT LEP_SetVidVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_VID_VIDEO_OUTPUT_FORMAT_E vidVideoOutputFormat );

/*
   LEP_RESULT LEP_GetOemVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_VID_VIDEO_OUTPUT_FORMAT_E_PTR vidVideoOutputFormatPtr );
   LEP_RESULT LEP_SetOemVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_VID_VIDEO_OUTPUT_FORMAT_E vidVideoOutputFormat );
*/

#if (USE_BORESIGHT_MEASUREMENT_FUNCTIONS == 1)
   LEP_RESULT LEP_CalcVidBoresightAlignment(LEP_VID_BORESIGHT_COORDINATES_T boresightCoordinates,
                                                   LEP_VID_TARGET_POSITION_T_PTR targetPositionPtr);
#endif

/* Hardware Specific I2C
*/
    LEP_RESULT DEV_I2C_MasterSelectDevice(LEP_PROTOCOL_DEVICE_E device);

    LEP_RESULT DEV_I2C_MasterInit(LEP_UINT16 portID,
                                         LEP_UINT16 *BaudRate);

    LEP_RESULT DEV_I2C_MasterClose();

    LEP_RESULT DEV_I2C_MasterReset(void );

    LEP_RESULT DEV_I2C_MasterReadData(LEP_UINT16 portID,
                                             LEP_UINT8   deviceAddress,
                                             LEP_UINT16  regAddress,            // Lepton Register Address
                                             LEP_UINT16 *readDataPtr,
                                             LEP_UINT16  wordsToRead,          // Number of 16-bit words to Read
                                             LEP_UINT16 *numWordsRead,         // Number of 16-bit words actually Read
                                             LEP_UINT16 *status
                                            );

    LEP_RESULT DEV_I2C_MasterWriteData(LEP_UINT16 portID,
                                              LEP_UINT8   deviceAddress,
                                              LEP_UINT16  regAddress,            // Lepton Register Address
                                              LEP_UINT16 *writeDataPtr,
                                              LEP_UINT16  wordsToWrite,        // Number of 16-bit words to Write
                                              LEP_UINT16 *numWordsWritten,     // Number of 16-bit words actually written
                                              LEP_UINT16 *status
                                             );

    LEP_RESULT DEV_I2C_MasterReadRegister( LEP_UINT16 portID,
                                                  LEP_UINT8  deviceAddress,
                                                  LEP_UINT16 regAddress,
                                                  LEP_UINT16 *regValue,     // Number of 16-bit words actually written
                                                  LEP_UINT16 *status
                                                );

    LEP_RESULT DEV_I2C_MasterWriteRegister( LEP_UINT16 portID,
                                                   LEP_UINT8  deviceAddress,
                                                   LEP_UINT16 regAddress,
                                                   LEP_UINT16 regValue,     // Number of 16-bit words actually written
                                                   LEP_UINT16 *status
                                                 );

    LEP_RESULT DEV_I2C_MasterStatus(void );



private:

const CRC16 ccitt_16Table[256] = {
   0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
   0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
   0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
   0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
   0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
   0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
   0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
   0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
   0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
   0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
   0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
   0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
   0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
   0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
   0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
   0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
   0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
   0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
   0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
   0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
   0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
   0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
   0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
   0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
   0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
   0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
   0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
   0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
   0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
   0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
   0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
   0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

static LEP_RESULT _LEP_DelayCounts(LEP_UINT32 counts);

int ByteCRC16(int value, int crcin);
CRC16 CalcCRC16Words(unsigned int count, short *buffer);
CRC16 CalcCRC16Bytes(unsigned int count, char *buffer);

// Internal buffer to convert enum size to Lep data size
LEP_UINT16 _LEP_data[16];

};

#endif /* _LEPTON_SDK_H_ */
