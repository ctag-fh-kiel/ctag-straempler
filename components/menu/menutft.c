#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <unistd.h>
#include "esp_log.h"
#include "tft.h"
#include "string_tools.h"
#include "menutft.h"
#include "menu_items.h"
#include "fixed.h"
#include "audio_luts.h"
#include "audio_luts.h"
#include "list.h"
#include "freertos/timers.h"




//definitions for element highlighting and selection
int _cur_row = 0, _cur_el = -1;
list_t *_bbox_list = NULL;




//definitions for general menu elements
static uint16_t loopMarker[3]; 
static uint16_t prevLoopMarker[3];

static vec2d_t adsrStartPoint;
static vec2d_t curAdsrPoints[4];
static vec2d_t prevAdsrPoints[4];





//Menu printing functions
//-------------------------------------------------------------------------------------------------------
void menuTFTPrintMenu(const char** items, const int* n_items){
    int y = 0, x = 4;
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    
    _bg = TFT_BLACK;
    _fg = TFT_LIGHTGREY;
    TFT_fillWindow(_bg);
    int i;
    _cur_row = 0;
    for(i = 0; i < *n_items; i++){
        TFT_print((char *)items[i], x, 3 + (TFT_getfontheight() + 3) *_cur_row);
        _cur_row++;
    }
}

void menuTFTPrintMenuH(const char** items, const int* n_items){
    int x = 4, w = 0;
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    TFT_fillWindow(TFT_BLACK);
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 0;
    for(int i=0; i < *n_items; i++){
        w = TFT_getStringWidth((char*)items[i]);
        TFT_print((char *)items[i], x, 4);
        x += w + 8;
    }
}

void menuTFTPrintMenuHSpaced(const char** items, const int* n_items){
    int x1 = 0, x2 = 4, w = 0, y = 4;
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    TFT_fillWindow(TFT_BLACK);
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 0;
    int x_incr = _width / *n_items;
    for(int i=0; i < *n_items; i++){
        w = TFT_getStringWidth((char *)items[i]);
        int x_offset = (x_incr/2) -  w/2;
        TFT_print((char *)items[i], x1 + x_offset, y);
        x1 += x_incr;
    }    
}

void menuTFTPrintMainMenus(){
    int x = 8, w = 0;
    _fg = TFT_WHITE;
	_bg = (color_t){ 64, 64, 64 };
    TFT_setFont(DEFAULT_FONT, NULL);
	TFT_fillRect(0, 0, _width-1, TFT_getfontheight()+8, _bg);
    TFT_X = 0;
    for(int i=0; i < n_main_menus; i++){
        w = TFT_getStringWidth((char*)main_menus[i]);
        TFT_print((char *)main_menus[i], x, 4);
        x += w + 8;
    }
}

void menuTFTPrintAbout(){
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    TFT_fillWindow(TFT_BLACK);
    _fg = TFT_WHITE;
	_bg = TFT_BLACK;
    TFT_print("Freesound Sampler Version:", 4, 4);
    TFT_print(VERSION, 4, TFT_Y);
    TFT_Y += TFT_getfontheight();
    TFT_print("Created by: ", 4, TFT_Y);
    TFT_print("Niklas Wantrupp", 4, TFT_Y);
    TFT_print("Phillip Lamp", 4, TFT_Y);
    TFT_print("Robert Manzke", 4, TFT_Y);
}

void menuTFTPrintVoiceMenu(){
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    int x = 4;
    _bg = TFT_BLACK;
    TFT_fillWindow(_bg);
    int i;
    _cur_row = 0;
    _fg = TFT_LIGHTGREY;
    for(i=0; i<n_voice_menus; i++){
        if(i == 9){
            TFT_drawFastHLine(2, ((TFT_getfontheight() + 3) *_cur_row) + (TFT_getfontheight() / 2) + 1, _width - 2, TFT_WHITE);
            _cur_row++;   
        }
        TFT_print((char *)voice_menus[i], x, 3 + (TFT_getfontheight() + 3) *_cur_row);
        _cur_row++;
    }
}

void menuTFTPrintADSRMenu(){
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    
    _bg = TFT_BLACK;
    TFT_fillWindow(_bg);
    int i, x = 4;
    _cur_row = 0;
    _fg = TFT_LIGHTGREY;
    for(i=0; i<n_adsr_menus; i++){
        TFT_print((char *)adsr_menus[i], x, 3 + (TFT_getfontheight() + 3) *_cur_row);
        _cur_row++;
    }
    _cur_row += 3;
    TFT_drawRect(16, TFT_getfontheight() * 7, _width - 32, (TFT_getfontheight() * 8) + 8 , TFT_WHITE);

}

void menuTFTPrintPlaymodeMenu(){
    TFT_setclipwin(0,TFT_getfontheight()+9, _width, _height);
    
    _bg = TFT_BLACK;
    TFT_fillWindow(_bg);
    int i, x = 4;
    _cur_row = 0;
    _fg = TFT_LIGHTGREY;
    for(i=0; i<n_playmode_menus; i++){
        TFT_print((char *)playmode_menus[i], x, 3 + (TFT_getfontheight() + 3) *_cur_row);
        _cur_row++;
    }

    TFT_drawRect(4, (TFT_getfontheight() * 7) + 9, _width - 8, TFT_getfontheight() * 4 , TFT_WHITE);
}

void menuTFTPrintMatrixMenu(){
    TFT_setclipwin(0,TFT_getfontheight()+8, _width-1, _height);
    TFT_saveClipWin();

    
    _bg = TFT_BLACK;
    TFT_fillWindow(_bg);
    int x_inc = _width/3;
    int w = 0;
    int x = 0;
    char tmp[15];
    _cur_row = 1;
    _fg = TFT_LIGHTGREY;

    //Draw top row
    for(int i = 0; i < n_cv_matrix_items; i++){
        w = TFT_getStringWidth((char*)cv_matrix_items[i]);
        TFT_setclipwin(x,TFT_getfontheight()+9, x + x_inc, _height);
        //TFT_fillWindow(TFT_RED);
        TFT_print((char *)cv_matrix_items[i], CENTER, 4);
        x+= x_inc;
        TFT_resetclipwin();
    }

    TFT_restoreClipWin();
    TFT_drawFastHLine(0, TFT_getfontheight()+8, _width, TFT_WHITE);
    TFT_drawFastVLine(x_inc, 1,((TFT_getfontheight() + 8) * 9), TFT_WHITE);
    TFT_drawFastVLine(x_inc * 2, 1, ((TFT_getfontheight() + 8) * 9), TFT_WHITE);

    //Draw horizontal lines
    TFT_setclipwin(0, TFT_getfontheight()* 2 + 18, _width - 1, _height);
    int y_l = TFT_getfontheight() + 6;
    for(int k = 0; k < 8; k++)
    {
        TFT_drawFastHLine(0, y_l, _width, TFT_WHITE);
        y_l += TFT_getfontheight() + 8;
    }
    
    TFT_restoreClipWin();
    TFT_setclipwin(0, TFT_getfontheight()* 2 + 18, x_inc, _height);
    int y = 4;
    
    for( int j = 1; j < 9; j++)
    {
        sprintf(tmp, "CV %d", j);
        TFT_print(tmp, CENTER, y);
        y += TFT_getfontheight() + 8;
    }
    

}

