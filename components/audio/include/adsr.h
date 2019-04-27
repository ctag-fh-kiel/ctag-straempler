#pragma once

#include "audio_types.h"
#include "audio_luts.h"

#define ENV_LUT_SIZE 512

/**
 * @brief pass over adsr to be initialized
 * 
 * @param adsr adsr struct to be initialised
 */

void init_adsr(adsr_t* adsr);

/**
 * @brief needs to be called when the sound gets retriggered, to set the right eg values
 * 
 * @param adsr the adsr object, which holds all needed data
 * @param ev the button state 1 == button down 2 == button up 
 */

void update_eg_state(adsr_t* adsr, int ev);

/**
 * @brief Get the next envelope value
 * 
 * @param adsr the adsr object, which holds all needed data
 * @param sample_left the left sample to perform the eg on
 * @param sample_right the right sample to perform the eg on
 */

void get_next_eg_val(adsr_t* adsr, float* sample_left, float* sample_right);

void process_fade(voice_t* voice, float* buffer, int buffer_size);

float fade(fade_t* fade);

void process_fade_state(voice_t* voice, int ev);

void update_fade_state(fade_t* fade, int ev);

void init_fade(voice_t* voice);

void calculate_adsr_phase_increment(adsr_t* adsr);