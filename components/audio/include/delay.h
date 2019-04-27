#pragma once
#include <stdint.h>

#define DELAY_REDUCE_NOISE 1
#define DELAY_NOISE_THRESHOLD 0.0001

enum delay_mode{
    DELAY_STEREO = 0x00,
    DELAY_PINGPONG = 0x01
};

typedef struct{
    float *bufL, *bufR;
    uint32_t bufLen;
    uint32_t pos;
    float sampleRate, msMaxLength, msLength;
    uint32_t tapOffset;
    float fTapOffset;
    float feedback, pan, volume;
    uint8_t mode;
}delay_t;

typedef struct{
    float msLength;
    float feedback;
    float volume;       
    float pan;  
    uint8_t mode;    
}delay_cfg_t;

// param description
// sampleRate -> sample rate of system
// msMaxLength -> maximum delay length in ms
// feedback -> delay feedback, 0.0 = no feedback, 1.0 = total feedback, >1.0 build up, < 0.0 inverted phase
// mode -> either stereo or pingpong
// pan -> 0.0 hard left, 0.5 center, 1.0 hard right
// volume -> 0.0 silent, 1.0 max volume, >1.0 amplification
void delay_create(delay_t *delay, float sampleRate, float msMaxLength, delay_cfg_t cfg);
void delay_set_params_smooth(delay_t *delay, delay_cfg_t cfg);
void delay_set_params(delay_t *delay, delay_cfg_t cfg);
void delay_free(delay_t *delay);
void delay_clear(delay_t *delay);
void delay_process(delay_t *delay, const float* bufIn, float* bufOut, uint32_t bufLen);