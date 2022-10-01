/*
 * Lepton related utilities
 *
 * Contains utility and access functions for the Lepton.
 *
 * Copyright 2020 Dan Julio
 *
 * This file is part of tCam.
 *
 * tCam is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tCam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tCam.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef LEPTON_UTILITIES_H
#define LEPTON_UTILITIES_H

#include <stdbool.h>
#include <stdint.h>

//
// Lepton Utilities Constants
//

//
// Telemetry words
//
// From Row A (words 0-79)
#define LEP_TEL_REV          0
#define LEP_TEL_TC_LOW       1
#define LEP_TEL_TC_HIGH      2
#define LEP_TEL_STATUS_LOW   3
#define LEP_TEL_STATUS_HIGH  4
#define LEP_TEL_SN_0         5
#define LEP_TEL_SN_1         6
#define LEP_TEL_SN_2         7
#define LEP_TEL_SN_3         8
#define LEP_TEL_SN_4         9
#define LEP_TEL_SN_5         10
#define LEP_TEL_SN_6         11
#define LEP_TEL_SN_7         12
#define LEP_TEL_REV_0        13
#define LEP_TEL_REV_1        14
#define LEP_TEL_REV_2        15
#define LEP_TEL_REV_3        16
#define LEP_TEL_FC_LOW       20
#define LEP_TEL_FC_HIGH      21
#define LEP_TEL_FRAME_MEAN   22
#define LEP_TEL_FPA_T_CNT    23
#define LEP_TEL_FPA_T_K100   24
#define LEP_TEL_HSE_T_CNT    25
#define LEP_TEL_HSE_T_K100   26
#define LEP_TEL_LAST_FPA_T   29
#define LEP_TEL_LAST_TC_LOW  30
#define LEP_TEL_LAST_TC_HIGH 31
#define LEP_TEL_LAST_HST_T   32
#define LEP_TEL_AGC_ROI_0    34
#define LEP_TEL_AGC_ROI_1    35
#define LEP_TEL_AGC_ROI_2    36
#define LEP_TEL_AGC_ROI_3    37
#define LEP_TEL_AGC_CL_HIGH  38
#define LEP_TEL_AGC_CL_LOW   39
#define LEP_TEL_VID_FMT_LOW  72
#define LEP_TEL_VID_FMT_HIGH 73
#define LEP_TEL_FFC_FR_LOG2  74

// From Row B (words 80-159)
#define LEP_TEL_EMISSIVITY       99
#define LEP_TEL_BG_T_K100       100
#define LEP_TEL_ATM_TRANS       101
#define LEP_TEL_ATM_T_K100      102
#define LEP_TEL_WND_TRANS       103
#define LEP_TEL_WND_REFL        104
#define LEP_TEL_WND_T_K100      105
#define LEP_TEL_WND_REFL_T_K100 106

// From Row C (words 160-239)
#define LEP_TEL_GAIN_MODE       165
#define LEP_TEL_EFF_GAIN_MODE   166
#define LEP_TEL_GAIN_MODE_DES   167
#define LEP_TEL_GAIN_TH_H2LC    168
#define LEP_TEL_GAIN_TH_L2HC    169
#define LEP_TEL_GAIN_TH_H2LK    170
#define LEP_TEL_GAIN_TH_L2HK    171
#define LEP_TEL_GAIN_POP_H2L    174
#define LEP_TEL_GAIN_POP_L2H    175
#define LEP_TEL_GAIN_MODE_ROI_0 182
#define LEP_TEL_GAIN_MODE_ROI_1 183
#define LEP_TEL_GAIN_MODE_ROI_2 184
#define LEP_TEL_GAIN_MODE_ROI_3 185
#define LEP_TEL_TLIN_ENABLE     208
#define LEP_TEL_TLIN_RES        209
#define LEP_TEL_SPOT_MEAN       210
#define LEP_TEL_SPOT_MAX        211
#define LEP_TEL_SPOT_MIN        212
#define LEP_TEL_SPOT_POP        213
#define LEP_TEL_SPOT_Y1         214
#define LEP_TEL_SPOT_X1         215
#define LEP_TEL_SPOT_Y2         216
#define LEP_TEL_SPOT_X2         217



//
// Telemetry Status DWORD mask
//
#define LEP_STATUS_FFC_DESIRED 0x00000008
#define LEP_STATUS_FFC_STATE   0x00000030
#define LEP_STATUS_AGC_STATE   0x00001000
#define LEP_STATUS_SHTR_LO     0x00008000
#define LEP_STATUS_OT_IMM      0x00100000

//
// Telemetry Status FFC State
//
#define LEP_FFC_STATE_IDLE     0x00000000
#define LEP_FFC_STATE_IMM      0x00000010
#define LEP_FFC_STATE_RUN      0x00000020
#define LEP_FFC_STATE_CMPL     0x00000030

//
// Lepton Types
//
#define LEP_TYPE_3_5           0
#define LEP_TYPE_3_0           1
#define LEP_TYPE_3_1           2
#define LEP_TYPE_UNK           3



//
// Lepton Utilities API
//
bool lepton_init();
bool lepton_is_radiometric();
int lepton_get_model();
void lepton_agc(bool en);
void lepton_ffc();
void lepton_gain_mode(uint8_t mode);
void lepton_spotmeter(uint16_t r1, uint16_t c1, uint16_t r2, uint16_t c2);
void lepton_emissivity(uint16_t e);

uint32_t lepton_get_tel_status(uint16_t* tel_buf);

float lepton_kelvin_to_C(uint32_t k, float lep_res);

#endif /* LEPTON_UTILITIES_H */