void menuTFTPrintSlotMenu(const cJSON *slotData, int activeSlot){
    if(_bbox_list != NULL) list_free(_bbox_list);
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    _bg = TFT_BLACK;
    TFT_fillWindow(_bg);
    char s[64];
    int i, x = 4;
    _cur_el = activeSlot - 1;
    _cur_row = 0;
    _fg = TFT_LIGHTGREY;
    _bbox_list = list_create();
    cJSON *slots = cJSON_GetObjectItem(slotData, "slots");
    for(i=0; i<2; i++){
        cJSON *slotObj = cJSON_GetArrayItem(slots, i);
        char *slot;
        snprintf(s, 64, "slot%d", i+1);
        slot = cJSON_GetObjectItemCaseSensitive(slotObj, "name")->valuestring;
        snprintf(s, 64, "Slot #%d: %s", i + 1, slot);
        bbox_t *bbox = (bbox_t*)calloc(sizeof(bbox_t), 1);
        bbox->x = 2; bbox->y = (TFT_getfontheight()+3)*_cur_row;
        bbox->w = TFT_getStringWidth(s) + 4; bbox->h = TFT_getfontheight() + 4;
        list_add_element(_bbox_list, (void*)bbox);
        TFT_print(s, x, 3 + (TFT_getfontheight() + 3) *_cur_row);
        _cur_row++;
    }
    menuTFTHighlightNextEl();
}

void menuTFTPrintSettings(const cJSON *data){
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height); 
    const char* hidden_items[] = {"passwd", "apikey"};
    const int n_hidden_items = sizeof(hidden_items)/sizeof(int);
    _bg = TFT_BLACK;
    _fg = TFT_WHITE;
    char buf[32];
    int x = 4, cnt = 0;
    cJSON *val = NULL;
    if(data != NULL){
        _cur_row = 0;
        val = cJSON_GetObjectItemCaseSensitive(data, "ssid");
        TFT_print(val->valuestring, x + _width/2, 3 + (TFT_getfontheight() + 3) *_cur_row);
        _cur_row++;
        for(int i = 0; i < n_hidden_items; i++)
        {
            val = cJSON_GetObjectItemCaseSensitive(data, hidden_items[i]);
            strncpy(buf, val->valuestring, 32);
            hideString(buf, 32, 22);
            TFT_print(buf, x + _width/2, 3 + (TFT_getfontheight() + 3) *_cur_row);
            _cur_row++;
        }
    }
}

void menuTFTPrintBrowseTextMenu(){
    _bg = TFT_BLACK;
    TFT_print("Soon", _width/2, _height/2);
}

void menuTFTPrintLoadingTagMenu(){
    int x = 0, w = 0;
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    TFT_fillWindow(TFT_BLACK);
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 3;
    TFT_print("Acquiring tags from freesound.org, wait...", x, 4);
}

void menuTFTPrintUserFileMenu(){
    int x = 0, w = 0;
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    TFT_fillWindow(TFT_BLACK);
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 3;
    TFT_print("Go http://ctag-modular (long press rtrn)", x, 4);
}

void menuTFTPrintSelectIDMenu(){
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    TFT_fillWindow(TFT_BLACK);
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 3;
    TFT_print("Enter sound ID:", TFT_X, 4);
}


void menuTFTPrintPresetLayout(){
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    _bg = TFT_BLACK;
    
    int y = TFT_getfontheight() + 8;
    TFT_drawFastHLine(0, (2*y) + 2, _width, TFT_WHITE);
}

void menuTFTPrintInputMenu(char* title){
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    TFT_fillWindow(TFT_BLACK);
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 3;
    TFT_print(title, TFT_X, 4);
}

void menuTFTPrintPresetList(cJSON* bank, print_ids_t action){
    TFT_setclipwin(0, 3* TFT_getfontheight() + 32, _width-1, _height);
    if(action == PRINT_CLEAR) TFT_fillWindow(TFT_BLACK);
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 3;
    int x = 4;
    int x_incr = _width / 3;
    int y_start = 0;
    int y = 8 + TFT_getfontheight();

    cJSON* element = NULL;
    cJSON* val;
    int i = 0;

    char bn[32];

    if(bank != NULL){
        cJSON_ArrayForEach(element, bank){
            if(cJSON_IsObject(element)){
                if(i > cJSON_GetArraySize(bank)){
                    return;
                }

                val = cJSON_GetObjectItemCaseSensitive(element, "presetName");
                sprintf(bn, val->valuestring);
                if(i < 10) TFT_print(bn, x, y_start + 3 + (TFT_getfontheight() + 3) * i);
                if(i >= 10 && i < 20) TFT_print(bn, x + x_incr, y_start + 3 + (TFT_getfontheight() + 3) * (i - 10));
                if(i >= 20 && i < 30) TFT_print(bn, x + 2*x_incr, y_start + 3 + (TFT_getfontheight() + 3) * (i - 20));

                i++;
            }
        }
    } 


}

void menuTFTPrintBankList(list_t* list, print_ids_t action){
    TFT_setclipwin(0, 3* TFT_getfontheight() + 32, _width-1, _height);
    if(action == PRINT_CLEAR) TFT_fillWindow(TFT_BLACK);
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 3;
    int x = 4;
    int x_incr = _width / 3;
    int y_start = 0;
    int y = 8 + TFT_getfontheight();

    char* bn;
    
    if(list != NULL){
        if(list->count != 0){
            for(int i = 0; i < list->count; i++)
            {
                bn = (char*)list_get_item(list, i)->value;
                if(i < 10) TFT_print(bn, x, y_start + 3 + (TFT_getfontheight() + 3) * i);
                if(i >= 10 && i < 20) TFT_print(bn, x + x_incr, y_start + 3 + (TFT_getfontheight() + 3) * (i - 10));
                if(i >= 20 && i < 30) TFT_print(bn, x + 2*x_incr, y_start + 3 + (TFT_getfontheight() + 3) * (i - 20));
                
            }         
        }
    }
}
//-------------------------------------------------------------------------------------------------------




//Selecting menu item functions
//-------------------------------------------------------------------------------------------------------
void menuTFTSelectMenuItem(int* activeSlot, int selected, const char** items, const int* n_items){
    int h = 0, y = 0;
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 0;
    h = TFT_getfontheight();
    for(int i = 0;i < *n_items; i++){
        _bg = TFT_BLACK;
        if(i== *activeSlot){
            if(selected == 1){
                _bg = TFT_RED;
            }else{
                _bg = TFT_CYAN;
            }
        }
        TFT_drawRect(2, y, TFT_getStringWidth((char*)items[i]) + 4, h + 3 , _bg);
        y += h + 3;
    }
}

void menuTFTSelectMenuItemH(int* activeSlot, int selected, const char** items, const int* n_items){
    int x = 0, w = 0;
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 0;
    for(int i=0;i<*n_items;i++){
        _bg = TFT_BLACK;
        if(i== *activeSlot){
            if(selected == 1){
                _bg = TFT_RED;
            }else{
                _bg = TFT_CYAN;
            }
        }
        w = TFT_getStringWidth((char*)items[i]);
        TFT_drawRect(x, 0, w+8, TFT_getfontheight()+8, _bg);
        x += w + 8;
    }
}

