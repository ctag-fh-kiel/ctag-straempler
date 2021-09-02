#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "sampler_fifo.h"
#include "esp_vfs_fat.h"


#define MWN 16
#define SZ_TBL 256 // size of cluster map table for fat fs, needed for fastseek option

typedef enum {
    OFF = 0x00,
    A = 0x01,
    D = 0x02,
    S = 0x04,
    R = 0x08,
    LISTEN = 0x10
} eg_state_t;

//Changes on Rev
typedef enum{
    outer_left = 4,
    inner_left = 6,
    inner_right = 5,
    outer_right = 7
} poti_num_t;

typedef enum{
    SINGLE = 0x00,
    LOOP = 0x01,
    PIPO = 0x02,
} voice_play_state_t;

typedef enum{
    MTX_NONE = 0x00,
    MTX_V0_VOLUME,
    MTX_V0_PAN,
    MTX_V0_PITCH,
    MTX_V0_PB_SPEED,
    MTX_V0_DIST_DRIVE,
    MTX_V0_DLY_SEND,
    MTX_V0_FILTER_BASE,
    MTX_V0_FILTER_WIDTH,
    MTX_V0_FILTER_Q,
    MTX_V0_ADSR_ATTACK,
    MTX_V0_ADSR_DECAY,
    MTX_V0_ADSR_SUSTAIN,
    MTX_V0_ADSR_RELEASE,
    MTX_V0_MODE_START,
    MTX_V0_MODE_LSTART,
    MTX_V0_MODE_LEND, 
    MTX_V1_VOLUME,
    MTX_V1_PAN,
    MTX_V1_PITCH,
    MTX_V1_PB_SPEED,
    MTX_V1_DIST_DRIVE,
    MTX_V1_DLY_SEND,
    MTX_V1_FILTER_BASE,
    MTX_V1_FILTER_WIDTH,
    MTX_V1_FILTER_Q,
    MTX_V1_ADSR_ATTACK,
    MTX_V1_ADSR_DECAY,
    MTX_V1_ADSR_SUSTAIN,
    MTX_V1_ADSR_RELEASE,
    MTX_V1_MODE_START,
    MTX_V1_MODE_LSTART,
    MTX_V1_MODE_LEND, 
    MTX_DELAY_TIME,
    MTX_DELAY_PAN,
    MTX_DELAY_FB,
    MTX_DELAY_VOL
} matrix_param_t;

typedef struct{
    matrix_param_t dst;
    float amt;
} matrix_row_t;


typedef struct{
    matrix_param_t dst;
    uint8_t amt;
} matrix_ui_row_t;

typedef struct{
    uint8_t source;
    uint8_t amount;
    matrix_param_t changed_param;
}matrix_event_t;

typedef struct
{
    uint32_t fpos;
    uint32_t fsize;
    char fname[32];
    FIL fil;
    DWORD clmt[SZ_TBL];
} audio_f_t;

typedef struct
{
    unsigned int rpos, len;
    void *buf;
} audio_b_t;

typedef struct{
    void (*play_mode) (void*, void*, void*);
    bool is_playback_direction_forward;
    bool is_pipo_playback_forward;
    uint32_t sample_start, loop_start, loop_end;
    float pitch_increment, phase;
    int32_t last_cummulated_sample;
    voice_play_state_t mode;
} voice_play_mode_t;

typedef struct{
    float a1, a2, b0, b1, b2;
} BiQuadCoeffs_t;

typedef enum {lowpass, highpass} filtertype_t;

typedef struct{
    float cutoff;
    float resonance;
    float sample_rate;
    float z1, z2;
    int type;
    BiQuadCoeffs_t* coeffs;
}  BiQuad_t;

typedef struct{
    float amp_fator_positive;
    float amp_fator_negative;
    int is_active;
} WaveShaper_t;

typedef struct{
    BiQuad_t* filter_left;
    BiQuad_t* filter_right;
    int is_active;
} Filter_t;

typedef struct{
    float position;
    float angle;
} PanPos_t;

typedef struct{
    float attack_time, decay_time, sustain_level, release_time, last_val;
    float adsr_phase, adsr_increment;
    int adsr_state;
} adsr_t;

typedef struct{
    float phase, increment, fade_time;
    int state;
    float start_val;
} fade_t;

typedef struct
{
    audio_b_t *voice_buffer;
    adsr_t adsr;
    Filter_t* lowpass_filter;
    Filter_t* highpass_filter;
    voice_play_mode_t playback_engine;
    fade_t fade[2];
} voice_t;

typedef struct{
    bool is_shaper_active;
    bool is_pitch_cv_active;
    float delay_send;
    float playback_speed;
    uint32_t filter_base_index, filter_width_index;
    float volume, pan, dist_amp, resonance;
    int8_t pitch;
    float attack_time, decay_time, sustain_level, release_time;
    uint32_t sample_start, loop_start, loop_end, mode;
} ui_param_holder_t;

typedef struct
{
    int ctrl[8];
} ctrl_t;
typedef struct
{
    uint32_t attack;
    uint32_t decay;
    uint8_t sustain;        //Q1.7 
    uint32_t release;
} adsr_data_t;

typedef struct 
{
    voice_play_state_t mode;
    uint16_t start;         //Q13.3 - 0.00 - 100.00
    uint16_t loop_start;    //Q13.3 - 0.00 - 100.00
    uint16_t loop_end;      //Q13.3 - 0.00 - 100.00
    uint16_t loop_position; //Q13.3 - 0.00 - 100.00
    uint16_t loop_length;   //Q13.3 - 0.00 - 100.00
} play_state_data_t;

typedef struct
{
    bool is_active;
    uint16_t base;
    uint16_t width;
    uint16_t q;     //Q10.6
} filter_data_t;


typedef struct{
    bool is_active;
    uint16_t time;
    uint8_t feedback;           //Q1.7 
    uint8_t volume;       
    uint8_t pan;          
    uint8_t mode;         //enum
} delay_data_t;

typedef struct{
    bool is_active;
    uint8_t volume;       
    int16_t pan;
    uint8_t delay_send;
} ext_in_data_t;

typedef struct{
    float volume;
    float pan;
    float delay_send;
} ext_in_cfg_t;

typedef struct{
    delay_data_t delay;
    ext_in_data_t extInData;
} effect_data_t;

typedef struct 
{
    bool pitch_cv_active;
    bool trig_mode_latch;
    uint8_t volume;         
    uint8_t pitch;          
    int16_t pan;             //Range -100/100
    uint16_t dist_drive;    
    bool dist_active;
    int16_t playback_speed;  //Range -100/100
    uint8_t delay_send;
    filter_data_t filter;
    adsr_data_t adsr;
    play_state_data_t play_state;
} param_data_t;
