#pragma once
#include "math.h"
#include "stdint.h"
#include "stdlib.h"
#include <assert.h>
#include <string.h>
#include "audio_types.h"
#include "audio_luts.h"
#include "sampler_fifo.h"
#include "fixed.h"


#define M_PI 3.14159265358979323846

#define PIOVR_2  (M_PI * 0.5)
#define ROOT_2_OVR_2  (sqrt(2.0) * 0.5)
#define FOUR_OVER_PI    1.2732395447351626861510701069801f
#define PI_2    1.5707963267948966192313216916398f
#define PI_4    0.78539816339744830961566084581988f


void calculate_coeffs(Filter_t* filter);
float fasttanh(float x);
float fasttan(float x);
void reset_filter(Filter_t* filter);
int32_t round_float_int(float in);
int32_t hard_limit(int32_t val);
float distort_sound(WaveShaper_t* shaper, float value);
void apply_volume(uint16_t* ctrlData, int16_t* buf, voice_t* voice, int len);
void apply_voice_fx(float* buf, int len, voice_t* voice, uint16_t* ctrl_data, int vid, float vol);
void set_resonance(Filter_t* filter, float resonance);
void set_cutoff(Filter_t* filter, float cutoff);
void init_shaper(WaveShaper_t* shaper);
void set_distortion_amp(WaveShaper_t* shaper, float value);
void init_pan(PanPos_t* pan);
void set_position(PanPos_t* pan, float pos);
void process_left_channel(PanPos_t* pan, float* left_channel);
void process_right_channel(PanPos_t* pan, float* right_channel);
void stereo_balance(float pan_val, float* sample_left, float* sample_right);
void buffer_copy(float* dst, int dst_start, float* src, int src_start, int len);
void set_cutoff_values(voice_t* voice, uint16_t* ctrl_data, int vid);
void filter_sound(Filter_t *filter, float* out_left, float* out_rights);
void init_filter(Filter_t* filter, filtertype_t f_type);
void buffer_add(float* dst, int dst_start,float* src, int src_start, int len);
void buffer_copy_stereo(float* dst, int32_t dst_start,float* src, int32_t src_start, int32_t len, int32_t channel);
void set_filter_values_ui(voice_t* voice, uint16_t base, uint16_t width, float q, int vid);
void set_filter_params(Filter_t* filter, float cutoff, float resonance);
int32_t modulateTimeParameter(int index, uint16_t* ctrl_data, matrix_row_t* matrix, uint32_t ui_param, uint32_t max_time);
float modulate_unipolar(float center, float max_min, float src, float amt);