void menuTFTSelectMenuItemHSpaced(int* activeSlot, int selected, const char** items, const int* n_items){
    int x1 = 0, x2 = 0, w = 0, x_incr = _width / *n_items, x_offset = 0;
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 0;
    //ESP_LOGI("UI", "Slot: %d, Selected: %d", *activeSlot, selected);
    for(int i=0;i<*n_items;i++){
        _bg = TFT_BLACK;
        if(i== *activeSlot){
            if(selected == 1){
                _bg = TFT_RED;
            }else{
                _bg = TFT_CYAN;
            }
        }
        w = TFT_getStringWidth((char*)items[i]);
        x_offset = (x_incr/2) -  (w/2) - 4;
        TFT_drawRect(x1 + x_offset, 0, w+8, TFT_getfontheight()+8, _bg);
        x1 += x_incr;   
    }  
}

void menuTFTSelectMainMenu(int active, int select){
    int x = 4, w = 0;
    TFT_resetclipwin();
    for(int i=0;i<n_main_menus;i++){
        _bg = (color_t){ 64, 64, 64 };
        if(i==active){
            if(select == 1){
                _bg = TFT_RED;
            }else{
                _bg = TFT_CYAN;
            }
        }
        w = TFT_getStringWidth((char*)main_menus[i]);
        TFT_drawRect(x, 0, w+8, TFT_getfontheight()+8, _bg);
        x += w + 8;
    }
}

void menuTFTSelectVoiceMenu(int active, int select){
    int h = 0, y = 0;
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 0;
    h = TFT_getfontheight();
    for(int i=0;i<n_voice_menus;i++){
        _bg = TFT_BLACK;
        if(i==active){
            if(select == 1){
                _bg = TFT_RED;
            }else{
                _bg = TFT_CYAN;
            }
        }
        TFT_drawRect(2, y, TFT_getStringWidth((char*)voice_menus[i]) + 4, h + 3 , _bg);
        y += h + 3;
        if(i == 8) y += h + 3;
    }
}

void menuTFTSelectMatrixItem(int active, int select, int column){
    int h = TFT_getfontheight(), y = 0;
    int x_incr = _width/3;
    int x_mod = 3;
    int x_offset = 0;
    TFT_setclipwin(0, TFT_getfontheight()* 2 + 18, _width - 1, _height);    
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 0;
    
    for(int k = 0; k < 3; k++)
    {
        x_offset = x_incr * k;
        if(k == 2) x_mod = 2;
        y = 0;
        for(int i=0;i<8;i++){
            _bg = TFT_BLACK;
            if(i==active && k == column){
                if(select == 1){
                    _bg = TFT_RED;
                }else{
                    _bg = TFT_CYAN;
                }  
            }
            TFT_drawRect(x_offset + 2, y, x_incr - x_mod, h + 5 , _bg);
            y += TFT_getfontheight() + 8;
        }   

    }
    
}

void menuTFTSelectPreset(int* activeSlot, cJSON* rootArray, int selected){

    int h = 0, y = 0;
    TFT_setclipwin(0, 3* TFT_getfontheight() + 32, _width-1, _height);
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 0;
    int x_incr = _width/3;
    h = TFT_getfontheight();
    char* bn;

    cJSON* element = NULL;

    //ESP_LOGI("menuTFTSelectPreset", "cJSON Array Size: %d", cJSON_GetArraySize(rootArray));
    int i = 0;
    if(rootArray != NULL){
        cJSON_ArrayForEach(element, rootArray){
            _bg = TFT_BLACK;
            if(i==*activeSlot){
                //ESP_LOGI("menuTFTSelectPresetBank", "i == activeSlot: %d | Value: %s", i, (char*)list_get_item(list, i)->value);
                if(selected) _bg = TFT_CYAN;
            }
            if(cJSON_IsObject(element) && element != NULL){
                bn = cJSON_GetObjectItemCaseSensitive(element, "presetName")->valuestring;
                //if(i == *activeSlot)ESP_LOGI("menuTFT", "Presetname: %s", bn);
                if(i == 0 || i == 10 || i == 20 || i == 30) y = 0;
                
                if(i < 10) TFT_drawRect(2, y, TFT_getStringWidth(bn) + 4, h + 3 , _bg);
                if(i >= 10 && i < 20) TFT_drawRect(2 + x_incr, y, TFT_getStringWidth(bn) + 4, h + 3 , _bg);
                if(i >= 20 && i < 30) TFT_drawRect(2 + 2*x_incr, y , TFT_getStringWidth(bn) + 4, h + 3 , _bg);
                y += h + 3;
            }
            i++;
        }
    }
}

void menuTFTSelectPresetBank(int* activeSlot, list_t* list, int selected){
    int h = 0, y = 0;
    TFT_setclipwin(0, 3* TFT_getfontheight() + 32, _width-1, _height);
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 0;
    int x_incr = _width/3;
    h = TFT_getfontheight();
    char* bn;
    for(int i=0;i<list->count;i++){
        _bg = TFT_BLACK;
        if(i==*activeSlot){
            //ESP_LOGI("menuTFTSelectPresetBank", "i == activeSlot: %d | Value: %s", i, (char*)list_get_item(list, i)->value);
            if(selected) _bg = TFT_CYAN;
        }

        bn = (char*)list_get_item(list, i)->value;
        if(i == 0 || i == 10 || i == 20 || i == 30) y = 0;
        
        if(i < 10) TFT_drawRect(2, y, TFT_getStringWidth(bn) + 4, h + 3 , _bg);
        if(i >= 10 && i < 20) TFT_drawRect(2 + x_incr, y, TFT_getStringWidth(bn) + 4, h + 3 , _bg);
        if(i >= 20 && i < 30) TFT_drawRect(2 + 2*x_incr, y , TFT_getStringWidth(bn) + 4, h + 3 , _bg);
        y += h + 3;
    }  
}
//-------------------------------------------------------------------------------------------------------




