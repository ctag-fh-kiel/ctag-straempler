#include "menu_playmode.h"

void updateStart(play_state_data_t* data, uint16_t* loopMarker, uint16_t* prevLoopMarker, const int* width, const int* height){
    loopMarker[I_START] = menuTFTScaleToWidth(data->start, 800, *width);
    if(loopMarker[I_START] == prevLoopMarker[I_START]) return;

    //When in single-shot-mode, start = loopStart
    if(data->mode == SINGLE) loopMarker[I_LSTART] = loopMarker[I_START];

    //Clear previous marker position
    TFT_drawFastVLine(prevLoopMarker[I_START], 0, *height, TFT_BLACK);

    //Print new marker position
    TFT_drawFastVLine(loopMarker[I_START], 0, *height, TFT_GREEN);
}

void updateLoopStart(play_state_data_t* data, uint16_t* loopMarker, uint16_t* prevLoopMarker, const int* width, const int* height, const int menu_item){
    if(data->mode == SINGLE) return;
    
    loopMarker[I_LSTART] = menuTFTScaleToWidth(data->loop_start, 800, *width);

    //If marker overlapped reprint start marker
    if(loopMarker[I_LSTART] == loopMarker[I_START] + 1 || loopMarker[I_LSTART] == loopMarker[I_START] - 1){
        TFT_drawFastVLine(loopMarker[I_START], 0, *height, TFT_GREEN);
    }
    //Clear previous marker position
    TFT_drawFastVLine(prevLoopMarker[I_LSTART], 0, *height, TFT_BLACK);

    //Print new marker position
    TFT_drawFastVLine(loopMarker[I_LSTART], 0, *height, TFT_RED);

}

void updateLoopEnd(play_state_data_t* data, uint16_t* loopMarker, uint16_t* prevLoopMarker, const int* width, const int* height){
    loopMarker[I_LEND] = menuTFTScaleToWidth(data->loop_end, 800, *width);
    if(loopMarker[I_LEND] == prevLoopMarker[I_LEND]) return;

    //Clear previous marker position
    TFT_drawFastVLine(prevLoopMarker[I_LEND], 0, *height, TFT_BLACK);

    //Print new marker position
    TFT_drawFastVLine(loopMarker[I_LEND], 0, *height, TFT_RED);

}


void resetPrevLoopMarker(uint16_t* prevLoopMarker){
    for(int i = 0; i < 3; i++)
    {
        //reset previous loopmarker to value outside of possible x values 
        prevLoopMarker[i] = 301;
    }
    
}

void updatePrevLoopMarker(uint16_t* loopMarker, uint16_t* prevLoopMarker){
    for(int i = 0; i < 3; i++)
    {
        prevLoopMarker[i] = loopMarker[i];
    }
}

void clearLStartMarker(uint16_t* loopMarker, const int* width, const int* height){
    TFT_drawFastVLine(loopMarker[I_LSTART], 0, *height, TFT_BLACK);
    TFT_drawFastVLine(loopMarker[I_START], 0, *height, TFT_GREEN);
}