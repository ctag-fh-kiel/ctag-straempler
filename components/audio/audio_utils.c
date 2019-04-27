#include "audio_utils.h"

void reverseBuffer32(size_t size, int32_t *buf){
    for (size_t a = 0; a < size >> 1; a++) {         
        int32_t temp = buf[a];         
        buf[a] = buf[size - a - 1];         
        buf[size - a - 1] = temp;     
    }
}


void reverseBuffer16(size_t size, int16_t *buf){
    for(int16_t a = 0; a < size >> 1; a++){
        int16_t temp =  buf[a];         
        buf[a] = buf[size - a - 1];         
        buf[size - a - 1] = temp;     
    }
}


void reverseBuffer8(size_t size, int8_t *buf){
    for (size_t a = 0; a < size >> 1; a++) {         
        int8_t temp = buf[a];         
        buf[a] = buf[size - a - 1];         
        buf[size - a - 1] = temp;     
    }
}


uint32_t normEncData(uint32_t val){
    
    if((val & 1) == 0){         
        return val;     
    }     
    return val + 1;
}

uint32_t norm32(uint32_t val){
    if(val % 32){
        return (val >> 5) << 5;
    }
    return val;
}

int sign(int x){
    return (x > 0) - (x < 0);
}

void accumulateBufferHardLimit(const int16_t *src, int16_t *dst, int len){
    for (int i = 0; i < len; i++)
    {
        int val = (int)dst[i] + (int)src[i];
        if (val > 32767)
            val = 32767;
        if (val < -32768)
            val = -32768;
        dst[i] = (int16_t)val;
    }
}

void multShiftBuffer(int16_t *buf, int len, int f, int sh){
    for(int i=0;i<len;i++){
        int val = buf[i] * f;
        val >>= sh;
        buf[i] = (int16_t) val;
    }
}

void multShiftBufferLI(int16_t *buf, int len, int pre, int f, int sh){
    for(int i=0;i<len;i++){
        int w = (f - pre) * i + pre * len;
        w /= len;
        //printf("w: %d\n", w);
        int val = buf[i] * w;
        val >>= sh;
        buf[i] = (int16_t) val;
    }
}

void fillValue(int16_t *buf, int len, int v){
    static int pre = 0;
    int w = 0;
    //ESP_LOGI("VAL", "%d", (int16_t) v);
    for(int i=0;i<len;i++){
        w = (v - pre) * i + pre * len;
        w /= len;
        buf[i] = (int16_t) w;
        //buf[i] = (int16_t) v;
    }
    pre = w;
}

int checkBlock(uint32_t *lStart, uint32_t *lEnd){
    if(((*lStart >> 13) + 1) == ((*lEnd >> 13 ) + 1)){
        return 1;
    }
    return 0;
}

int getBlockCount(int32_t* pos){
    return (*pos >> 13) + 1;
}

int32_t nullCheck(int32_t val){
    if(val <= 0){
        return 0;
    }
    return val;
}

uint32_t calcRpos(int b, uint32_t* loopStart ){
    return *loopStart - 8192 *(b-1);
}

void shiftRI(uint16_t* arr, int val, int size){
    for(int i = 0; i < size; i++)
    {
        arr[i] >>= val;
    }
}

uint32_t get_audio_file_size(FILE* f)
{
    struct stat file_info;
    int fd = fileno_unlocked(f);
    if(fstat(fd, &file_info) == 0){
        return file_info.st_size;
    }else{
        ESP_LOGE("FILE", "Was not able to get file size");
    }
    return 0;
}