//Print parameter values
//-------------------------------------------------------------------------------------------------------
void menuTFTPrintVoiceValues(param_data_t* data, int r){
    int y = 0;
    char tmp[10];
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
   
    _bg = TFT_BLACK;
    _fg = TFT_LIGHTGREY;
    
    switch (r)
    {
        case SID_TRIG_TYPE:
            sprintf(tmp, trig_items[data->trig_mode_latch]);
            menuTFTFlush(0, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 0); 
            break;
        case SID_VOLUME:
            sprintf(tmp,"%d %%", data->volume);
            menuTFTFlushValue(data->volume, 1, default_conditions, &default_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 1);  
            break;
        case SID_PAN:
            if((data->pan >> SCALE_8) == 0){
                sprintf(tmp, "%s", "C");
            }else{
                (data->pan >> SCALE_8) < 0 ? sprintf(tmp, "%dL", (data->pan >> SCALE_8) * -1) : sprintf(tmp, "%dR", (data->pan >> SCALE_8));
            }
            menuTFTFlushValue(abs(data->pan >> SCALE_8), 2, negpos_conditions, &negpos_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 2);
            break;
        case SID_PITCH:
            sprintf(tmp,"%d st",(data->pitch - 12));
            menuTFTFlushValue(abs(data->pitch - 12), 3, negpos_conditions, &negpos_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 3);
            break;
        case SID_PITCH_CV_ACTIVE:
            sprintf(tmp, toggle_items[data->pitch_cv_active]);
            menuTFTFlush(4, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 4); 
            break;
        case SID_PBSPEED: ;
            int16_t scaledPbspeed = menuTFTScaleToRange(data->playback_speed, 16384, 100);
            if(scaledPbspeed == 0){
                sprintf(tmp,"%d %%", scaledPbspeed);
            }else{
                scaledPbspeed < 0 ? sprintf(tmp,"-%d %%", scaledPbspeed * -1) : sprintf(tmp,"+%d %%", scaledPbspeed);
            }
            menuTFTFlushValue(abs(scaledPbspeed),5,  negpos_conditions, &negpos_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 5);  
            break;
        case SID_DIST_ACTIVE:
            sprintf(tmp, toggle_items[data->dist_active]);
            menuTFTFlush(6, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 6); 
            break;
        case SID_DIST:
            sprintf(tmp,"%.2f", dist_q_lut[data->dist_drive]);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 7); 
            break;
        case SID_DELAY_SEND:
            sprintf(tmp,"%d %%", data->delay_send);
            menuTFTFlushValue(data->delay_send, 8, default_conditions, &default_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 8);
            break;
        case PRINT_ALL: ;
        //print all

            sprintf(tmp, trig_items[data->trig_mode_latch]);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y); 
            y++;

            sprintf(tmp,"%d %%", data->volume);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);  
            y++;

            if(data->pan == 0){
                sprintf(tmp, "%s", "C");
            }else{
                data->pan < 0 ? sprintf(tmp, "%2dL", (data->pan >> SCALE_8) * -1) : sprintf(tmp, "%2dR", (data->pan >> SCALE_8));
            }
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);  
            y++;

            sprintf(tmp,"%d st",(data->pitch - 12));
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            sprintf(tmp, toggle_items[data->pitch_cv_active]);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y); 
            y++;

            int16_t pbspeed = menuTFTScaleToRange(data->playback_speed, 16384, 100);
            if(pbspeed == 0){
                sprintf(tmp,"%d %%", pbspeed);
            }else{
                pbspeed < 0 ? sprintf(tmp,"-%d %%", pbspeed * -1) : sprintf(tmp,"+%d %%", pbspeed);
            }
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);  
            y++;
            
            sprintf(tmp, toggle_items[data->dist_active]);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y); 
            y++;

            sprintf(tmp,"%.2f", dist_q_lut[data->dist_drive]);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y); 
            y++;
            
            sprintf(tmp,"%d %%", data->delay_send);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            break;    
        default:
            break;
    }
}

void menuTFTPrintADSRValues(adsr_data_t* data, int r){
    int y = 0;
    char tmp[10];
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    
    _bg = TFT_BLACK;
    _fg = TFT_LIGHTGREY;
    switch (r)
    {
        case SID_ATTACK:
            sprintf(tmp,"%d ms", data->attack);
            menuTFTFlushValue(data->attack, 0, adsr_conditions, &adsr_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 0);
            break;
        case SID_DECAY:
            sprintf(tmp,"%d ms", data->decay);
            menuTFTFlushValue(data->decay, 1, adsr_conditions, &adsr_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 1);
            break;
        case SID_SUSTAIN:
            sprintf(tmp,"%d", data->sustain);
            menuTFTFlushValue(data->sustain, 2, default_conditions, &default_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 2);
            break;
        case SID_RELEASE:
            sprintf(tmp,"%d ms", data->release);      
            menuTFTFlushValue(data->release, 3, adsr_conditions, &adsr_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 3);
            break;
        case PRINT_ALL:
            sprintf(tmp,"%d ms", data->attack);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            sprintf(tmp,"%d ms", data->decay);          
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            sprintf(tmp,"%d", data->sustain);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            sprintf(tmp,"%d ms", data->release);               
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);            
        default:
            break;
    }    
    
}

void menuTFTPrintFilterValues(filter_data_t* data, int r){
    int y = 0;
    char tmp[10];
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    
    _bg = TFT_BLACK;
    _fg = TFT_LIGHTGREY;

    switch (r)
    {
        case SID_FILTER_ACTIVE:
        
            sprintf(tmp, toggle_items[data->is_active]);
            menuTFTFlush(0, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 0);
            break;
        case SID_BASE:
            sprintf(tmp,"%.2f Hz", poti_vals[data->base]);
            menuTFTFlushValue(data->base, 1, filter_conditions, &filter_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 1);
            break;
        case SID_WIDTH:
            sprintf(tmp,"%.2f Hz", poti_vals[data->width]);
            menuTFTFlushValue(data->width, 2, filter_conditions, &filter_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 2);
            break;
        case SID_Q:
            sprintf(tmp,"%.2f", dist_q_lut[data->q]);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 3);
            break;

        case PRINT_ALL:

            sprintf(tmp, toggle_items[data->is_active]);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y); 
            y++;

            sprintf(tmp,"%.2f Hz", poti_vals[data->base]);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            sprintf(tmp,"%.2f Hz", poti_vals[data->width]);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            sprintf(tmp,"%.2f", dist_q_lut[data->q]);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;
   
        default:
            break;
    }    
}

