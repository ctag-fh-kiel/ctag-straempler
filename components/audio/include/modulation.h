#pragma once

#include "freertos/FreeRTOS.h"
#include "audio_types.h"
#include "freertos/queue.h"
#include "fixed.h"
#include "math.h"
#include "audio_luts.h"
#include "freertos/task.h"
#include "fill_buffer.h"
#include "esp_log.h"

/**
 * @brief initializes the given ui parameter struct
 * 
 * @param params pointer to given parameter object
 */

void init_ui_params(ui_param_holder_t* params);


/**
 * @brief parse all params from ui queue into voice and ui parameter holder
 * 
 * @param queue ui queue handle
 * @param voice pointer to voice struct
 * @param ui_params pointer to ui parameter holder
 * @param params pointer to parameter object
 */

void parse_voice_param_data(xQueueHandle* queue, voice_t* voice, ui_param_holder_t* ui_params, param_data_t* params);


/**
 * @brief 
 * 
 * @param queue 
 * @param task_handle 
 * @param voice 
 * @param vid 
 */

void parse_play_direction(xQueueHandle* queue, TaskHandle_t* task_handle, voice_t* voice, int vid);


/**
 * @brief 
 * 
 * @param queue 
 * @param play_state_data 
 * @param ui_params 
 * @param task_handle 
 * @param voice 
 * @param file 
 * @param play_modes 
 * @param vid 
 */
void parse_play_state_data(xQueueHandle* queue, play_state_data_t* play_state_data, ui_param_holder_t* ui_params, TaskHandle_t* task_handle, voice_t* voice, audio_f_t* file, void (**play_modes)(void*,void*,void*), int vid);