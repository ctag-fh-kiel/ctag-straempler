#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "delay.h"
#include "esp_heap_caps.h"
#include "esp_log.h"


void delay_create(delay_t *delay, float sampleRate, float msMaxLength, delay_cfg_t cfg){
    delay->msMaxLength = msMaxLength;
    delay->msLength = cfg.msLength;
    delay->sampleRate = sampleRate;
    delay->tapOffset = (uint32_t)(sampleRate * (msMaxLength - cfg.msLength) / 1000.0);
    delay->fTapOffset = delay->tapOffset;
    delay->feedback = cfg.feedback;
    delay->pan = cfg.pan;
    delay->volume = cfg.volume;
    delay->pos = 0;
    delay->mode = cfg.mode;

    delay->bufLen = ceilf(sampleRate * msMaxLength/1000.0);
    delay->bufL = heap_caps_calloc(delay->bufLen, sizeof(float), MALLOC_CAP_SPIRAM); // mono
    if(delay->bufL == NULL){
        ESP_LOGE("DELAY", "Could not allocate memory --> delay buffer!");
    }
    delay->bufR = heap_caps_calloc(delay->bufLen, sizeof(float), MALLOC_CAP_SPIRAM); // mono
    if(delay->bufR == NULL){
        ESP_LOGE("DELAY", "Could not allocate memory --> delay buffer!");
    }
}

void delay_free(delay_t *delay){
    free(delay->bufL);
    free(delay->bufR);
}

void delay_set_params_smooth(delay_t *delay, delay_cfg_t cfg){
    delay->msLength = cfg.msLength;
    //if(delay->msLength > delay->msMaxLength) delay->msLength = delay->msMaxLength;
    delay->fTapOffset = 0.995 * delay->fTapOffset + 0.005 * (delay->msMaxLength - delay->msLength) * delay->sampleRate / 1000.0;
    delay->tapOffset = (uint32_t) delay->fTapOffset;
    delay->feedback = 0.8 * delay->feedback + 0.2 * cfg.feedback;
    delay->pan = 0.8 * delay->pan + 0.2 * cfg.pan;
    delay->volume = 0.8 * delay->volume + 0.2 * cfg.volume;
    delay->mode = cfg.mode;
}

void delay_set_params(delay_t *delay, delay_cfg_t cfg){
    delay->msLength = cfg.msLength;
    //if(delay->msLength > delay->msMaxLength) delay->msLength = delay->msMaxLength;
    delay->fTapOffset = (delay->msMaxLength - delay->msLength) * delay->sampleRate / 1000.0;
    delay->tapOffset = (uint32_t) delay->fTapOffset;
    delay->feedback = cfg.feedback;
    delay->pan = cfg.pan;
    delay->volume = cfg.volume;
    delay->mode = cfg.mode;
}

void delay_clear(delay_t *delay){
    memset(delay->bufL, delay->bufLen, sizeof(float));
    memset(delay->bufR, delay->bufLen, sizeof(float));
}

void delay_process(delay_t *delay, const float* bufIn, float* bufOut, uint32_t bufLen){
    float tempL, tempR;
    float pan = 2.0 * delay->pan;
    float negPan = 2.0 * (1.0 - delay->pan);
    for(uint32_t i=0; i<bufLen/2; i++){
        uint32_t pos;

        // read
        pos = (delay->pos + delay->tapOffset) % delay->bufLen;
        tempL = delay->bufL[pos];
        pos = (delay->pos + delay->tapOffset) % delay->bufLen;
        tempR = delay->bufR[pos];

        // write
        pos = delay->pos;
        if(delay->mode == DELAY_PINGPONG){
            delay->bufL[pos] = tempR * delay->feedback;
            delay->bufR[pos] = tempL * delay->feedback;
        }else{
            delay->bufL[pos] = tempL * delay->feedback;
            delay->bufR[pos] = tempR * delay->feedback;
        }
        delay->bufL[pos] += (bufIn[i*2] + bufIn[i*2 + 1]) * negPan;
        delay->bufR[pos] += (bufIn[i*2] + bufIn[i*2 + 1]) * pan;


#ifdef DELAY_REDUCE_NOISE
        if(delay->bufL[pos] > -DELAY_NOISE_THRESHOLD && delay->bufL[pos] < DELAY_NOISE_THRESHOLD) // remove noise
            delay->bufL[pos] = 0.0;
        if(delay->bufR[pos] > -DELAY_NOISE_THRESHOLD && delay->bufR[pos] < DELAY_NOISE_THRESHOLD) // remove noise
            delay->bufR[pos] = 0.0;
#endif
        
        // update position
        delay->pos++;
        delay->pos %= delay->bufLen;

        // set volume
        bufOut[i*2] += tempL * delay->volume;
        bufOut[i*2 + 1] += tempR * delay->volume;
    }
}