void menuTFTPrintPlaymodeValues(play_state_data_t *data, int r){
    int y = 0;
    char tmp[10];
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    
    _bg = TFT_BLACK;
    _fg = TFT_LIGHTGREY;

    uint16_t scaledStart = (uint16_t) menuTFTScaleToRange(data->start, 800, 100);
    uint16_t scaledLStart = (uint16_t) menuTFTScaleToRange(data->loop_start, 800, 100);
    uint16_t scaledLEnd = (uint16_t) menuTFTScaleToRange(data->loop_end, 800,100);
    uint16_t scaledLPos = (uint16_t) menuTFTScaleToRange(data->loop_position, 800, 100);

    switch (r)
    {
        case SID_MODE:
            sprintf(tmp, playmode_modes[data->mode]);
            menuTFTFlush(0, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 0);

            if(data->mode == SINGLE){
                sprintf(tmp,"%d %%", scaledLStart);
                menuTFTFlush(2, &_bg);
                TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 2);
                
                sprintf(tmp,"%d %%", scaledLPos);
                menuTFTFlush(4, &_bg);
                TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 4); 
            }
            break;
        case SID_START:

            sprintf(tmp,"%d %%", scaledStart);
            menuTFTFlushValue(data->start, 1, playmode_conditions, &playmode_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 1);

            if(data->mode == SINGLE){
                sprintf(tmp,"%d %%", scaledLStart);
                menuTFTFlushValue(data->loop_start, 2, playmode_conditions, &playmode_cond_size, &_bg);
                TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 2);

                sprintf(tmp,"%d %%", scaledLPos);
                menuTFTFlushValue(data->loop_position, 4, playmode_conditions, &playmode_cond_size, &_bg);
                TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 4); 
            }
            break;
        case SID_LSTART:
            sprintf(tmp,"%d %%", scaledLStart);
            menuTFTFlushValue(data->loop_start, 2, playmode_conditions, &playmode_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 2);
            
            sprintf(tmp,"%d %%", scaledLPos);
            menuTFTFlushValue(data->loop_position, 4, playmode_conditions, &playmode_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 4); 

            if(data->mode == SINGLE){
                sprintf(tmp,"%d %%", scaledStart);
                menuTFTFlushValue(data->start, 1, playmode_conditions, &playmode_cond_size, &_bg);
                TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 1);
            } 
            break;
        case SID_LEND:
            sprintf(tmp,"%d %%", scaledLEnd);
            menuTFTFlushValue(data->loop_end, 3, playmode_conditions, &playmode_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 3);
            break;
        
        case SID_LPOSITION: ;
            //Reprint LoopStart/LoopEnd/LoopPosition
            sprintf(tmp,"%d %%", scaledLStart);
            menuTFTFlushValue(data->loop_start, 2, playmode_conditions, &playmode_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 2);

            sprintf(tmp,"%d %%", scaledLEnd);
            menuTFTFlushValue(data->loop_end, 3, playmode_conditions, &playmode_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 3);
            
            sprintf(tmp,"%d %%", scaledLPos);
            menuTFTFlushValue(data->loop_position, 4, playmode_conditions, &playmode_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 4); 

            if(data->mode == SINGLE){
                sprintf(tmp,"%d %%", scaledStart);
                menuTFTFlushValue(data->start, 1, playmode_conditions, &playmode_cond_size, &_bg);
                TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 1);
            }   
            break;

        case PRINT_ALL:

            sprintf(tmp, playmode_modes[data->mode]);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y); 
            y++;

            sprintf(tmp,"%d %%", scaledStart);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            sprintf(tmp,"%d %%", scaledLStart);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            sprintf(tmp,"%d %%", scaledLEnd);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;
   
            sprintf(tmp,"%d %%", scaledLPos);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

        default:
            break;
    }    
}

void menuTFTPrintMatrixAmount(matrix_ui_row_t* matrix, int r){
    
    int x_offset = _width/3;    
    int x_incr = _width/3;   
    TFT_setclipwin(x_offset ,TFT_getfontheight()* 2 +18, x_offset * 2, _height);
    int y = 4;
    int amt = 0;
    char tmp[10];
    static matrix_param_t prevParam [8];
    _fg = TFT_WHITE;
    
    _bg = TFT_BLACK;

    switch(r){
        case PRINT_ALL:
            for(int i = 0; i < 8; i++)
            {
                if(matrix[i].dst == MTX_NONE){
                    sprintf(tmp, "---");
                }else{
                    sprintf(tmp, "%d %%", matrix[i].amt);
                }
                
                TFT_print(tmp, CENTER, y);
                y += TFT_getfontheight() + 8;
            }
            break;
        default: ;
            uint8_t* amt = &(matrix[r].amt);
            y += (TFT_getfontheight() + 8) * r;

            if(matrix[r].dst == MTX_NONE){
                sprintf(tmp, "---");
            }else{
                sprintf(tmp, "%d %%", *amt);
            }
            
            if(*amt == 9 || *amt == 10 || *amt== 99 || *amt == 100 || prevParam[r] != matrix[r].dst){
                TFT_fillRect(4, y, x_incr - 6, TFT_getfontheight(), _bg);
            }
            
            TFT_print(tmp, CENTER, y);
            prevParam[r] = matrix[r].dst;
            break;
    }
}

void menuTFTPrintMatrixDestination(matrix_ui_row_t* matrix, int row){
    int x_offset = (_width/3) * 2;    
    int x_incr = _width/3;   
    TFT_setclipwin(x_offset ,TFT_getfontheight()* 2 +18, x_offset + x_incr, _height);
    int y = 4;
    char tmp[10];
    _fg = TFT_WHITE;
    
    _bg = TFT_BLACK;

    switch(row){
        case PRINT_ALL:
            for(int i = 0; i < 8; i++)
            {
                sprintf(tmp, matrix_parameter_items[matrix[i].dst]); 
                TFT_print(tmp, CENTER, y);
                y += TFT_getfontheight() + 8;
            }
            break;
        default:
            if(row != 0 && row != 1){
                y += (TFT_getfontheight() + 8) * row;
                sprintf(tmp, matrix_parameter_items[matrix[row].dst]);
                TFT_fillRect(4, y, x_incr - 6, TFT_getfontheight(), _bg);
                TFT_print(tmp, CENTER, y);
            }
            break;
    }
}

void menuTFTPrintDelayValues(delay_data_t* data, int r){
    int y = 0;
    char tmp[10];
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    
    _bg = TFT_BLACK;
    _fg = TFT_LIGHTGREY;

    int8_t pan = data->pan - 64;
    switch(r){
        case SID_DELAY_ACTIVE:
            sprintf(tmp, toggle_items[data->is_active]);
            menuTFTFlush(0, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 0);
            break;
        case SID_DELAY_MODE:
            sprintf(tmp, delay_mode_items[data->mode]);
            menuTFTFlush(1, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 1);
            break;
        case SID_DELAY_TIME:
            sprintf(tmp,"%d ms", data->time);
            menuTFTFlushValue(data->time, 2, default_conditions, &default_cond_size, &_bg);    
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 2);
            break;
        case SID_DELAY_PAN:
            if(pan == 0){
                sprintf(tmp, "%s", "C");
            }else{
                pan < 0 ? sprintf(tmp, "%dL", pan * -1) : sprintf(tmp, "%dR", pan);
            }
            menuTFTFlushValue(abs(pan), 3, negpos_conditions, &negpos_cond_size, &_bg);  
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 3);
            break;
        case SID_DELAY_FEEDBACK:
            sprintf(tmp,"%d %%", data->feedback);
            menuTFTFlushValue(data->feedback, 4, default_conditions, &default_cond_size, &_bg);  
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 4);
            break;
        case SID_DELAY_VOLUME:
            sprintf(tmp,"%d %%", data->volume);
            menuTFTFlushValue(data->volume, 5, default_conditions, &default_cond_size, &_bg);  
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 5);  
            break;
        case PRINT_ALL:
            sprintf(tmp, toggle_items[data->is_active]);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            sprintf(tmp, delay_mode_items[data->mode]);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            sprintf(tmp,"%d ms", data->time);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            if(pan == 0){
                sprintf(tmp, "%s", "C");
            }else{
                pan < 0 ? sprintf(tmp, "%dL", pan * -1) : sprintf(tmp, "%dR", pan);
            }
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            sprintf(tmp,"%d %%", data->feedback);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            sprintf(tmp,"%d %%", data->volume);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;
            break;
    }

}

