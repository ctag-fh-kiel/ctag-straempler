#pragma once
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "audio_types.h"
#include "dsp_lib.h"
#include "fill_buffer.h"

void initAudio(xQueueHandle ui_queue_v0, xQueueHandle ui_queue_v1, xQueueHandle eff_queue, 
    xQueueHandle cv_queue_v0, xQueueHandle cv_queue_v1, xQueueHandle _mode_handle_v0, xQueueHandle _mode_handle_v1, 
    xQueueHandle matrix_event_queue, xQueueHandle ui_ev_q);
void assignAudioFiles();
void enableTrigModeLatch(uint8_t vid);
void disableTrigModeLatch(uint8_t vid);




