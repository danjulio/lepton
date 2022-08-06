/*
 * Lepton Task
 *
 * Contains functions to initialize the Lepton and then sampling images from it,
 * making those available to other tasks through a shared buffer and event
 * interface.  This task should be run on the "PRO" core (0) while other application
 * tasks run on the "PRO" core (1).
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
 * along with firecam.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef LEP_TASK_H
#define LEP_TASK_H

#include <stdint.h>



//
// LEP Task Constants
//

// Number of consecutive VoSPI resynchronization attempts before attempting to reset
#define LEP_SYNC_FAIL_FAULT_LIMIT 10

// Reset fail delay before attempting a re-init (seconds)
#define LEP_RESET_FAIL_RETRY_SECS 60



//
// LEP Task API
//
void lep_task();

#endif /* LEP_TASK_H */