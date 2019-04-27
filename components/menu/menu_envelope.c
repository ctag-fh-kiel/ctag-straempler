#include "menu_envelope.h"

void updateAttackPoint(vec2d_t* curve, vec2d_t* prevCurve, adsr_data_t* data, vec2d_t* startPoint, const int* width, const int* height){
    curve[I_ATTACK].x = 2 + menuTFTScaleToWidth(data->attack, 10000, *width - 2);
    curve[I_ATTACK].y = 2;

    //Return if x value has not changed
    if(curve[I_ATTACK].x == prevCurve[I_ATTACK].x) return;

    TFT_drawLine(startPoint->x, startPoint->y, prevCurve[I_ATTACK].x, prevCurve[I_ATTACK].y, TFT_BLACK);
    TFT_drawLine(startPoint->x, startPoint->y, curve[I_ATTACK].x, curve[I_ATTACK].y, TFT_WHITE);
}


void updateDecayPoint(vec2d_t* curve, vec2d_t* prevCurve, adsr_data_t* data, vec2d_t* startPoint, const int* width, const int* height){
    curve[I_DECAY].x = curve[I_ATTACK].x + menuTFTScaleToWidth(data->decay, 10000, *width - 2);
    curve[I_DECAY].y = 2 + abs(data->sustain - 100);

    if(curve[I_DECAY].x == prevCurve[I_DECAY].x && curve[I_DECAY].y == prevCurve[I_DECAY].y) return;
    TFT_drawLine(prevCurve[I_ATTACK].x, prevCurve[I_ATTACK].y, prevCurve[I_DECAY].x, prevCurve[I_DECAY].y, TFT_BLACK);
    TFT_drawLine(curve[I_ATTACK].x, curve[I_ATTACK].y, curve[I_DECAY].x, curve[I_DECAY].y, TFT_WHITE);

    //If attack and decay lines overlap, redraw attack line
    if(((curve[I_DECAY].x - curve[I_ATTACK].x) <= 10) && curve[I_ATTACK].x <= 30){
        TFT_drawLine(startPoint->x, startPoint->y, curve[I_ATTACK].x, curve[I_ATTACK].y, TFT_WHITE);        
    }

}


void updateSustainPoint(vec2d_t* curve, vec2d_t* prevCurve, adsr_data_t* data, const int* width, const int* height){
    if(curve[I_DECAY].x >= 190){
        curve[I_SUSTAIN].x = curve[I_DECAY].x;
    }else{
        curve[I_SUSTAIN].x = 190;
    }
    curve[I_SUSTAIN].y = 2 + abs(data->sustain - 100);

    if(curve[I_SUSTAIN].y == prevCurve[I_SUSTAIN].y && curve[I_DECAY].x == prevCurve[I_DECAY].x) return;
    if(curve[I_DECAY].x <= 190){
        TFT_drawFastHLine(prevCurve[I_DECAY].x, prevCurve[I_SUSTAIN].y, prevCurve[I_SUSTAIN].x - prevCurve[I_DECAY].x, TFT_BLACK);
        TFT_drawFastHLine(curve[I_DECAY].x, curve[I_SUSTAIN].y, curve[I_SUSTAIN].x - curve[I_DECAY].x, TFT_WHITE);
    }
}


void updateReleasePoint(vec2d_t* curve, vec2d_t* prevCurve, adsr_data_t* data, const int* width, const int* height){
    curve[I_RELEASE].x = curve[I_SUSTAIN].x + menuTFTScaleToWidth(data->release, 10000, *width - 2);
    curve[I_RELEASE].y = 102;

    if(curve[I_RELEASE].x == prevCurve[I_RELEASE].x && curve[I_SUSTAIN].y == prevCurve[I_SUSTAIN].y) return;
    if(curve[I_SUSTAIN].x < *width + 8){
        TFT_drawLine(prevCurve[I_SUSTAIN].x, prevCurve[I_SUSTAIN].y, prevCurve[I_RELEASE].x, prevCurve[I_RELEASE].y, TFT_BLACK);
        TFT_drawLine(curve[I_SUSTAIN].x, curve[I_SUSTAIN].y, curve[I_RELEASE].x, curve[I_RELEASE].y, TFT_WHITE);        
    }
}

void updatePrevCurve(vec2d_t* curve, vec2d_t* prevCurve){
    for(int i = 0; i < 4; i++)
    {
        prevCurve[i].x = curve[i].x;
        prevCurve[i].y = curve[i].y;
    }
}

void resetPrevCurve(vec2d_t* prevCurve){
    for(int i = 0; i < 4; i++)
    {
        prevCurve[i].x = 2;
        prevCurve[i].y = 2;
    }
}

void printIndicatorPoints(vec2d_t* curve, vec2d_t* startPoint, int radius, color_t color){
    TFT_fillCircle(startPoint->x, startPoint->y, radius, color);

    for(int i = 0; i < 4; i++)
    {
        TFT_fillCircle(curve[i].x, curve[i].y, radius, color);
    }
    
}