void menuTFTPrintExtInValues(ext_in_data_t* data, int r){
    int y = 0;
    char tmp[10];
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    _bg = TFT_BLACK;
    _fg = TFT_LIGHTGREY;

    switch(r){
        case SID_EXTIN_ACTIVE:
            sprintf(tmp, toggle_items[data->is_active]);
            menuTFTFlush(0, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 0);
            break;
        case SID_EXTIN_VOLUME:
            sprintf(tmp,"%d %%", data->volume);
            menuTFTFlushValue(data->volume, 1, default_conditions, &default_cond_size, &_bg);  
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 1);  
            break;
        case SID_EXTIN_PAN:
            if((data->pan >> SCALE_8) == 0){
                sprintf(tmp, "%s", "C");
            }else{
                (data->pan >> SCALE_8) < 0 ? sprintf(tmp, "%dL", (data->pan >> SCALE_8) * -1) : sprintf(tmp, "%dR", (data->pan >> SCALE_8));
            }
            menuTFTFlushValue(abs(data->pan >> SCALE_8), 2, negpos_conditions, &negpos_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 2);
            break;
        case SID_EXTIN_DELAY_SEND:
            sprintf(tmp,"%d %%", data->delay_send);
            menuTFTFlushValue(data->delay_send, 3, default_conditions, &default_cond_size, &_bg);  
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * 3);
            break;
        case PRINT_ALL:
            sprintf(tmp, toggle_items[data->is_active]);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            sprintf(tmp,"%d %%   ", data->volume);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            if((data->pan >> SCALE_8) == 0){
                sprintf(tmp, "%s", "C");
            }else{
                (data->pan >> SCALE_8) < 0 ? sprintf(tmp, "%dL", (data->pan >> SCALE_8) * -1) : sprintf(tmp, "%dR", (data->pan >> SCALE_8));
            }
            menuTFTFlushValue(abs(data->pan >> SCALE_8), 2, negpos_conditions, &negpos_cond_size, &_bg);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;

            sprintf(tmp,"%d %%   ", data->delay_send);
            TFT_print(tmp, _width/2, 3 + (TFT_getfontheight() + 3) * y);
            y++;


            break;
    }

}
//-------------------------------------------------------------------------------------------------------




//Additional UI
//-------------------------------------------------------------------------------------------------------
void menuTFTPrintTime(int *shift){
    time_t time_now;
    struct tm* tm_info;
    char s[10];
    time(&time_now);
    tm_info = localtime(&time_now);
    tm_info->tm_hour += *shift;
    mktime(tm_info);
    //ESP_LOGI("MENU", "%02d:%02d:%02d", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    snprintf(s, 10, "%02d:%02d:%02d", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    TFT_saveClipWin();
    TFT_resetclipwin();
    _bg = (color_t){ 64, 64, 64 };
    _fg = TFT_WHITE;
    TFT_print(s, _width - TFT_getStringWidth(s) - 4, 4);
    TFT_restoreClipWin();
}

void menuTFTPrintTimezone(const char** items, const int* n_items, int *shift){
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height); 
    _bg = TFT_BLACK;
    _fg = TFT_WHITE;
    char buf[32];
    int x = 4, cnt = 0;
    _cur_row = 0;
    for(int i = 0; i < *n_items; i++)
    {
        if(strcasecmp(items[i],"Timezone") == 0) _cur_row = i; 
    }
    menuTFTFlushValue(abs(*shift), _cur_row, negpos_conditions, &negpos_cond_size, &_bg);
    (*shift >= 0) ? sprintf(buf, "CET+%d",*shift) : sprintf(buf, "CET%d",*shift);
    TFT_print(buf, x + _width/2, 3 + (TFT_getfontheight() + 3) *_cur_row);
}

void menuTFTPrintPlaymodeIndicators(play_state_data_t* data, int ind){
    int x1 = 10;
    int x2 = x1 + 300;
    int y1 = (TFT_getfontheight() * 9) + 16;
    int y2 = (y1 + TFT_getfontheight() * 4) - 23;
    const int width = x2 - x1;
    const int height = y2 - y1;
    TFT_resetclipwin();
    TFT_setclipwin(x1, y1, x2, y2);
    _bg = TFT_BLACK;

    updateStart(data, loopMarker, prevLoopMarker, &width, &height);
    updateLoopStart(data, loopMarker, prevLoopMarker, &width, &height, ind);
    updateLoopEnd(data, loopMarker, prevLoopMarker, &width, &height);

    updatePrevLoopMarker(loopMarker, prevLoopMarker);

    if(ind == SID_MODE && data->mode == SINGLE) clearLStartMarker(loopMarker, &width, &height);
}

void menuTFTInitPlaymodeIndicators(){
    resetPrevLoopMarker(prevLoopMarker);
}

void menuTFTPrintADSRCurve(adsr_data_t* data){
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    TFT_saveClipWin();

    //Window params
    const int x1 = 17;
    const int x2 = x1 + _width - 35;
    const int y1 = (TFT_getfontheight() * 9);
    const int y2 = (y1 + TFT_getfontheight() * 8);
    const int width = x2 - x1;
    const int height = y2 - y1;
    TFT_setclipwin(x1, y1, x2, y2);

    //Update Points
    updateAttackPoint(curAdsrPoints, prevAdsrPoints, data, &adsrStartPoint,&width, &height);
    updateDecayPoint(curAdsrPoints, prevAdsrPoints, data, &adsrStartPoint, &width, &height);
    updateSustainPoint(curAdsrPoints, prevAdsrPoints, data, &width, &height);
    updateReleasePoint(curAdsrPoints, prevAdsrPoints, data, &width, &height);

    //Flush indicators
    printIndicatorPoints(prevAdsrPoints, &adsrStartPoint, 2, TFT_BLACK);
    //Reprint
    printIndicatorPoints(curAdsrPoints, &adsrStartPoint, 2, TFT_RED);

    TFT_restoreClipWin();
    updatePrevCurve(curAdsrPoints, prevAdsrPoints);
    
}

void menuTFTPrintMatrixRowIndicator(int select, int row){
    int h = 8, w = 8;
    int x_offset = _width/3;
    TFT_setclipwin(0 ,TFT_getfontheight()* 2 +18, x_offset, _height);
    int y = 9;
    if(select){
        TFT_fillCircle(16, y + (TFT_getfontheight() + 8)*row , 4, TFT_RED);
    }else{
        TFT_fillCircle(16, y + (TFT_getfontheight() + 8)*row , 4, TFT_BLACK);
    }
}

void menuTFTInitAdsrCurve(){

    //Init startpoint
    adsrStartPoint.x = 2;
    adsrStartPoint.y = 102;
    resetPrevCurve(prevAdsrPoints);
}

void menuTFTPrintCurrentSettings(char* bank, char* preset, param_data_t* data){
    TFT_saveClipWin();
    _bg = TFT_BLACK;
    _fg = TFT_WHITE;
    int x = 4, x_incr = _width/2, y = 8, y_incr = TFT_getfontheight() + 24;
    char buf[64];

    TFT_setclipwin(0, TFT_getfontheight()+9 + y, x_incr, 80);
    TFT_print("Current bank:", CENTER, 8);
    TFT_print(bank, (x_incr/2) - TFT_getStringWidth(bank) / 2, TFT_getfontheight() + 8 + 4);

    TFT_restoreClipWin();
    TFT_setclipwin(x_incr, TFT_getfontheight()+9 + y, _width, 80);
    TFT_print("Current preset:", CENTER, 8);
    TFT_print(preset, (x_incr/2) - TFT_getStringWidth(preset) / 2, TFT_getfontheight() + 8 + 4);

    TFT_restoreClipWin();
    TFT_print("Voice: 0", 10, 104);
    sprintf(buf, "Mode: %s", playmode_modes[data[0].play_state.mode]);
    TFT_print(buf, TFT_getStringWidth("Voice: 0") + 40, 104);    

    TFT_print("Voice: 1", 10, 159);
    sprintf(buf, "Mode: %s", playmode_modes[data[1].play_state.mode]);
    TFT_print(buf, TFT_getStringWidth("Voice: 1") + 40, 159);    

}

