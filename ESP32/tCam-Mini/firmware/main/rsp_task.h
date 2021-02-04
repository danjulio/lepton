/*
 * Response Task
 *
 * Implement the response transmission module under control of the command module.
 * Responsible for sending responses to the connected client.  Sources of responses
 * include the command task, lepton task and file task.
 *
 * Copyright 2020-2021 Dan Julio
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
 * along with firecam.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef RSP_TASK_H
#define RSP_TASK_H

#include <stdint.h>


//
// RSP Task Constants
//

// Task evaluation interval
#define RSP_TASK_EVAL_NORM_MSEC 50
#define RSP_TASK_EVAL_FAST_MSEC 20

// Maximum send packet size (less than a MTU)
#define RSP_MAX_TX_PKT_LEN 1280

// Response Task notifications
#define RSP_NOTIFY_CMD_GET_IMG_MASK    0x00000001
#define RSP_NOTIFY_CMD_STREAM_ON_MASK  0x00000002
#define RSP_NOTIFY_CMD_STREAM_OFF_MASK 0x00000004
#define RSP_NOTIFY_LEP_FRAME_MASK_0    0x00000010
#define RSP_NOTIFY_LEP_FRAME_MASK_1    0x00000020

//
// RSP Task API
//
void rsp_task();
void rsp_set_stream_parameters(uint32_t delay_ms, uint32_t num_frames);

#endif /* RSP_TASK_H */