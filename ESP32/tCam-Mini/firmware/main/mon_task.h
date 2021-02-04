/*
 * Mon Task
 *
 * Monitor system CPU and memory utilization for debugging and application turning.
 * This task should only be included during development.
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
 * along with firecam.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef MON_TASK_H
#define MON_TASK_H


//
// Mon Task Constants
//
#define MON_SAMPLE_MSEC 5000
#define MON_MAX_TASKS   20

// Uncomment to enable monitoring of memory and/or tasks
#define MON_MEM
#define MON_TASKS

// Uncomment for a more verbose memory monitoring output
//#define MON_MEM_VERBOSE



//
// Mon Task API
//
void mon_task();

#endif /* MON_TASK_H */