void menuTFTPrintCurrentPresetSettings(char* title, char* data){
    char buf[32];
    strcpy(buf, title);
    strcat(buf, data);
    TFT_resetclipwin();
    _bg = TFT_BLACK;
    _fg = TFT_WHITE;
    TFT_drawFastHLine(0, 2* TFT_getfontheight() + 18, _width, TFT_WHITE);
    TFT_fillRect(0, 2* TFT_getfontheight() + 22, _width-1, TFT_getfontheight(), TFT_BLACK);
    TFT_print(buf, CENTER, 2* TFT_getfontheight() + 22);
    TFT_drawFastHLine(0, 3* TFT_getfontheight() + 24, _width, TFT_WHITE);
}

void menuTFTFeedbackMenuItemHSpaced(int *cnt, int* pos, const char** items, const int* n_items, int state){
    int x = 0, w = 0, y = 4;
    color_t clr;
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    int x_incr = _width / *n_items;
    //grab timer data
    int position = *pos;
    int count = *cnt;
    //ESP_LOGI("UI", "Pos: %d, Cnt: %d", pos, cnt);
    w = TFT_getStringWidth((char *)items[position]);
    x = x_incr * position + (x_incr/2) - (w/2);
    _bg = TFT_BLACK;
    _fg = TFT_WHITE;
    if(state == MS_SUCCESS)clr = TFT_GREEN;
    else clr = TFT_RED;

    if((count%2) == 0) _fg = clr;
    TFT_print((char *)items[position], x, y);
}
//-------------------------------------------------------------------------------------------------------




//Utility
//-------------------------------------------------------------------------------------------------------
void menuTFTUpdateProgress(char *text, int progress){
    char s[32];
    int w;
    snprintf(s, 32, "%s progress ", text);
    w = TFT_getStringWidth(s);
    snprintf(s, 32, "%s progress %3d%%", text, progress);
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    int y = 8 + TFT_getfontheight();
    TFT_fillRect(w, TFT_getfontheight() + 8, 320 - w, TFT_getfontheight() + 3, _bg);
    TFT_print(s, 3, TFT_getfontheight() + 8);
}

void menuTFTPrintBrowseTagList(list_t *tag_list){
    if(_bbox_list != NULL) list_free(_bbox_list);
    _cur_el = -1;
    _cur_row = 0;
    _bbox_list = list_create();
    _bg = (color_t){ 32, 32, 32 };
    TFT_fillWindow(_bg);
    _bg = (color_t){ 32, 32, 64 };
    _fg = TFT_LIGHTGREY;
    TFT_X = 3;
    TFT_Y = 0;

    // print all items
    list_each_element(tag_list, printTags);
    
    // highlight first
    menuTFTHighlightNextEl();
  
}

int menuTFTHighlightNextEl(){
    _bg = TFT_BLACK;
    bbox_t *bbox = list_get_item(_bbox_list, _cur_el)->value;
    TFT_drawRect(bbox->x, bbox->y, bbox->w, bbox->h, _bg);
    _cur_el++;
    if(_cur_el >= _bbox_list->count) _cur_el = 0;
    bbox = list_get_item(_bbox_list, _cur_el)->value;
    _fg = TFT_CYAN;
    TFT_drawRect(bbox->x, bbox->y, bbox->w, bbox->h, _fg);
    return _cur_el;
}

int menuTFTHighlightPrevEl(){
    _bg = TFT_BLACK;
    bbox_t *bbox = list_get_item(_bbox_list, _cur_el)->value;
    TFT_drawRect(bbox->x, bbox->y, bbox->w, bbox->h, _bg);
    _cur_el--;
    if(_cur_el < 0) _cur_el = _bbox_list->count - 1;
    bbox = list_get_item(_bbox_list, _cur_el)->value;
    _fg = TFT_CYAN;
    TFT_drawRect(bbox->x, bbox->y, bbox->w, bbox->h, _fg);
    return _cur_el;
}

void menuTFTPrintChar(char c, int pos){
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 3 + pos * 14;
    char s[2] = "\0";
    s[0] = c;
    int y = 8 + TFT_getfontheight();
    Font f = cfont;

    TFT_setFont(UBUNTU16_FONT, NULL);
    TFT_fillRect(TFT_X, y, 14, TFT_getfontheight(), _bg);                       //flush previous char
    TFT_print(s, TFT_X, y);                                                     //print new char
    TFT_fillRect(TFT_X, y, 14, TFT_getfontheight(), _bg);    
    cfont = f;
}

void menuTFTPrintCharSettings(char c, int pos, print_ids_t id){
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 3 + pos * 14;
    char s[2] = "\0";
    if(id == PRINT_UPPER && isalpha(c) != 0) s[0] = toupper(c);
    else s[0] = c;
    int y = 8 + TFT_getfontheight(), x = 0;
    Font f = cfont;
    //ESP_LOGI("UI", "POS: %d | strlen token : %d | char: %c", pos, strlen(CONFIG_FREESOUND_TOKEN), s[0]);
    if(pos >= 22){
        TFT_X = 3 + (pos - 22) * 14;
        y = (8 + TFT_getfontheight()) * 2;
    }
    TFT_setFont(UBUNTU16_FONT, NULL);
    if(pos == 21) TFT_fillRect(3, y*2, 14, TFT_getfontheight(), _bg);  
    TFT_fillRect(TFT_X, y, 14, TFT_getfontheight(), _bg);                       //flush previous char
    TFT_print(s, TFT_X, y);                                                     //print new char
    TFT_fillRect(TFT_X, y, 14, TFT_getfontheight(), _bg);    
    cfont = f;
}

int menuTFTPrintAllCharSettings(const char* s){
    int sz = strlen(s);
    int i = 0;
    for(; i < sz; i++)
    {
        menuTFTPrintCharSettings(s[i], i, PRINT_NORM);
    }
    return i;
}

