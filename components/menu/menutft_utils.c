#include "menutft_utils.h"

//Flush conditions, for additional conditions expand default_conditions or add new array and size
const int default_conditions[9] = {8,9,10,98,99,100, 998, 999, 1000};
const int default_cond_size = sizeof(default_conditions) / sizeof(int);
const int adsr_conditions[8] = {9, 10, 98, 101, 981, 1006, 9748, 10000};
const int adsr_cond_size = sizeof(adsr_conditions) / sizeof(int);
const int filter_conditions[6] = {120, 121, 293, 294, 466, 467};
const int filter_cond_size = sizeof(filter_conditions)/sizeof(int);
const int negpos_conditions[6] = {0, 1, 9, 10, 99, 100};
const int negpos_cond_size = sizeof(negpos_conditions) / sizeof(int);
const int playmode_conditions[4] = {79, 80, 799, 800};
const int playmode_cond_size = sizeof(playmode_conditions) / sizeof(int);


uint16_t menuTFTScaleToWidth(uint16_t val, uint16_t log_max, uint16_t width){
    uint32_t value = val * width; 
   return (uint16_t) ((value / log_max) & 0xFFFFU);
}

int16_t menuTFTScaleToRange(int16_t val, int16_t log_max, int16_t phys_max){
    int32_t value = val * phys_max; 
   return (int16_t) ((value / log_max) & 0xFFFFU);
}

void menuTFTFlushValue(int value, int multiplier, const int* cond, const int* cond_size, color_t* color){
    if(cond != NULL){
        for(int i = 0; i < *cond_size; i++){
            if(value == cond[i]){
                TFT_fillRect(_width/2, 3 + (TFT_getfontheight() + 3) * multiplier, _width/2, TFT_getfontheight(), *color);
                break;
            }
        }
    }else{
        TFT_fillRect(_width/2, 3 + (TFT_getfontheight() + 3) * multiplier, _width/2, TFT_getfontheight(), *color);
    }
}

void menuTFTFlush(int multiplier, color_t* color){
    TFT_fillRect(_width/2, 3 + (TFT_getfontheight() + 3) * multiplier, _width/2, TFT_getfontheight(), *color);
}

void menuTFTFlushMenuDataRect(){
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    TFT_fillWindow(TFT_BLACK);
}