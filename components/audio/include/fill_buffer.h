#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "audio_utils.h"
#include "esp_heap_caps.h"
#include "audio_types.h"
#include <string.h>

#define SD_BUF_SZ 8192


/**
 * @brief Initialise a play mode object which holds all data, which is needed for data loading from a file
 * 
 * @return voice_play_mode_t* 
 */

void init_play_mode(voice_play_mode_t* pm);


/**
 * @brief fills given audio buffer with data from a given audiofile in one shot mode
 * 
 * @param playback_engine object which holds information on sample positon
 * @param buffer audio buffer which gets filled
 * @param file audio file from where data should be read
 */

void fill_buffer_one_shot(void* playback_engine, void* buffer, void* file);


/**
 * @brief fills given audio buffer with data from a given audiofile in loop mode
 * 
 * @param playback_engine object which holds information on sample positon
 * @param buffer audio buffer which gets filled
 * @param file audio file from where data should be read
 */

void fill_buffer_loop(void* playback_engine, void* buffer, void* file);


/**
 * @brief fills given audio buffer with data from a given audiofile in pipo mode
 * 
 * @param _playback_engine object which holds information on sample positon
 * @param _buffer audio buffer which gets filled
 * @param _file audio file from where data should be read
 */

void fill_buffer_pipo(void* _playback_engine, void* _buffer, void* _file);


/**
 * @brief calls the callback function for filling a voice buffer
 * 
 * @param playback_engine holds the playback function
 * @param buffer audio buffer which gets filled
 * @param file audio file from where data should be read
 */

void fill_audio_buffer(voice_play_mode_t* playback_engine, audio_b_t* buffer, audio_f_t* file, SemaphoreHandle_t* mutex);


/**
 * @brief needs to be called when sample is retriggered, takes care of loading the buffer from the selected start point
 * 
 * @param playback_engine holds the playback function
 * @param buffer audio buffer which gets filled
 * @param file audio file from where data should be read
 */

void jump_to_start(voice_play_mode_t* playback_engine, audio_b_t* first_buffer, audio_b_t* second_buffer, audio_f_t* file, SemaphoreHandle_t* mutex);


/**
 * @brief needs to be called when play mode changes to make sure right data is in first buffer
 * 
 * @param playback_engine holds the playback function
 * @param buffer first buffer which gets played first when sample is triggered
 * @param file file from where data should be read
 */

void refill_first_buf(voice_play_mode_t* playback_engine, audio_b_t* buffer, audio_f_t* file, SemaphoreHandle_t* mutex);