void menuTFTPrintFileBrowser(int currentFile, int maxFiles, const cJSON* desc){
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    TFT_fillWindow(TFT_BLACK);
    char buf[64];
    _cur_el = 0;
    _cur_row = 0;
    _fg = TFT_WHITE;
    _bg = TFT_BLACK;
    TFT_X = 3;
    if(cJSON_IsNumber(cJSON_GetObjectItem(desc, "id")))
        snprintf(buf, 64, "File %d out of %d, id: %d", currentFile + 1, maxFiles, cJSON_GetObjectItem(desc, "id")->valueint);
    else
        snprintf(buf, 64, "File %d out of %d, id: %s", currentFile + 1, maxFiles, cJSON_GetObjectItem(desc, "id")->valuestring);
    TFT_print(buf, 3, (TFT_getfontheight() + 3) * _cur_row++);
    TFT_print("Name:", 3, 3 + (TFT_getfontheight() + 3) * _cur_row++);
    cleanString(cJSON_GetObjectItem(desc, "name")->valuestring);
    TFT_print(cJSON_GetObjectItem(desc, "name")->valuestring, 3, (TFT_getfontheight() + 3) * _cur_row++);

    TFT_print("Description:", 3, (TFT_getfontheight() + 3) * _cur_row++);
    cleanString(cJSON_GetObjectItem(desc, "description")->valuestring);
    TFT_print(cJSON_GetObjectItem(desc, "description")->valuestring, 3, (TFT_getfontheight() + 3) * _cur_row++);

    TFT_print("Tags:", 3, (TFT_getfontheight() + 3) * _cur_row++);
    cleanString(cJSON_GetObjectItem(desc, "tags_s")->valuestring);
    TFT_print(cJSON_GetObjectItem(desc, "tags_s")->valuestring, 3, (TFT_getfontheight() + 3) * _cur_row++);

    TFT_print("User:", 3, (TFT_getfontheight() + 3) * _cur_row++);
    cleanString(cJSON_GetObjectItem(desc, "username")->valuestring);
    TFT_print(cJSON_GetObjectItem(desc, "username")->valuestring, 3, (TFT_getfontheight() + 3) * _cur_row++);
    
    TFT_print("Url:", 3, (TFT_getfontheight() + 3) * _cur_row++);
    cleanString(cJSON_GetObjectItem(desc, "url")->valuestring);
    TFT_print(cJSON_GetObjectItem(desc, "url")->valuestring, 3, (TFT_getfontheight() + 3) * _cur_row++);
    
    TFT_print("License:", 3, (TFT_getfontheight() + 3) * _cur_row++);
    cleanString(cJSON_GetObjectItem(desc, "license")->valuestring);
    TFT_print(cJSON_GetObjectItem(desc, "license")->valuestring, 3, (TFT_getfontheight() + 3) * _cur_row++);
}

void menuTFTPrintDecoding(){
    _cur_el = -1;
    _cur_row = 0;
    _bbox_list = list_create();
    _bg = (color_t){ 32, 32, 32 };
    TFT_fillWindow(_bg);
    TFT_print("MP3 file to raw samples:", 3, 4);
}

void menuTFTAnimateFileBrowser(const cJSON* desc){
    char *s;
    _cur_el++;
    _cur_row = 2;
    int tooWide = 0;
    _bg = TFT_BLACK;
    _fg = TFT_WHITE;
    s = cJSON_GetObjectItem(desc, "name")->valuestring;
    tooWide |= printSubStringIfTooWide(s, 3, (TFT_getfontheight() + 3) * _cur_row, _cur_el);
    _cur_row += 2;
    s = cJSON_GetObjectItem(desc, "description")->valuestring;
    tooWide |= printSubStringIfTooWide(s, 3, (TFT_getfontheight() + 3) * _cur_row, _cur_el);
    _cur_row += 2;
    s = cJSON_GetObjectItem(desc, "tags_s")->valuestring;
    tooWide |= printSubStringIfTooWide(s, 3, (TFT_getfontheight() + 3) * _cur_row, _cur_el);
    _cur_row += 2;
    s = cJSON_GetObjectItem(desc, "username")->valuestring;
    tooWide |= printSubStringIfTooWide(s, 3, (TFT_getfontheight() + 3) * _cur_row, _cur_el);
    _cur_row += 2;
    s = cJSON_GetObjectItem(desc, "url")->valuestring;
    tooWide |= printSubStringIfTooWide(s, 3, (TFT_getfontheight() + 3) * _cur_row, _cur_el);
    _cur_row += 2;
    s = cJSON_GetObjectItem(desc, "license")->valuestring;
    tooWide |= printSubStringIfTooWide(s, 3, (TFT_getfontheight() + 3) * _cur_row, _cur_el);
    if(!tooWide) _cur_el = 0;
}

int printSubStringIfTooWide(char *s, int x, int y, int pos){
    if(strlen(s) > pos){
        if(TFT_getStringWidth(&s[pos]) + 3 > 317){
            TFT_fillRect(x, y, 317, (TFT_getfontheight() + 3), _bg);
            TFT_print(&s[_cur_el], x, y);
            return 1;
        }
    }
    return 0;
}

int printTags(list_item_t* it){
    char *s = (char*)it->value;
    if(TFT_X + TFT_getStringWidth(s) > _width){
        _cur_row++;
        TFT_X = 3;
    }
    if((TFT_getfontheight()+3)*_cur_row + TFT_getfontheight() + 4 + TFT_getfontheight() + 9 >= _height)
        return 0;
    
    bbox_t *bbox = (bbox_t*)calloc(sizeof(bbox_t), 1);
    bbox->x = TFT_X - 2; bbox->y = (TFT_getfontheight()+3)*_cur_row;
    bbox->w = TFT_getStringWidth(s) + 4; bbox->h = TFT_getfontheight() + 4;
    list_add_element(_bbox_list, (void*)bbox);
    
    TFT_print(s, TFT_X, 3 + (TFT_getfontheight() + 3) *_cur_row);
    TFT_X += 3;
    return 0;
}

void menuTFTUpdatePlayState(int vid, int state){
    static int p_state[2] = {0, 0};
    if(p_state[vid] == state && state != -1) return; // -1 for re-drawing last position
    if(state == -1) state = p_state[vid];
    TFT_fillRect(10, 120 + 55 * vid, 301, 20, (color_t){32, 32, 48});
    TFT_drawFastVLine(10 + state, 120 + 55 * vid, 20, TFT_WHITE);
    p_state[vid] = state;
}

void menuTFTPrintInputError(char* s){
    TFT_setclipwin(0,TFT_getfontheight()+9, _width-1, _height);
    char str[32];
    sprintf(str, "%s", s);
    _fg = TFT_RED;
    _bg = TFT_BLACK;
    TFT_print(str, CENTER, CENTER);
}

void menuTFTClearListItem(int* activeSlot){
    TFT_setclipwin(0, 3* TFT_getfontheight() + 32, _width-1, _height);
    _bg = TFT_BLACK;
    int x = 0;
    int x_incr = _width / 3;
    int y_start = 0;
    int y = 8 + TFT_getfontheight();
    int y_draw = 0;

    if(*activeSlot < 10)y_draw = y_start + 3 + (TFT_getfontheight() + 3) * (*activeSlot);
    if(*activeSlot >= 10 && *activeSlot < 20){
        y_draw = y_start + 3 + (TFT_getfontheight() + 3) * ((*activeSlot) - 10);
        x = x_incr;
    }
    if(*activeSlot >= 20 && *activeSlot < 30){
        y_draw = y_start + 3 + (TFT_getfontheight() + 3) * ((*activeSlot) - 20);
        x = x_incr*2;
    }            
        
    TFT_fillRect(x, y_draw - 4, x_incr-1, TFT_getfontheight() + 4, _bg);     
}
//-------------------------------------------------------------------------------------------------------