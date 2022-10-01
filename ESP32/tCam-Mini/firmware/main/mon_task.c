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
#include "mon_task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include <stdint.h>


//
// Mon Task internal variables
//
static const char* TAG = "mon_task";

static TaskStatus_t* start_task_sample_array;
static TaskStatus_t* end_task_sample_array;



//
// Mon Task Forward Declarations for internal functions
//
static bool init_mon_task();
#ifdef MON_MEM
static void print_memory_stats();
#endif
#ifdef MON_TASKS
static void print_task_stats();
#endif



//
// Mon Task API
//
void mon_task()
{
	ESP_LOGI(TAG, "Start task");
	
	// Allocate memory for our statistics in the external SPI SRAM (least impact)
	if (!init_mon_task()) {
		ESP_LOGE(TAG, "Could not allocate buffers - bailing");
		vTaskDelete(NULL);
	}
	
	// Let the system start up
	vTaskDelay(pdMS_TO_TICKS(5000));
	
	while (1) {
		// This shouldn't happen, but we check to protect from overrunning our sample arrays
		if (uxTaskGetNumberOfTasks() > MON_MAX_TASKS) {
			ESP_LOGE(TAG, "More than MON_MAX_TASKS tasks (%d) - bailing", uxTaskGetNumberOfTasks());
			vTaskDelete(NULL);
		}
		
#ifdef MON_MEM
		print_memory_stats();
#endif
#ifdef MON_TASKS
		print_task_stats();
#endif

		vTaskDelay(pdMS_TO_TICKS(MON_SAMPLE_MSEC));
	}
}



//
// Mon Task internal functions
//
static bool init_mon_task()
{
	start_task_sample_array = heap_caps_malloc(sizeof(TaskStatus_t) * MON_MAX_TASKS, MALLOC_CAP_SPIRAM);
	if (start_task_sample_array == NULL) {
		return false;
	}
	
	end_task_sample_array = heap_caps_malloc(sizeof(TaskStatus_t) * MON_MAX_TASKS, MALLOC_CAP_SPIRAM);
	if (end_task_sample_array == NULL) {
		return false;
	}
	
	return true;
}


#ifdef MON_MEM
static void print_memory_stats()
{
#ifdef MON_MEM_VERBOSE
	heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
	heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);
#else
	ESP_LOGI(TAG, "Int Heap free: %d / Min: %d - SPIRAM free: %d / Min %d", 
	        heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
	        heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL),
	        heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
	        heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM));
#endif
	heap_caps_check_integrity_all(true);
}
#endif


#ifdef MON_TASKS
static void print_task_stats()
{
	uint32_t start_run_time, end_run_time;
	uint32_t total_elapsed_time;
	uint32_t task_elapsed_time;
	uint32_t percentage_time;
	volatile UBaseType_t start_array_size, end_array_size;
    
    // Sample the tasks over a 1 second period so we can compute some stats
    start_array_size = uxTaskGetSystemState(start_task_sample_array, MON_MAX_TASKS, &start_run_time);
    vTaskDelay(pdMS_TO_TICKS(1000));
    end_array_size = uxTaskGetSystemState(end_task_sample_array, MON_MAX_TASKS, &end_run_time);
	
    // Compute and display stats
    total_elapsed_time = end_run_time - start_run_time;
    if (total_elapsed_time != 0) {
    	ESP_LOGI(TAG, "Task Statistics:");
    	printf("\tTask\t\tRun Time\t%%\tPri\tStack Highwater\n");
    	
    	// Match each task in start_array to those in the end_array
    	for (int i = 0; i < start_array_size; i++) {
        	int k = -1;
        	for (int j = 0; j < end_array_size; j++) {
            	if (start_task_sample_array[i].xHandle == end_task_sample_array[j].xHandle) {
               	 k = j;
                	// Mark that task have been matched by overwriting their handles
                	start_task_sample_array[i].xHandle = NULL;
                	end_task_sample_array[j].xHandle = NULL;
                	break;
            	}
        	}
        	// Check if matching task found
        	if (k >= 0) {
            	task_elapsed_time = end_task_sample_array[k].ulRunTimeCounter - start_task_sample_array[i].ulRunTimeCounter;
            	percentage_time = (task_elapsed_time * 100UL) / (total_elapsed_time * portNUM_PROCESSORS);
            	printf("\t%16s\t%d\t%d%%\t%d\t%d\n", start_task_sample_array[i].pcTaskName,
            	       task_elapsed_time, percentage_time,
            	       start_task_sample_array[i].uxCurrentPriority,
            	       start_task_sample_array[i].usStackHighWaterMark);
        	}
    	}
    
    	// Print unmatched tasks
    	for (int i = 0; i < start_array_size; i++) {
        	if (start_task_sample_array[i].xHandle != NULL) {
           	 ESP_LOGI(TAG, "Task %s deleted\n", start_task_sample_array[i].pcTaskName);
        	}
    	}
    	for (int i = 0; i < end_array_size; i++) {
       	 if (end_task_sample_array[i].xHandle != NULL) {
            	ESP_LOGI(TAG, "Task %s created\n", end_task_sample_array[i].pcTaskName);
        	}
    	}
    }
}
#endif
