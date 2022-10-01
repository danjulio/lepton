/*
 * Response Task
 *
 * Implement the response transmission module under control of the command module.
 * Responsible for sending responses to the connected client.  Sources of responses
 * include the command task, lepton task and file task.
 *
 * Copyright 2020-2022 Dan Julio
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
#ifndef RSP_TASK_H
#define RSP_TASK_H

#include <stdint.h>


//
// RSP Task Constants
//

// Info Message values
#define RSP_INFO_CMD_NACK     0
#define RSP_INFO_CMD_ACK      1
#define RSP_INFO_CMD_UNIMPL   2
#define RSP_INFO_CMD_BAD      3
#define RSP_INFO_INT_ERROR    4
#define RSP_INFO_DEBUG_MSG    5
#define RSP_INFO_UPD_STATUS   6

// Task evaluation interval
#define RSP_TASK_EVAL_NORM_MSEC 50
#define RSP_TASK_EVAL_FAST_MSEC 10

// Maximum send packet size (less than a MTU)
#define RSP_MAX_TX_PKT_LEN 1280

// Maximum cam_info string length
#define RSP_MAX_CAM_INFO_LEN 128

// Maximum wait for the user to respond after firmware update request
#define RSP_MAX_FW_UPD_REQ_WAIT_MSEC 60000

// Maximum number of times to try to get a FW chunk
#define FW_REQ_MAX_ATTEMPTS 6

// Maximum wait time for a fw_segment response to a get_fw request from this firmware before retrying
#define RSP_MAX_FW_UPD_GET_WAIT_MSEC 10000

// Response Task notifications
#define RSP_NOTIFY_CMD_GET_IMG_MASK    0x00000001
#define RSP_NOTIFY_CMD_STREAM_ON_MASK  0x00000002
#define RSP_NOTIFY_CMD_STREAM_OFF_MASK 0x00000004
#define RSP_NOTIFY_LEP_FRAME_MASK_0    0x00000010
#define RSP_NOTIFY_LEP_FRAME_MASK_1    0x00000020
#define RSP_NOTIFY_FW_UPD_REQ_MASK     0x00000100
#define RSP_NOTIFY_FW_UPD_SEG_MASK     0x00000200
#define RSP_NOTIFY_FW_UPD_EN_MASK      0x00000400
#define RSP_NOTIFY_FW_UPD_END_MASK     0x00000800



//
// RSP Task API
//
void rsp_task();
void rsp_set_stream_parameters(uint32_t delay_ms, uint32_t num_frames);
void rsp_set_cam_info_msg(uint32_t info_value, char* info_string);
void rsp_set_fw_upd_req_info(uint32_t length, char* version);
void rsp_set_fw_upd_seg_info(uint32_t start, uint32_t length);

#endif /* RSP_TASK_H */