/*
 * SIF Cmd Task
 *
 * Implement the command processing module including management of the Serial interface.
 * Designed to be executed if Serial communication is enabled.
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
 * along with tCam.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef SIF_CMD_TASK_H
#define SIF_CMD_TASK_H

#include <stdbool.h>
#include <stdint.h>


//
// SIF CMD Task Constatns
//

// Evaluation interval
#define SIF_CMD_EVAL_MSEC 50



//
// SIF CMD Task API
//
void sif_cmd_task();

#endif /* SIF_CMD_TASK_H */