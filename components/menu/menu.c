#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "menu.h"
#include "tft.h"
#include "freesound.h"
#include "list.h"
#include "mp3.h"
#include "menusys.h"
#include "fileio.h"
#include "storage.h"
#include "ui_events.h"
#include "esp_log.h"
#include "audio.h"
#include "menutft.h"
#include "menu_utils.h"
#include "menu_types.h"
#include "audio_luts.h"
#include "menu_items.h"
#include "menutft_utils.h"
#include "gpio.h"
#include "audio.h"
#include "preset.h"
#include "wifi.h"
#include "esp_wifi.h"
#include "timer_utils.h"
#include "rest-api.h"
#include "menu_config.h"

#define N_MAIN_MENUS 5


static param_data_t data [2]; //data that gets displayed
static effect_data_t effectData;
static uint16_t envelopeIndex[2][3];
static bool pbs_state_v0, pbs_state_v1;
static matrix_ui_row_t matrix[8];
static matrix_event_t matrix_event;
static char current_bank[8] =  {"DEFAULT"};
static char current_preset_name[13] = {"Init"};
static int current_fb_pos = 0;
static int tz_shift = 0;
static menu_fb_state_t fb_state;

menusys_t *_ms = NULL;
static xQueueHandle v0_queue = NULL;
static xQueueHandle v1_queue = NULL;
static xQueueHandle eff_queue = NULL;
static xQueueHandle playbackspeed_state_queue_v0 = NULL;
static xQueueHandle playbackspeed_state_queue_v1 = NULL;
static xQueueHandle mode_queue_v0 = NULL;
static xQueueHandle mode_queue_v1 = NULL;
static xQueueHandle m_event_queue = NULL;
static xQueueHandle ui_ev_queue = NULL;
static voice_play_state_t play_state_v0 = SINGLE, play_state_v1 = SINGLE;

// data which needs to be passed from one menu state to another
// should be only used to pass data between defined state transitions
void *_state_data = NULL;
void *_state_voice = NULL;
void *_state_json = NULL;
void *_fb_state = NULL;

// handler has format caller_id, caller_name, caller_item data, event, event data
static int timer_handler(int it_id, int event, void* event_data){
    menuTFTPrintTime(&tz_shift);
    return 0; // remain in current menu
}

static int main_menu_def_handler(int it_id, int event, void* event_data){
    const int menu_states[] = {M_PLAY, M_SLOT, M_BROWSE, M_PRESET, M_MORE};
    const int states = sizeof(menu_states)/sizeof(int);
    static int menu_state_current = 0;

    switch(event){
        case EV_ENTERED_MENU:
            menuTFTSelectMainMenu(menu_state_current, 0);
            menuTFTUpdatePlayState(0, -1);
            menuTFTUpdatePlayState(1, -1);
            menuTFTPrintCurrentSettings(current_bank, current_preset_name, data);
            break;
        case EV_FWD:
            menu_state_current++;
            if(menu_state_current >= states) menu_state_current = 0;
            menuTFTSelectMainMenu(menu_state_current, 0);
            break;
        case EV_BWD:
            menu_state_current--;
            if(menu_state_current < 0) menu_state_current = states - 1;
            menuTFTSelectMainMenu(menu_state_current, 0);
            break;
        case EV_LONG_PRESS:
            menuTFTSelectMainMenu(menu_state_current, 1);
            return menu_states[menu_state_current];
            break;
        case EV_UPDATE_V0_POS:
            menuTFTUpdatePlayState(0, (int) event_data);
            break;
        case EV_UPDATE_V1_POS:
            menuTFTUpdatePlayState(1, (int) event_data);
            break;
        default:
            break;
    }
    
    return 0; // remain in current menu
}

static int about_def_handler(int it_id, int event, void* event_data){
    switch(event){
        case EV_ENTERED_MENU:
            menuTFTPrintAbout();
            //unmountSDStorage();
            break;
        case EV_LONG_PRESS:
            //mountSDStorage();
            menuTFTFlushMenuDataRect();
            return M_MORE;
            break;
        default:
            break;
    }

    return 0; // remain in current menu
}

static int more_def_handler(int it_id, int event, void* event_data){
    const int menu_states[] = {M_SETTINGS, M_ABOUT};
    const int states = sizeof(menu_states)/sizeof(int);
    static int menu_state_current = 0;

    switch(event){
        case EV_ENTERED_MENU:
            menuTFTPrintMenu(more_menus, &n_more_menus);
            menuTFTSelectMenuItem(&menu_state_current, 0, more_menus, &n_more_menus);
            break;
        case EV_FWD:
            menu_state_current++;
            if(menu_state_current >= states) menu_state_current = 0;
            menuTFTSelectMenuItem(&menu_state_current, 0, more_menus, &n_more_menus);
            break;
        case EV_BWD:
            menu_state_current--;
            if(menu_state_current < 0) menu_state_current = states - 1;
            menuTFTSelectMenuItem(&menu_state_current, 0, more_menus, &n_more_menus);
            break;
        case EV_SHORT_PRESS:
            return menu_states[menu_state_current];
            break;
        case EV_LONG_PRESS:
            menuTFTFlushMenuDataRect();
            return M_MAIN;
            break;
        default:
            break;
    }
    
    return 0; // remain in current menu
}

static int settings_def_handler(int it_id, int event, void* event_data){
    const int menu_items[] = {SID_WIFI_SSID, SID_WIFI_PASSWD, SID_APIKEY, SID_TIMEZONE};
    const int items = sizeof(menu_items)/sizeof(int);
    static int menu_pos = 0, activeSlot = 0, selected = 0;
    static cJSON *cfgData = NULL, *settings = NULL;
    switch(event){
        case EV_ENTERED_MENU:
            selected = 0;
            menuTFTPrintMenu(settings_menus, &n_settings_menus);
            menuTFTSelectMenuItem(&menu_pos, 0, settings_menus, &n_settings_menus);
            if(_state_json != NULL)cfgData = (cJSON*) _state_json;  
            else cfgData = readJSONFileAsCJSON("/sdcard/CONFIG.JSN");
            if(cfgData != NULL){
                settings = cJSON_GetObjectItemCaseSensitive(cfgData, "settings");
                if(settings != NULL)menuTFTPrintSettings(settings);
                menuTFTPrintTimezone(settings_menus, &n_settings_menus, &tz_shift);
            }else ESP_LOGE("UI", "couldn't fetch cfgData from state or file");
            break;
        case EV_FWD:
            if(!selected){
                menu_pos++;
                if(menu_pos >= items) menu_pos = 0;
                menuTFTSelectMenuItem(&menu_pos, 0, settings_menus, &n_settings_menus);
            }else{
                incSettingsItem(&tz_shift, menu_items[menu_pos]);
                menuTFTPrintTimezone(settings_menus, &n_settings_menus, &tz_shift);
            }
            break;
        case EV_BWD:
            if(!selected){
                menu_pos--;
                if(menu_pos < 0) menu_pos = items - 1;
                menuTFTSelectMenuItem(&menu_pos, 0, settings_menus, &n_settings_menus);
            }else{
                decSettingsItem(&tz_shift, menu_items[menu_pos]);
                menuTFTPrintTimezone(settings_menus, &n_settings_menus, &tz_shift);
            }
            break;
        case EV_SHORT_PRESS:
            if(menu_items[menu_pos] != SID_TIMEZONE){
                _state_json = (void*) cfgData;
                _state_data = (void*) &menu_pos;
                return M_SETTINGS_INPUT;
            }else{
                selected = !selected;
                menuTFTSelectMenuItem(&menu_pos, selected, settings_menus, &n_settings_menus);
            }
            break;
        case EV_LONG_PRESS: ;
            int wifiChanged = wifiSettingsChanged(settings);
            //replace tz_shift value
            cJSON_ReplaceItemInObjectCaseSensitive(settings, "tz_shift", cJSON_CreateNumber(tz_shift));
            //save current settings on menu exit
            writeJSONFile("/sdcard/CONFIG.jsn", cJSON_Print(cfgData));
            //set token 
            char* token = cJSON_GetObjectItem(settings, "apikey")->valuestring;
            freesoundSetToken(token);
            //if wifi settings changed reconnect wifi with new config
            if(wifiChanged){
                ESP_LOGI("UI", "Wifi settings have changed. Reconnecting...");
                char *ssid = cJSON_GetObjectItem(settings, "ssid")->valuestring;
                char *passwd = cJSON_GetObjectItem(settings, "passwd")->valuestring;
                wifi_config_t wifi_config;
                memset(&wifi_config, 0, sizeof(wifi_config));
                strcpy((char*) wifi_config.sta.ssid, ssid);
                strcpy((char*) wifi_config.sta.password, passwd);
                restartWifi(&wifi_config);
            }
            menuTFTFlushMenuDataRect();
            _state_data = NULL;
            _state_json = NULL;
            menu_pos = 0;
            cJSON_Delete(cfgData);
            return M_MORE;
            break;
        default:
            break;
    }

    return 0; // remain in current menu
}

static int settings_input_def_handler(int it_id, int event, void* event_data){
    const char *c_list = "=0123456789abcdefghijklmnopqrstuvwxyz-_ !?<^";
    static int pos = 0, c = 1, menu_pos = 0;
    static char input[48];
    char *buf;
    char title[32];
    static cJSON *root = NULL, *settings = NULL, *val = NULL;

    switch(event){
        case EV_ENTERED_MENU:
            bzero(input, 48);
            bzero(title, 32);
            pos = 0;
            c = 41;
            menu_pos = 0;
            menuTFTResetTextWrap();
            //Get settings object from _state_json & menu_pos from _state_data
            if(_state_json != NULL){
                root = (cJSON*) _state_json;
                if(root != NULL)settings = cJSON_GetObjectItemCaseSensitive(root, "settings");
                menu_pos = *((int*) _state_data);
                if(settings != NULL)sprintf(title, "Enter %s:", settings_menus[menu_pos]);
                menuTFTPrintInputMenu(title);
                val = cJSON_GetArrayItem(settings, menu_pos);
                //print current valuestring, copy to input buffer and increase pos to stringlength
                if(cJSON_IsString(val)){
                    pos = menuTFTPrintAllCharSettings(val->valuestring);
                    strcpy(input, val->valuestring);
                }
            }else ESP_LOGE("PRESET", "_state_json is NULL");
        case EV_FWD:
            c++;
            if(c>43) c = 43;
            menuTFTPrintChar(input, pos, c_list[c], PRINT_NORM);
            break;
        case EV_BWD:
            c--;
            if(c<0)c=0;
            menuTFTPrintChar(input, pos, c_list[c], PRINT_NORM);
            break;
        case EV_SHORT_PRESS:
            switch(c_list[c]){
                case '^':
                    _state_data = NULL;
                    menuTFTFlushMenuDataRect();
                    return M_SETTINGS;
                    break;
                case '<':
                    input[pos] = '\0';
                    pos--;
                    if(pos<0)pos=0;
                    menuTFTPrintChar(input, pos, c_list[c], PRINT_NORM);
                    break;
                case '=':
                    if(pos == 0) break;
                    input[pos] = '\0';
                    buf = calloc(strlen(input) + 1, 1);
                    strcpy(buf, input);
                    cJSON* val = cJSON_GetArrayItem(settings, menu_pos);
                    cJSON_ReplaceItemInObjectCaseSensitive(settings, val->string, cJSON_CreateString(buf));
                    _state_data = NULL;
                    pos = 0;
                    return M_SETTINGS;
                    break;
                default:
                    input[pos] = c_list[c];
                    pos++;
                    if(menu_pos == 2){
                        if(pos>42)pos=42;
                    }else{
                        if(pos>32)pos=32;
                    }
                    menuTFTPrintChar(input, pos, c_list[c], PRINT_NORM);
                    break;
            }
            break;
        case EV_LONG_PRESS:
            //print current char
            if(c_list[c] == '^' || c_list[c] == '<') break;
            input[pos] = toupper(c_list[c]);
            menuTFTPrintChar(input, pos, c_list[c], PRINT_UPPER);
            pos++;
            if(menu_pos == 2){
                if(pos>42)pos=42;
            }else{
                if(pos>32)pos=32;
            }
            menuTFTPrintChar(input, pos, c_list[c], PRINT_NORM);
            break;
        default:
            break;
    }

    return 0;
}

static int browse_def_handler(int it_id, int event, void* event_data){
    const int menu_states[] = {M_BROWSE_TAG, M_BROWSE_ID, M_BROWSE_SEARCH, M_BROWSE_USER};
    const int states = sizeof(menu_states)/sizeof(int);
    static int menu_state_current = 0;
    
    switch(event){
        case EV_ENTERED_MENU:
            if(!isWiFiConnected()) return M_MAIN;
            menuTFTPrintMenuH(browse_menus, &n_browse_menus);
            menuTFTSelectMenuItemH(&menu_state_current, 0, browse_menus, &n_browse_menus);
            break;
        case EV_FWD:
            menu_state_current++;
            if(menu_state_current >= states) menu_state_current = 0;
            menuTFTSelectMenuItemH(&menu_state_current, 0, browse_menus, &n_browse_menus);
            break;
        case EV_BWD:
            menu_state_current--;
            if(menu_state_current < 0) menu_state_current = states - 1;
            menuTFTSelectMenuItemH(&menu_state_current, 0, browse_menus, &n_browse_menus);
            break;
        case EV_SHORT_PRESS:
            return menu_states[menu_state_current];
            break;
        case EV_LONG_PRESS:
            menu_state_current = 0;
            menuTFTFlushMenuDataRect();
            return M_MAIN;
            break;
        default:
            break;
    }

    return 0; // remain in current menu
}

static int play_def_handler(int it_id, int event, void* event_data){
    const int menu_states[] = {M_VOICE0, M_VOICE1, M_EFFECTS, M_CV_MATRIX};
    const int states = sizeof(menu_states)/sizeof(int);
    static int menu_state_current = 0;

    switch(event){
        case EV_ENTERED_MENU:
            menuTFTPrintMenu(play_menus, &n_play_menus);
            menuTFTSelectMenuItem(&menu_state_current, 0, play_menus, &n_play_menus);
            break;
        case EV_FWD:
            menu_state_current++;
            if(menu_state_current >= states) menu_state_current = 0;
            menuTFTSelectMenuItem(&menu_state_current, 0, play_menus, &n_play_menus);
            break;
        case EV_BWD:
            menu_state_current--;
            if(menu_state_current < 0) menu_state_current = states - 1;
            menuTFTSelectMenuItem(&menu_state_current, 0, play_menus, &n_play_menus);
            break;
        case EV_SHORT_PRESS:
            _state_voice = (void*) menu_state_current;
            if(menu_state_current == 0 || menu_state_current == 1) return M_VOICE;
            else return menu_states[menu_state_current];
            break;
        case EV_LONG_PRESS:
            menu_state_current = 0;
            menuTFTFlushMenuDataRect();
            return M_MAIN;
            break;
        default:
            break;
    }

    return 0; // remain in current menu
}

static int voice_def_handler(int it_id, int event, void* event_data){
    const int menu_states[] = {M_FILTER, M_ADSR, M_PLAYMODE};
    const int menu_items[] = {SID_TRIG_TYPE, SID_VOLUME, SID_PAN, SID_PITCH, SID_PITCH_CV_ACTIVE, SID_PBSPEED, SID_DIST_ACTIVE, SID_DIST, SID_DELAY_SEND};
    const int items = sizeof(menu_items)/sizeof(int);
    const int states = sizeof(menu_states)/sizeof(int);
    const int all_items = items + states;
    static int menu_pos = 0;
    static int vid = 0; //pass current voice state to next handler
    static int selected = 0;

    switch(event){
        case EV_ENTERED_MENU:
            vid = (int) _state_voice;
            menuTFTPrintVoiceMenu();
            menuTFTSelectVoiceMenu(menu_pos,selected);
            menuTFTPrintVoiceValues(&data[vid], PRINT_ALL);
            break;
        case EV_FWD:
           if(!selected){
                menu_pos++;
                if(menu_pos >= all_items) menu_pos = 0;
                menuTFTSelectVoiceMenu(menu_pos,selected);
            }else{
                if(vid)
                    incParamValue(&data[vid], menu_items[menu_pos], vid, &pbs_state_v1, matrix, playbackspeed_state_queue_v1);
                else
                    incParamValue(&data[vid], menu_items[menu_pos], vid, &pbs_state_v0, matrix, playbackspeed_state_queue_v0);
                menuTFTPrintVoiceValues(&data[vid],menu_items[menu_pos]);

                //Push new matrix event when pitch cv active changed
                if(menu_pos == SID_PITCH_CV_ACTIVE){
                        if(vid) matrix_event.changed_param = MTX_V1_PITCH;
                        else matrix_event.changed_param = MTX_V0_PITCH;

                        matrix_event.source = vid;
                        matrix_event.amount = matrix[vid].amt;
                        xQueueSend(m_event_queue, (void*) &matrix_event, portMAX_DELAY);
                }
                //Push data into queue
                if(vid){
                    xQueueSend(v1_queue, (void*) &data[vid], portMAX_DELAY);
                }else{
                    xQueueSend(v0_queue, (void*) &data[vid], portMAX_DELAY);
                }
            }
            break;
        case EV_BWD:
            if(!selected){
                menu_pos--;
                if(menu_pos < 0) menu_pos = all_items - 1;
                menuTFTSelectVoiceMenu(menu_pos,selected);
            }else{
                if(vid)
                    decParamValue(&data[vid], menu_items[menu_pos],vid, &pbs_state_v1, matrix, playbackspeed_state_queue_v1);
                else
                    decParamValue(&data[vid], menu_items[menu_pos],vid, &pbs_state_v0, matrix, playbackspeed_state_queue_v0);
                menuTFTPrintVoiceValues(&data[vid],menu_items[menu_pos]);

                //Push new matrix event when pitch cv active changed
                if(menu_pos == SID_PITCH_CV_ACTIVE){
                        if(vid) matrix_event.changed_param = MTX_V1_PITCH;
                        else matrix_event.changed_param = MTX_V0_PITCH;

                        matrix_event.source = vid;
                        matrix_event.amount = matrix[vid].amt;
                        xQueueSend(m_event_queue, (void*) &matrix_event, portMAX_DELAY);
                }

                //Push data into queue
                if(vid){
                    xQueueSend(v1_queue, (void*) &data[vid], portMAX_DELAY);
                }else{
                    xQueueSend(v0_queue, (void*) &data[vid], portMAX_DELAY);
                }

            }
            break;
        case EV_SHORT_PRESS:
            if(!selected){
                if(menu_pos >= items){
                    return menu_states[menu_pos - items];
                }else{
                    selected = 1;
                    menuTFTSelectVoiceMenu(menu_pos,selected);
                    break;
                }
            }else{
                selected = 0;
                menuTFTSelectVoiceMenu(menu_pos,selected);
                // set latch mode
                if(data[vid].trig_mode_latch)
                    enableTrigModeLatch(vid);
                else
                    disableTrigModeLatch(vid);
                break;
            }
            break;
        case EV_LONG_PRESS:
            selected = 0;
            menu_pos = 0;
            return M_PLAY;
            break;
        default:
            break;
    }

    return 0; // remain in current menu
}

static int filter_def_handler(int it_id, int event, void* event_data){
    const int menu_items[] = {SID_FILTER_ACTIVE, SID_BASE, SID_WIDTH, SID_Q};
    const int items = sizeof(menu_items)/sizeof(int);
    static int menu_pos = 0; 
    static int vid = 0; //pass current voice state to next handler
    static int selected = 0;

    switch(event){
        case EV_ENTERED_MENU:
            vid = (int) _state_voice;
            menuTFTPrintMenu(filter_menus, &n_filter_menus);
            menuTFTSelectMenuItem(&menu_pos, selected, filter_menus, &n_filter_menus);
            menuTFTPrintFilterValues(&data[vid].filter, PRINT_ALL);
            break;
        case EV_FWD:
           if(!selected){
                menu_pos++;
                if(menu_pos >= items) menu_pos = 0;
                menuTFTSelectMenuItem(&menu_pos, selected, filter_menus, &n_filter_menus);
            }else{
                //Increment value
                incFilterValue(&data[vid].filter, menu_items[menu_pos]);
                //Reprint values
                menuTFTPrintFilterValues(&data[vid].filter,menu_items[menu_pos]);
                //Push data into queue
                if(vid){
                    xQueueSend(v1_queue, (void*) &data[vid], portMAX_DELAY);
                }else{
                    xQueueSend(v0_queue, (void*) &data[vid], portMAX_DELAY);
                }
            }
            break;
        case EV_BWD:
            if(!selected){
                menu_pos--;
                if(menu_pos < 0) menu_pos = items - 1;
                menuTFTSelectMenuItem(&menu_pos, selected, filter_menus, &n_filter_menus);
            }else{
                //Decrement value
                decFilterValue(&data[vid].filter, menu_items[menu_pos]);
                //Reprint values
                menuTFTPrintFilterValues(&data[vid].filter,menu_items[menu_pos]);
                //Push data into queue
                if(vid){
                    xQueueSend(v1_queue, (void*) &data[vid], portMAX_DELAY);
                }else{
                    xQueueSend(v0_queue, (void*) &data[vid], portMAX_DELAY);
                }
            }
            break;
        case EV_SHORT_PRESS:
            if(!selected)
            {
                selected = 1;
                menuTFTSelectMenuItem(&menu_pos, selected, filter_menus, &n_filter_menus);
            }else{
                selected = 0;
                menuTFTSelectMenuItem(&menu_pos, selected, filter_menus, &n_filter_menus);
            }
            break;
        case EV_LONG_PRESS:
            selected = 0;
            menu_pos = 0;
            return M_VOICE;
            break;
        default:
            break;
    }

    return 0;
}

static int adsr_def_handler(int it_id, int event, void* event_data){
    const int menu_items[] = {SID_ATTACK, SID_DECAY, SID_SUSTAIN, SID_RELEASE};
    const int items = sizeof(menu_items)/sizeof(int);
    static int menu_pos = 0; 
    static int vid = 0; //pass current voice state to next handler
    static int selected = 0;

    switch(event){
        case EV_ENTERED_MENU:
            vid = (int) _state_voice;
            menuTFTPrintADSRMenu();
            menuTFTSelectMenuItem(&menu_pos, selected, adsr_menus, &n_adsr_menus);
            menuTFTPrintADSRValues(&data[vid].adsr, PRINT_ALL);
            menuTFTInitAdsrCurve(&data[vid].adsr);
            menuTFTPrintADSRCurve(&data[vid].adsr);
            break;
        case EV_FWD:
           if(!selected){
                menu_pos++;
                if(menu_pos >= items) menu_pos = 0;
                menuTFTSelectMenuItem(&menu_pos, selected, adsr_menus, &n_adsr_menus);
            }else{
                //Increment value
                incADSRValue(&data[vid].adsr, envelopeIndex[vid],menu_items[menu_pos]);
                //Reprint values
                menuTFTPrintADSRValues(&data[vid].adsr, menu_items[menu_pos]);
                menuTFTPrintADSRCurve(&data[vid].adsr);
                //Push data into queue
                if(vid){
                    xQueueSend(v1_queue, (void*) &data[vid], portMAX_DELAY);
                }else{
                    xQueueSend(v0_queue, (void*) &data[vid], portMAX_DELAY);
                }
            }
            break;
        case EV_BWD:
            if(!selected){
                menu_pos--;
                if(menu_pos < 0) menu_pos = items - 1;
                menuTFTSelectMenuItem(&menu_pos, selected, adsr_menus, &n_adsr_menus);
            }else{
                //Decrement value
                decADSRValue(&data[vid].adsr, envelopeIndex[vid], menu_items[menu_pos]);
                //Reprint values
                menuTFTPrintADSRValues(&data[vid].adsr,menu_items[menu_pos]);
                menuTFTPrintADSRCurve(&data[vid].adsr);
                //Push data into queue
                if(vid){
                    xQueueSend(v1_queue, (void*) &data[vid], portMAX_DELAY);
                }else{
                    xQueueSend(v0_queue, (void*) &data[vid], portMAX_DELAY);
                }

            }
            break;
        case EV_SHORT_PRESS:
            if(!selected)
            {
                selected = 1;
                menuTFTSelectMenuItem(&menu_pos, selected, adsr_menus, &n_adsr_menus);
            }else{
                selected = 0;
                menuTFTSelectMenuItem(&menu_pos, selected, adsr_menus, &n_adsr_menus);
            }
            break;
        case EV_LONG_PRESS:
            selected = 0;
            menu_pos = 0;
            return M_VOICE;
            break;
        default:
            break;
    }
    
    return 0;
}

static int playmode_def_handler(int it_id, int event, void* event_data){
    const int menu_items[] = {SID_MODE, SID_START, SID_LSTART, SID_LEND, SID_LPOSITION};
    const int items = sizeof(menu_items)/sizeof(int);
    static int menu_pos = 0; 
    static int vid = 0; //pass current voice state to next handler
    static int selected = 0;

    switch(event){
        case EV_ENTERED_MENU:
            vid = (int) _state_voice;
            menuTFTPrintPlaymodeMenu();
            menuTFTSelectMenuItem(&menu_pos, selected, playmode_menus, &n_playmode_menus);

            menuTFTPrintPlaymodeValues(&data[vid].play_state, PRINT_ALL);
            menuTFTInitPlaymodeIndicators();
            menuTFTPrintPlaymodeIndicators(&data[vid].play_state, menu_items[menu_pos]);
            break;
        case EV_FWD:
           if(!selected){
                menu_pos++;
                if(menu_pos >= items) menu_pos = 0;
                menuTFTSelectMenuItem(&menu_pos, selected, playmode_menus, &n_playmode_menus);
            }else{
                //Increment value
                if(vid){
                    incPlaymodeValue(&data[vid].play_state, menu_items[menu_pos], mode_queue_v1);
                }
                else
                    incPlaymodeValue(&data[vid].play_state, menu_items[menu_pos], mode_queue_v0);
                //Reprint values
                menuTFTPrintPlaymodeValues(&data[vid].play_state, menu_items[menu_pos]);
                menuTFTPrintPlaymodeIndicators(&data[vid].play_state, menu_items[menu_pos]);
                //Push data into queue
                if(vid){
                    xQueueSend(mode_queue_v1, &data[vid].play_state, portMAX_DELAY);
                }else{
                    xQueueSend(mode_queue_v0, &data[vid].play_state, portMAX_DELAY);
                }
            }
            break;
        case EV_BWD:
            if(!selected){
                menu_pos--;
                if(menu_pos < 0) menu_pos = items - 1;
                menuTFTSelectMenuItem(&menu_pos, selected, playmode_menus, &n_playmode_menus);
            }else{
                //Decrement value
                if(vid){
                    decPlaymodeValue(&data[vid].play_state, menu_items[menu_pos], mode_queue_v1);
                }
                else
                    decPlaymodeValue(&data[vid].play_state, menu_items[menu_pos], mode_queue_v0);
                //Reprint values
                menuTFTPrintPlaymodeValues(&data[vid].play_state, menu_items[menu_pos]);
                menuTFTPrintPlaymodeIndicators(&data[vid].play_state, menu_items[menu_pos]);
                //Push data into queue
                
                if(vid){
                    xQueueSend(mode_queue_v1, &data[vid].play_state, portMAX_DELAY);
                }else{
                    xQueueSend(mode_queue_v0, &data[vid].play_state, portMAX_DELAY);
                }

            }
            break;
        case EV_SHORT_PRESS:
            if(!selected)
            {
                selected = 1;
                menuTFTSelectMenuItem(&menu_pos, selected, playmode_menus, &n_playmode_menus);
            }else{
                selected = 0;
                menuTFTSelectMenuItem(&menu_pos, selected, playmode_menus, &n_playmode_menus);
            }
            break;
        case EV_LONG_PRESS:
            selected = 0;
            menu_pos = 0;
            return M_VOICE;
            break;
        default:
            break;
    }
    
    return 0;
}

static int matrix_def_handler(int it_id, int event, void* event_data){
    static int matrix_pos_v = 0, matrix_pos_h = 0;  //vertical/horizontal position
    static bool selected_v = 0, selected_h = 0;
    static int items_v = 8, items_h = 3;            //vertical/horizontal number of items in matrix
    static int cur_destination = 0;                 //current displayed destination
    static int dst_changed = 0;
    switch(event){
        case EV_ENTERED_MENU:
            matrix_pos_v = 0;
            matrix_pos_h = 0;
            selected_v = 0;
            selected_h = 0;
            
            menuTFTPrintMatrixMenu();
            menuTFTSelectMatrixItem(matrix_pos_v, 0,matrix_pos_h);
            menuTFTPrintMatrixAmount(matrix, PRINT_ALL);
            menuTFTPrintMatrixDestination(matrix, PRINT_ALL);

            //ESP_LOGI("UI", "Entered %d CV Matrix", matrix_pos_v);
            break;
        case EV_FWD: ;
            if(selected_h){
                //Incr Amount/Destination
                if(matrix_pos_h == 1){
                    incItemAmount(matrix, matrix_pos_v);
                    //Print matrix amount
                    menuTFTPrintMatrixAmount(matrix, matrix_pos_v);

                } 
                if(matrix_pos_h == 2){
                    dst_changed = 1;
                    incDestination(matrix, matrix_pos_v);
                    menuTFTPrintMatrixDestination(matrix, matrix_pos_v);
                    menuTFTPrintMatrixAmount(matrix, matrix_pos_v);        
                }
                //Prepare & send event to audio thread
                if(matrix_pos_h != 2 || (matrix[matrix_pos_v].dst != MTX_V0_PITCH && matrix[matrix_pos_v].dst != MTX_V1_PITCH)){
                    if(!((matrix_pos_h == 1) && (matrix[matrix_pos_v].dst == MTX_NONE))){
                        if(dst_changed){
                            //ESP_LOGI("UI", "Destination changed");
                            matrix_event.changed_param = matrix[matrix_pos_v].dst - 1;
                            matrix_event.source = matrix_pos_v;
                            matrix_event.amount = 0;
                            dst_changed = 0;
                            xQueueSend(m_event_queue, (void*) &matrix_event, portMAX_DELAY);
                        }
                        matrix_event.changed_param = matrix[matrix_pos_v].dst;
                        matrix_event.source = matrix_pos_v;
                        matrix_event.amount = matrix[matrix_pos_v].amt;
                        xQueueSend(m_event_queue, (void*) &matrix_event, portMAX_DELAY);
                    }
                }
            }else{
                if(selected_v){
                    //Moving horizontally
                    matrix_pos_h++;
                    if(matrix_pos_h >= items_h) matrix_pos_h = 0;
                    menuTFTSelectMatrixItem(matrix_pos_v,0,matrix_pos_h);
                }else{
                    //Moving vertically
                    matrix_pos_v++;
                    if(matrix_pos_v >= items_v) matrix_pos_v = 0;
                    menuTFTSelectMatrixItem(matrix_pos_v,0,matrix_pos_h);
                }
            }
            break;
        case EV_BWD: 
            if(selected_h){
                //Decr Amount/Destination
                if(matrix_pos_h == 1){
                    decItemAmount(matrix, matrix_pos_v);
                    menuTFTPrintMatrixAmount(matrix, matrix_pos_v);
                } 
                if(matrix_pos_h == 2){
                    dst_changed = 1;
                    decDestination(matrix, matrix_pos_v);
                    menuTFTPrintMatrixDestination(matrix, matrix_pos_v);
                    menuTFTPrintMatrixAmount(matrix, matrix_pos_v);
                } 

               //Prepare & send event to audio thread
                if(matrix_pos_h != 2 || (matrix[matrix_pos_v].dst != MTX_V0_PITCH && matrix[matrix_pos_v].dst != MTX_V1_PITCH)){
                    if(!((matrix_pos_h == 1) && (matrix[matrix_pos_v].dst == MTX_NONE))){
                        if(dst_changed){
                            matrix_event.changed_param = matrix[matrix_pos_v].dst + 1;
                            matrix_event.source = matrix_pos_v;
                            matrix_event.amount = 0;
                            dst_changed = 0;
                            xQueueSend(m_event_queue, (void*) &matrix_event, portMAX_DELAY); 
                        }
                        matrix_event.changed_param = matrix[matrix_pos_v].dst;
                        matrix_event.source = matrix_pos_v;
                        matrix_event.amount = matrix[matrix_pos_v].amt;
                        xQueueSend(m_event_queue, (void*) &matrix_event, portMAX_DELAY);
                    }
                }
            }else{
                if(selected_v){
                    //Moving horizontally
                    matrix_pos_h--;
                    if(matrix_pos_h < 0) matrix_pos_h = items_h - 1;
                    menuTFTSelectMatrixItem(matrix_pos_v,0,matrix_pos_h);
                }else{
                    //Moving vertically
                    matrix_pos_v--;
                    if(matrix_pos_v < 0) matrix_pos_v = items_v - 1;
                    menuTFTSelectMatrixItem(matrix_pos_v,0,matrix_pos_h);
                }
            }
            break;
        case EV_SHORT_PRESS:
            if(!selected_v){
                selected_v = 1;
                menuTFTPrintMatrixRowIndicator(selected_v, matrix_pos_v);
            }else if(selected_v){
                if(matrix_pos_h > 0){
                    selected_h = !selected_h;
                }else{
                    selected_v = 0;
                    menuTFTPrintMatrixRowIndicator(selected_v, matrix_pos_v);
                }
                menuTFTSelectMatrixItem(matrix_pos_v, selected_h, matrix_pos_h);
            }
            break;
        case EV_LONG_PRESS:
            matrix_pos_v = 0;
            matrix_pos_h = 0;
            cur_destination = 0;
            return M_PLAY;
            break;
        default:
            break;
    }
    return 0;
}

static int effects_def_handler(int it_id, int event, void* event_data){
    const int menu_states[] = {M_DELAY, M_EXTERNAL_IN};
    const int states = sizeof(menu_states)/sizeof(int);
    static int menu_state_current = 0; 
    
    switch(event){

        case EV_ENTERED_MENU:
            menuTFTPrintMenu(effect_menus, &n_effect_menus);
            menuTFTSelectMenuItem(&menu_state_current, 0, effect_menus, &n_effect_menus);
            break;
        case EV_FWD:
            menu_state_current++;
            if(menu_state_current >= states) menu_state_current = 0;
            menuTFTSelectMenuItem(&menu_state_current, 0, effect_menus, &n_effect_menus);
            break;
        case EV_BWD:
            menu_state_current--;
            if(menu_state_current < 0) menu_state_current = states - 1;
            menuTFTSelectMenuItem(&menu_state_current, 0, effect_menus, &n_effect_menus);
            break;
        case EV_SHORT_PRESS:
            return menu_states[menu_state_current];
            break;
        case EV_LONG_PRESS:
            menu_state_current = 0;
            return M_PLAY;
            break;
        default:
            break;
    }
    return 0;
}

static int extin_def_handler(int it_id, int event, void* event_data){
    const int menu_items[] = {SID_EXTIN_ACTIVE, SID_EXTIN_VOLUME, SID_EXTIN_PAN, SID_EXTIN_DELAY_SEND};
    const int items = sizeof(menu_items)/sizeof(int);
    static int menu_pos = 0; 
    static int selected = 0;
    ext_in_data_t* extin = &effectData.extInData;

    switch(event){
        case EV_ENTERED_MENU:
            menuTFTPrintMenu(externel_in_menus, &n_externel_in_menus);
            menuTFTSelectMenuItem(&menu_pos, selected, externel_in_menus, &n_externel_in_menus);
            menuTFTPrintExtInValues(extin, PRINT_ALL);
            break;
        case EV_FWD:
            if(!selected){
                menu_pos++;
                if(menu_pos >= items) menu_pos = 0;
                menuTFTSelectMenuItem(&menu_pos, selected, externel_in_menus, &n_externel_in_menus);
            }else{
                incExtInValue(extin, menu_items[menu_pos]);
                menuTFTPrintExtInValues(extin, menu_items[menu_pos]);
                xQueueSend(eff_queue, &effectData, portMAX_DELAY);
                
            }
            break;
        case EV_BWD:
            if(!selected){
                menu_pos--;
                if(menu_pos < 0) menu_pos = items - 1;
                menuTFTSelectMenuItem(&menu_pos, selected, externel_in_menus, &n_externel_in_menus);
            }else{
                decExtInValue(extin, menu_items[menu_pos]);
                menuTFTPrintExtInValues(extin, menu_items[menu_pos]);
                xQueueSend(eff_queue, &effectData, portMAX_DELAY);
            }
            break;
        case EV_SHORT_PRESS:
            if(!selected){
                selected = 1;
                menuTFTSelectMenuItem(&menu_pos, selected, externel_in_menus, &n_externel_in_menus);
            }else{
                selected = 0;
                menuTFTSelectMenuItem(&menu_pos, selected, externel_in_menus, &n_externel_in_menus);
            }
            
            break;
        case EV_LONG_PRESS:
            selected = 0;
            menu_pos = 0;
            return M_EFFECTS;
            break;
        default:
            break;
    }

    return 0;

}

static int delay_def_handler(int it_id, int event, void* event_data){
    const int menu_items[] = {SID_DELAY_ACTIVE, SID_DELAY_MODE, SID_DELAY_TIME, SID_DELAY_PAN, SID_DELAY_FEEDBACK, SID_DELAY_VOLUME};
    const int items = sizeof(menu_items)/sizeof(int);
    static int menu_pos = 0; 
    static int selected = 0;
    delay_data_t* delay = &effectData.delay;

    switch(event){
        case EV_ENTERED_MENU:
            menuTFTPrintMenu(delay_menus, &n_delay_menus);
            menuTFTSelectMenuItem(&menu_pos, selected, delay_menus, &n_delay_menus);
            menuTFTPrintDelayValues(delay, PRINT_ALL);
            break;
        case EV_FWD:
            if(!selected){
                menu_pos++;
                if(menu_pos >= items) menu_pos = 0;
                menuTFTSelectMenuItem(&menu_pos, selected, delay_menus, &n_delay_menus);
            }else{
                incDelayValue(delay, menu_items[menu_pos]);
                menuTFTPrintDelayValues(delay, menu_items[menu_pos]);
                xQueueSend(eff_queue, &effectData, portMAX_DELAY);
                
            }

            break;
        case EV_BWD:
            if(!selected){
                menu_pos--;
                if(menu_pos < 0) menu_pos = items - 1;
                menuTFTSelectMenuItem(&menu_pos, selected, delay_menus, &n_delay_menus);
            }else{
                decDelayValue(delay, menu_items[menu_pos]);
                menuTFTPrintDelayValues(delay, menu_items[menu_pos]);
                xQueueSend(eff_queue, &effectData, portMAX_DELAY);
            }
            break;
        case EV_SHORT_PRESS:
            if(!selected){
                selected = 1;
                menuTFTSelectMenuItem(&menu_pos, selected, delay_menus, &n_delay_menus);
            }else{
                selected = 0;
                menuTFTSelectMenuItem(&menu_pos, selected, delay_menus, &n_delay_menus);
            }
            
            break;
        case EV_LONG_PRESS:
            selected = 0;
            menu_pos = 0;
            return M_EFFECTS;
            break;
        default:
            break;
    }

    return 0;

}

static int bank_def_handler(int it_id, int event, void* event_data){
    static int activeSlot = 0;
    static cJSON *cfgData = NULL;
    switch(event){
        case EV_ENTERED_MENU:
            cfgData = readJSONFileAsCJSON("/sdcard/CONFIG.JSN");
            menuTFTPrintSlotMenu(cfgData, activeSlot);
            cJSON_Delete(cfgData);
            break;
        case EV_FWD:
            activeSlot++; 
            activeSlot %= 2;
            menuTFTHighlightNextEl();
            break;
        case EV_BWD:
            activeSlot--; 
            activeSlot = abs(activeSlot) % 2;
            menuTFTHighlightPrevEl();
            break;
        case EV_SHORT_PRESS:
            _state_data = (void*)activeSlot;
            return M_SLOT_TYPESELECT;
            break;
        case EV_LONG_PRESS:
            assignAudioFiles();
            menuTFTFlushMenuDataRect();
            return M_MAIN;
            break;
        default:
            break;
    }

    return 0; // remain in current menu
}

static int filebrowser_def_handler(int it_id, int event, void* event_data){
    static list_t *file_list;
    static int currentFile = 0, activeSlot = 0;
    static cJSON *info = NULL, *cfgData, *slots, *slotObject;
    char buf[64], *id;
    
    switch(event){
        case EV_ENTERED_MENU:
            file_list = list_create();
            getFilesInDir(file_list, "/sdcard/POOL", ".MP3");
            // check if sound file pool is empty
            if(file_list->count == 0){
                ESP_LOGW("MENU", "No files in pool to browse");
                list_free(file_list);
                return M_SLOT_TYPESELECT;
            }
            activeSlot = (int) _state_data;
            if(file_list->count == 0) break;
            cfgData = readJSONFileAsCJSON("/sdcard/CONFIG.JSN");
            slots = cJSON_GetObjectItem(cfgData, "slots");
            slotObject = cJSON_GetArrayItem(slots, activeSlot);
            currentFile = list_find_element(file_list, (void*)cJSON_GetObjectItemCaseSensitive(slotObject, "name")->valuestring, f_compare_strings);
            if(currentFile == -1) currentFile = 0;
            id = (char*)list_get_item(file_list, currentFile)->value;
            snprintf(buf, 64, "/sdcard/POOL/%s.JSN", id);
            info = readJSONFileAsCJSON(buf);
            parseJSONAudioTags(info);
            menuTFTPrintFileBrowser(currentFile, file_list->count, info);
            break;
        case EV_FWD:
            currentFile++;
            if(currentFile == file_list->count) currentFile = 0;
            cJSON_Delete(info);
            id = (char*)list_get_item(file_list, currentFile)->value;
            snprintf(buf, 64, "/sdcard/POOL/%s.JSN", id);
            info = readJSONFileAsCJSON(buf);
            parseJSONAudioTags(info);
            menuTFTPrintFileBrowser(currentFile, file_list->count, info);
            break;
        case EV_BWD:
            currentFile--;
            if(currentFile < 0) currentFile = file_list->count - 1;
            cJSON_Delete(info);
            id = (char*)list_get_item(file_list, currentFile)->value;
            snprintf(buf, 64, "/sdcard/POOL/%s.JSN", id);
            info = readJSONFileAsCJSON(buf);
            parseJSONAudioTags(info);
            menuTFTPrintFileBrowser(currentFile, file_list->count, info);
            break;
        case EV_SHORT_PRESS:
            cJSON_Delete(info);
            id = (char*)list_get_item(file_list, currentFile)->value;
            snprintf(buf, 64, "/RAW/%s.RAW", id);
            cJSON_ReplaceItemInObjectCaseSensitive(slotObject, "name", cJSON_CreateString(id));
            cJSON_ReplaceItemInObjectCaseSensitive(slotObject, "file", cJSON_CreateString(buf));
            //cJSON_ReplaceItemInArray(slots, (int)_state_data, cJSON_CreateString(list_get_item(file_list, currentFile)->value));
            writeJSONFile("/sdcard/CONFIG.JSN", cJSON_Print(cfgData));
            cJSON_Delete(cfgData);
            snprintf(buf, 64, "/sdcard/RAW/%s.RAW", id);
            if(fileExists(buf) != 0){
                list_free(file_list);
                return M_SLOT;
            }
            _state_data = calloc(1, strlen(id) + 1);
            memcpy(_state_data, id, strlen(id));
            list_free(file_list);
            return M_SLOT_DECODING;
            break;
        /*
        case EV_LONG_PRESS:
            cJSON_Delete(info);
            list_free(file_list);
            return M_MAIN;
            break;
            */
        case EV_TIMER_REPEATING_FAST:
            if(info)menuTFTAnimateFileBrowser(info);
            break;
        default:
            break;
    }

    return 0; // remain in current menu
}

static int userfilebrowser_def_handler(int it_id, int event, void* event_data){
    static list_t *file_list;
    static int currentFile = 0, activeSlot = 0;
    static cJSON *info = NULL, *cfgData, *slots, *slotObject;
    char buf[64], *name;

    switch(event){
        case EV_ENTERED_MENU:
            file_list = list_create();
            getFilesInDir(file_list, "/sdcard/usr", ".RAW");
            // check if sound file pool is empty
            if(file_list->count == 0){
                ESP_LOGW("MENU", "No files in usr to browse");
                list_free(file_list);  
                return M_SLOT_TYPESELECT;
            }
            activeSlot = (int) _state_data;
            if(file_list->count == 0) break;
            cfgData = readJSONFileAsCJSON("/sdcard/CONFIG.JSN");
            slots = cJSON_GetObjectItem(cfgData, "slots");
            slotObject = cJSON_GetArrayItem(slots, activeSlot);
            currentFile = list_find_element(file_list, (void*)cJSON_GetObjectItemCaseSensitive(slotObject, "name")->valuestring, f_compare_strings);
            if(currentFile == -1) currentFile = 0;
            name = (char*)list_get_item(file_list, currentFile)->value;
            snprintf(buf, 64, "/sdcard/usr/%s.JSN", name);
            //ESP_LOGI("Menu", "jsn file %s", buf);
            info = readJSONFileAsCJSON(buf);
            //ESP_LOGI("Menu", "JSON String %s", cJSON_Print(info));
            menuTFTPrintFileBrowser(currentFile, file_list->count, info);
            break;
        case EV_TIMER_REPEATING_FAST:
            if(info)menuTFTAnimateFileBrowser(info);
            break;
        case EV_FWD:
            currentFile++;
            if(currentFile == file_list->count) currentFile = 0;
            cJSON_Delete(info);
            name = (char*)list_get_item(file_list, currentFile)->value;
            snprintf(buf, 64, "/sdcard/usr/%s.JSN", name);
            info = readJSONFileAsCJSON(buf);
            menuTFTPrintFileBrowser(currentFile, file_list->count, info);
            break;
        case EV_BWD:
            currentFile--;
            if(currentFile < 0) currentFile = file_list->count - 1;
            cJSON_Delete(info);
            name = (char*)list_get_item(file_list, currentFile)->value;
            snprintf(buf, 64, "/sdcard/usr/%s.JSN", name);
            info = readJSONFileAsCJSON(buf);
            menuTFTPrintFileBrowser(currentFile, file_list->count, info);
            break;
        case EV_SHORT_PRESS:
            name = (char*)list_get_item(file_list, currentFile)->value;
            snprintf(buf, 64, "/USR/%s.RAW", name);
            cJSON_ReplaceItemInObjectCaseSensitive(slotObject, "name", cJSON_CreateString(name));
            cJSON_ReplaceItemInObjectCaseSensitive(slotObject, "file", cJSON_CreateString(buf));
            //cJSON_ReplaceItemInArray(slots, (int)_state_data, cJSON_CreateString(list_get_item(file_list, currentFile)->value));
            writeJSONFile("/sdcard/CONFIG.JSN", cJSON_Print(cfgData));
            cJSON_Delete(info);
            cJSON_Delete(cfgData);
            list_free(file_list);
            menuTFTFlushMenuDataRect();
            return M_SLOT;
            break;
        default:
            break;
    }
    /*   
    switch(event){
        case EV_ENTERED_MENU:
            file_list = list_create();
            getFilesInDir(file_list, "/sdcard/usr", ".RAW");
            activeSlot = (int) _state_data;
            if(file_list->count == 0) break;
            cfgData = readJSONFileAsCJSON("/sdcard/CONFIG.JSN");
            slots = cJSON_GetObjectItem(cfgData, "slots");
            currentFile = list_find_element(file_list, (void*)cJSON_GetArrayItem(slots, activeSlot)->valuestring, f_compare_strings);
            if(currentFile == -1) currentFile = 0;
            id = (char*)list_get_item(file_list, currentFile)->value;
            snprintf(buf, 64, "/sdcard/POOL/%s.JSN", id);
            info = readJSONFileAsCJSON(buf);
            parseJSONAudioTags(info);
            menuTFTPrintFileBrowser(currentFile, file_list->count, info);
            break;
        case EV_FWD:
            currentFile++;
            if(currentFile == file_list->count) currentFile = 0;
            cJSON_Delete(info);
            id = (char*)list_get_item(file_list, currentFile)->value;
            snprintf(buf, 64, "/sdcard/POOL/%s.JSN", id);
            info = readJSONFileAsCJSON(buf);
            parseJSONAudioTags(info);
            menuTFTPrintFileBrowser(currentFile, file_list->count, info);
            break;
        case EV_BWD:
            currentFile--;
            if(currentFile < 0) currentFile = file_list->count - 1;
            cJSON_Delete(info);
            id = (char*)list_get_item(file_list, currentFile)->value;
            snprintf(buf, 64, "/sdcard/POOL/%s.JSN", id);
            info = readJSONFileAsCJSON(buf);
            parseJSONAudioTags(info);
            menuTFTPrintFileBrowser(currentFile, file_list->count, info);
            break;
        case EV_SHORT_PRESS:
            cJSON_Delete(info);
            cJSON_ReplaceItemInArray(slots, (int)_state_data, cJSON_CreateString(list_get_item(file_list, currentFile)->value));
            writeJSONFile("/sdcard/CONFIG.JSN", cJSON_Print(cfgData));
            cJSON_Delete(cfgData);
            id = (char*)list_get_item(file_list, currentFile)->value;
            snprintf(buf, 64, "/sdcard/RAW/%s.RAW", id);
            if(fileExists(buf) != 0){
                list_free(file_list);
                return M_SLOT;
            }
            _state_data = calloc(1, strlen(id) + 1);
            memcpy(_state_data, id, strlen(id));
            list_free(file_list);
            return M_SLOT_DECODING;
            break;
        case EV_TIMER_REPEATING_FAST:
            menuTFTAnimateFileBrowser(info);
            break;
        default:
            break;
    }
    */


    return 0; // remain in current menu
}

static int decoding_def_handler(int it_id, int event, void* event_data){
    static char *id; 
    switch(event){
        case EV_ENTERED_MENU:
            id = (char*) _state_data;
            menuTFTPrintDecoding();
            menuTFTUpdateProgress("Decoding", 0);
            decodeMP3File(id);
            break;
        case EV_DECODING_PROGRESS:
            menuTFTUpdateProgress("Decoding", (int) event_data);
            break;
        case EV_DECODING_DONE:
            free(_state_data);
            return M_SLOT;
            break;
        default:
            break;
    }

    return 0; // remain in current menu
}

static int browse_user_def_handler(int it_id, int event, void* event_data){
    switch(event){
        case EV_ENTERED_MENU:
            menuTFTPrintUserFileMenu();
            setRestAPIUserReceiveOn();
            break;
        case EV_DECODING_PROGRESS:
            menuTFTUpdateProgress("User upload", (int)event_data);
            break;
        case EV_LONG_PRESS:
            setRestAPIUserReceiveOff();
            menuTFTFlushMenuDataRect();
            return M_MAIN;
        case EV_DECODING_DONE:
            break;
        default:
            break;
    }

    return 0; // remain in current menu
}

static int browse_tag_def_handler(int it_id, int event, void* event_data){
    static list_t *tag_list = NULL;
    static bool isBusy = false;
    static uint32_t item = 0;
    static list_t *selected_tags_list = NULL;
    char *buf = NULL;

    switch(event){
        case EV_ENTERED_MENU:
            isBusy = true;
            menuTFTPrintLoadingTagMenu();
            freesoundGetTags("");
            selected_tags_list = list_create();
            break;
        case EV_PROGRESS_UPDATE:
            menuTFTUpdateProgress("Download", (int)event_data);
            break;
        case EV_FREESND_TAGLIST:
            tag_list = (list_t*)event_data;
            menuTFTPrintBrowseTagList(tag_list);
            isBusy = false;
            break;
        case EV_FWD:
            if(tag_list == NULL) break;
            item = menuTFTHighlightNextEl();
            break;
        case EV_BWD:
            if(tag_list == NULL) break;
            item = menuTFTHighlightPrevEl();
            break;
        case EV_SHORT_PRESS:
            if(isBusy) break;
            char *tag = (char*)list_get_item(tag_list, item)->value;
            buf = calloc(strlen(tag)+1, 1);
            strcpy(buf, tag);
            _state_data = (void*)buf; // used in next state
            return M_BROWSE_TAG_RESULTS;
            break;
        case EV_LONG_PRESS:
            if(isBusy) break;
            tag_list = NULL;
            list_free(selected_tags_list);
            menuTFTFlushMenuDataRect();
            return M_MAIN;
            break;
        default:
            break;
    }

    return 0; // remain in current menu
}

static int browse_tag_results_def_handler(int it_id, int event, void* event_data){

    switch(event){
        case EV_ENTERED_MENU:
            freesoundSearch((const char*)_state_data);
            break;
        case EV_PROGRESS_UPDATE:

            break;
        case EV_LONG_PRESS:
            free(_state_data);
            menuTFTFlushMenuDataRect();
            return M_MAIN;
            break;
        default:
            break;
    }

    return 0; // remain in current menu
}

static int browse_search_def_handler(int it_id, int event, void* event_data){
    
    switch(event){
        case EV_ENTERED_MENU:
            menuTFTPrintBrowseTextMenu();
            break;
        case EV_LONG_PRESS:
            return M_BROWSE;
            break;
        default:
            break;
    }

    return 0; // remain in current menu
}

static int browse_id_res_def_handler(int it_id, int event, void* event_data){
    static int isBusy = 1;
    char *id = (char*) _state_data;
    switch(event){
        case EV_ENTERED_MENU:
            isBusy = 1;
            freesoundGetInstance(id);
            break;
        case EV_PROGRESS_UPDATE:
            menuTFTUpdateProgress("Download", (int)event_data);
            break;
        case EV_FREESND_MP3_COMPLETE:
        case EV_FREESND_NOT_FOUND:
            // a ui message would make sense here
            isBusy = false;
        case EV_LONG_PRESS:
            if(isBusy) break;
            free(_state_data);
            menuTFTFlushMenuDataRect();
            return M_MAIN;
            break;
        default:
            break;
    }

    return 0; // remain in current menu
}

static int browse_id_def_handler(int it_id, int event, void* event_data){
    const char *c_list = "=0123456789<";
    static int pos = 0, c = 1;
    static char num_s[10];
    char *buf;
    
    switch(event){
        case EV_ENTERED_MENU:
            bzero(num_s, 10);
            pos = 0;
            c = 1;
            menuTFTPrintSelectIDMenu();
            menuTFTPrintChar(num_s, pos, c_list[c], PRINT_NORM);
        case EV_FWD:
            c++;
            if(c>11)c=11;
            menuTFTPrintChar(num_s, pos, c_list[c], PRINT_NORM);
            break;
        case EV_BWD:
            c--;
            if(c<0)c=0;
            menuTFTPrintChar(num_s, pos, c_list[c], PRINT_NORM);
            break;
        case EV_SHORT_PRESS:
            switch(c_list[c]){
                case '<':
                    num_s[pos] = '\0';
                    pos--;
                    if(pos<0)pos=0;
                    menuTFTPrintChar(num_s, pos, c_list[c], PRINT_NORM);
                    break;
                case '=':
                    if(pos == 0) break;
                    num_s[pos] = '\0';
                    buf = calloc(strlen(num_s) + 1, 1);
                    strcpy(buf, num_s);
                    _state_data = buf;
                    return M_BROWSE_ID_RESULT;
                    break;
                default:
                    num_s[pos] = c_list[c];
                    pos++;
                    if(pos>10)pos=10;
                    menuTFTPrintChar(num_s, pos, c_list[c], PRINT_NORM);
                    break;
            }
            break;
        case EV_LONG_PRESS:
            return M_BROWSE;
            break;
        default:
            break;
    }

    return 0; // remain in current menu
}

static int preset_def_handler(int it_id, int event, void* event_data){
    const int menu_states[] = {M_PRESET_P, M_PRESET_B};
    const int states = sizeof(menu_states)/sizeof(int);
    static int menu_state_current = 0;
    
    switch(event){
        case EV_ENTERED_MENU:
            //ESP_LOGI("PRESET", "Entered preset top level menu");
            menuTFTPrintMenuH(preset_menus, &n_preset_menus);
            menuTFTSelectMenuItemH(&menu_state_current, 0, preset_menus, &n_preset_menus);
            break;
        case EV_FWD:
            menu_state_current++;
            if(menu_state_current >= states) menu_state_current = 0;
            menuTFTSelectMenuItemH(&menu_state_current, 0, preset_menus, &n_preset_menus);
            break;
        case EV_BWD:
            menu_state_current--;
            if(menu_state_current < 0) menu_state_current = states - 1;
            menuTFTSelectMenuItemH(&menu_state_current, 0, preset_menus, &n_preset_menus);
            break;
        case EV_SHORT_PRESS:
            return menu_states[menu_state_current];
            break;
        case EV_LONG_PRESS:
            menu_state_current = 0;
            savePresetConfig(current_preset_name, current_bank);
            menuTFTFlushMenuDataRect();
            return M_MAIN;
            break;
        default:
            break;
    }
    
    return 0; // remain in current menu
}

static int typeselect_def_handler(int it_id, int event, void* event_data){
    const int menu_states[] = {M_SLOT_FILEBROWSER, M_SLOT_USER};
    const int states = sizeof(menu_states)/sizeof(int);
    static int menu_state_current = 0;
    
    switch(event){
        case EV_ENTERED_MENU:
            //ESP_LOGI("PRESET", "Entered preset top level menu");
            menuTFTPrintMenuH(slot_menus, &n_slot_menus);
            menuTFTSelectMenuItemH(&menu_state_current, 0, slot_menus, &n_slot_menus);
            break;
        case EV_FWD:
            menu_state_current++;
            if(menu_state_current >= states) menu_state_current = 0;
            menuTFTSelectMenuItemH(&menu_state_current, 0, slot_menus, &n_slot_menus);
            break;
        case EV_BWD:
            menu_state_current--;
            if(menu_state_current < 0) menu_state_current = states - 1;
            menuTFTSelectMenuItemH(&menu_state_current, 0, slot_menus, &n_slot_menus);
            break;
        case EV_SHORT_PRESS:
            return menu_states[menu_state_current];
            break;
        case EV_LONG_PRESS:
            menu_state_current = 0;
            menuTFTFlushMenuDataRect();
            return M_SLOT;
            break;
        default:
            break;
    }
    
    return 0; // remain in current menu
}

static int presets_menu_def_handler(int it_id, int event, void* event_data){
    const int menu_states[] = {M_PRESET_LOAD, M_PRESET_STORE, M_PRESET_NEW, M_PRESET_DELETE, M_PRESET_RESET};
    const int states = sizeof(menu_states)/sizeof(int);
    static int menu_state_current = 0, list_pos = 0, selected = 0, submenu_active = 0;
    static cJSON *root = NULL, *bank = NULL;

    switch(event){
        case EV_ENTERED_MENU:
            selected = 0;
            current_fb_pos = 0;
            menuTFTPrintMenuHSpaced(preset_choices, &n_preset_choices);
            menuTFTSelectMenuItemHSpaced(&menu_state_current, selected, preset_choices, &n_preset_choices);
            menuTFTPrintCurrentPresetSettings("Current preset: ", current_preset_name);
            break;
        case EV_TIMER_COMPLETE:
            if(root == NULL){
                root = getBankRoot(current_bank); 
                if(root != NULL)bank = cJSON_GetObjectItemCaseSensitive(root, "bank");
                else menuTFTPrintInputError("Error reading bank file");
                menuTFTPrintPresetList(bank, PRINT_FAST);
            }
            break;
        case EV_FWD:
            if(selected){
                //In list selection
                list_pos++;
                if(list_pos >= cJSON_GetArraySize(bank)) list_pos = 0;
                menuTFTSelectPreset(&list_pos, bank, selected);
            }else{
                //In topmenu selection
                menu_state_current++;
                if(menu_state_current >= states) menu_state_current = 0;
                menuTFTSelectMenuItemHSpaced(&menu_state_current, 0, preset_choices, &n_preset_choices);
            }
            break;
        case EV_BWD:
            if(selected){
                //In list selection
                list_pos--;
                if(list_pos < 0) list_pos = cJSON_GetArraySize(bank) - 1;
                menuTFTSelectPreset(&list_pos, bank, selected);
            }else{
                //In topmenu selection
                menu_state_current--;
                if(menu_state_current < 0) menu_state_current = states - 1;
                menuTFTSelectMenuItemHSpaced(&menu_state_current, 0, preset_choices, &n_preset_choices);
            }
            break;
        case EV_SHORT_PRESS:
            switch(menu_states[menu_state_current]){
                case M_PRESET_NEW:
                    //Selected new bank menu, prepare and return new menu state
                    menu_state_current = 0;
                    _state_json = (void*) root;
                    root = NULL;
                    bank = NULL;
                    return M_PRESET_NEW;
                    break;
                case M_PRESET_RESET:
                    current_fb_pos = menu_state_current;
                    //Reset current settings
                    initParams();
                    //Display feedback
                    fb_state = MS_SUCCESS;
                    setTimerTopmenuFeedback(250, ui_ev_queue);
                    break;
                default:
                    //Menustate with "submenu"
                    if(!selected){
                        //Selecting top menu functionality
                        selected = 1;
                        if(!submenu_active)submenu_active = 1;
                        menuTFTSelectMenuItemHSpaced(&menu_state_current, selected, preset_choices, &n_preset_choices);
                        //Print preset selection
                        menuTFTSelectPreset(&list_pos, bank, selected);
                    }else if(selected && submenu_active){   //in submenu
                        current_fb_pos = menu_state_current;
                        switch (menu_states[menu_state_current])
                        {
                            case M_PRESET_LOAD:
                                //Load preset from bank
                                loadParams(cJSON_GetArrayItem(bank, list_pos));
                                //Display feedback
                                fb_state = MS_SUCCESS;
                                setTimerTopmenuFeedback(250, ui_ev_queue);
                                menuTFTPrintCurrentPresetSettings("Current preset: ", current_preset_name);  
                                break;
                            case M_PRESET_DELETE:
                                //Delete preset from bank
                                if(strcasecmp(current_preset_name, getPresetNameAtIndex(bank, &list_pos)) != 0){
                                    //Display feedback
                                    fb_state = MS_SUCCESS;
                                    //Preset to delete is not current preset
                                    deletePreset(bank, &list_pos);
                                    //Save to bank
                                    saveBank(root, current_bank, NO_ACTION);
                                    //Clearing list item
                                    if(list_pos != cJSON_GetArraySize(bank)){
                                        //deleted item not last in list, refresh
                                        menuTFTPrintPresetList(bank, PRINT_CLEAR);
                                    }else menuTFTClearListItem(&list_pos); //clear last item

                                    if(list_pos > 0) list_pos--;
                                    menuTFTSelectPreset(&list_pos, bank, selected);
                                }else{
                                    fb_state = MS_ERROR;
                                    ESP_LOGE("PRESET", "Cannot delete current loaded preset.");
                                }
                                setTimerTopmenuFeedback(250, ui_ev_queue);
                                break;
                            case M_PRESET_STORE: ;
                                //Save current settings to preset at index list_pos
                                cJSON* preset = buildPreset(getPresetNameAtIndex(bank, &list_pos));
                                cJSON_ReplaceItemInArray(bank, list_pos, preset);
                                //Display feedback
                                fb_state = MS_SUCCESS;
                                //Save bank file
                                saveBank(root, current_bank, NO_ACTION); 
                                setTimerTopmenuFeedback(250, ui_ev_queue);
                                break;
                            default:
                                break;
                            }
                    }
                    break;
                }
            break;
        case EV_LONG_PRESS:
            if(!selected){
                //exiting menu
                menu_state_current = 0;
                list_pos = 0;
                cJSON_Delete(root);
                _state_json = NULL;
                root = NULL;
                bank = NULL;
                menuTFTFlushMenuDataRect();
                return M_PRESET;
            }else{
                //Deselect
                selected = 0;
                list_pos = 0;
                if(submenu_active) submenu_active = 0;
                menuTFTSelectPreset(&list_pos, bank, selected);
                menuTFTSelectMenuItemHSpaced(&menu_state_current, selected, preset_choices, &n_preset_choices);
            }
            break;
        case EV_TIMER_MENU_FB:
            //Display feedback when timer triggered
            if(event_data != NULL)menuTFTFeedbackMenuItemHSpaced((int*) event_data, &current_fb_pos, preset_choices, &n_preset_choices, fb_state);
            else ESP_LOGE("UI", "Error displaying feedback");
            break;
        default:
            break;
    }
    
    return 0; // remain in current menu  
}

static int banks_menu_def_handler(int it_id, int event, void* event_data){
    const int menu_states[] = {M_PRESET_BANK_LOAD, M_PRESET_BANK_NEW, M_PRESET_BANK_DELETE};
    const int states = sizeof(menu_states)/sizeof(int);
    static int menu_state_current = 0, list_pos = 0, selected = 0, submenu_active = 0;
    static list_t* file_list = NULL;

    switch(event){
        case EV_ENTERED_MENU:
            selected = 0;
            current_fb_pos = 0;
            menuTFTPrintMenuHSpaced(bank_choices, &n_bank_choices);
            menuTFTSelectMenuItemHSpaced(&menu_state_current, 0, bank_choices, &n_bank_choices);
            menuTFTPrintCurrentPresetSettings("Current bank: ", current_bank);
            break;
        case EV_TIMER_COMPLETE:
            if(file_list == NULL){
                file_list = list_create();
                getFilesInDir(file_list, "/sdcard/banks", ".JSN");
                menuTFTPrintBankList(file_list, PRINT_FAST);
            }
            break;
        case EV_FWD:
            if(selected){
                //In list selection
                list_pos++;
                if(list_pos >= file_list->count) list_pos = 0;
                menuTFTSelectPresetBank(&list_pos, file_list,selected);
                break;
            }
            //In topmenu selection
            menu_state_current++;
            if(menu_state_current >= states) menu_state_current = 0;
            menuTFTSelectMenuItemHSpaced(&menu_state_current, 0, bank_choices, &n_bank_choices);
            break;
        case EV_BWD:
            if(selected){
                //In list selection
                list_pos--;
                if(list_pos < 0) list_pos = file_list->count - 1;
                menuTFTSelectPresetBank(&list_pos, file_list, selected);
                break;
            }
            //In topmenu selection
            menu_state_current--;
            if(menu_state_current < 0) menu_state_current = states - 1;
            menuTFTSelectMenuItemHSpaced(&menu_state_current, 0, bank_choices, &n_bank_choices);
            break;
        case EV_SHORT_PRESS:
            if(menu_states[menu_state_current] == M_PRESET_BANK_NEW){
                //Selected new bank menu, prepare and return new menu state
                _state_data = (void*) file_list;
                menu_state_current = 0;
                list_pos = 0;   
                file_list = NULL;
                return M_PRESET_BANK_NEW;
            }else if(!selected){
                //Selecting top menu functionality
                selected = 1;
                if(!submenu_active) submenu_active = 1;
                menuTFTSelectMenuItemHSpaced(&menu_state_current, selected, bank_choices, &n_bank_choices);
                //Print bank selection
                menuTFTSelectPresetBank(&list_pos, file_list, selected);
            }else if(selected && submenu_active){   //in submenu
                current_fb_pos = menu_state_current;
                switch (menu_states[menu_state_current]){
                    case M_PRESET_BANK_LOAD:
                        //Loading bank and preset at index 0
                        memset(current_bank, 0, sizeof(current_bank)/sizeof(char));
                        strcpy(current_bank, (char*)list_get_item(file_list, list_pos)->value);
                        cJSON* root = getBankRoot(current_bank);
                        cJSON* bank = cJSON_GetObjectItemCaseSensitive(root, "bank");
                        cJSON* preset = cJSON_GetArrayItem(bank, 0);
                        loadParams(preset);
                        fb_state = MS_SUCCESS;
                        cJSON_Delete(root);
                        setTimerTopmenuFeedback(250, ui_ev_queue); 
                        menuTFTPrintCurrentPresetSettings("Current bank: ", current_bank);
                        break;
                    case M_PRESET_BANK_DELETE: ;
                        //Delete preset from bank
                        char* bn;
                        bn = (char*)list_get_item(file_list, list_pos)->value;
                        if(strcmp(bn, current_bank) != 0){
                            //currentbank != bank to delete
                            deleteBank(bn);
                            fb_state = MS_SUCCESS;
                            //free old list
                            list_free(file_list);
                            //create updated list
                            file_list = list_create();
                            getFilesInDir(file_list, "/sdcard/banks", ".JSN");
                            //clear deleted bank from list
                            if(list_pos != file_list->count){
                                //deleted item not last in list, refresh
                                menuTFTPrintBankList(file_list, PRINT_CLEAR);
                            }else menuTFTClearListItem(&list_pos); //clear last item
                                 
                            if(list_pos > 0) list_pos--;
                            menuTFTSelectPresetBank(&list_pos, file_list, selected);
                        }else{
                            //currentbank == bank to delete, issue error
                            fb_state = MS_ERROR;
                            ESP_LOGE("UI", "cannot delete currently loaded bank");
                        }
                        setTimerTopmenuFeedback(250, ui_ev_queue); 
                        break;
                    default:
                        break;
                }
            }
            break;
        case EV_LONG_PRESS:
            if(!selected){
                menu_state_current = 0;
                list_pos = 0;
                if(file_list != NULL)list_free(file_list);
                file_list = NULL;
                _state_data = NULL;
                menuTFTFlushMenuDataRect();
                return M_PRESET;
            }else{
                //Deselect
                selected = 0;
                list_pos = 0;
                if(submenu_active) submenu_active = 0;
                menuTFTSelectPresetBank(&list_pos, file_list, selected);
                menuTFTSelectMenuItemHSpaced(&menu_state_current, selected, bank_choices, &n_bank_choices);
            }
            break;
        case EV_TIMER_MENU_FB:
            //Display feedback when timer is triggered
            if(event_data != NULL)menuTFTFeedbackMenuItemHSpaced((int*) event_data, &current_fb_pos, bank_choices, &n_bank_choices, fb_state);
            else ESP_LOGE("UI", "Error displaying feedback");
        default:
            break;
    }
    
    return 0; // remain in current menu  
}

static int preset_new_def_handler(int it_id, int event, void* event_data){
    const char *c_list = "=0123456789abcdefghijklmnopqrstuvwxyz<";
    static int pos = 0, c = 1;
    static char name[12];
    char *buf;
    char nbuf[64];
    static cJSON *root, *bank, *preset;
    switch(event){
        case EV_ENTERED_MENU:
            bzero(name, 12);
            pos = 0;
            c = 1;
            menuTFTResetTextWrap();
            if(_state_json != NULL){
                root = (cJSON*) _state_json;
                bank = cJSON_GetObjectItemCaseSensitive(root, "bank");
                menuTFTPrintInputMenu("Enter preset name:");
                //if(cJSON_IsArray(bank)) ESP_LOGI("PRESET", "bank is an array");
            }else ESP_LOGE("PRESET", "_state_json is NULL");
        case EV_FWD:
            c++;
            if(c>37) c = 37;
            menuTFTPrintChar(name, pos, c_list[c], PRINT_NORM);
            break;
        case EV_BWD:
            c--;
            if(c<0)c=0;
            menuTFTPrintChar(name, pos, c_list[c], PRINT_NORM);
            break;
        case EV_SHORT_PRESS:
            switch(c_list[c]){
                case '<':
                    name[pos] = '\0';
                    pos--;
                    if(pos<0)pos=0;
                    menuTFTPrintChar(name, pos, c_list[c], PRINT_NORM);
                    break;
                case '=':
                    if(pos == 0) break;
                    name[pos] = '\0';
                    buf = calloc(strlen(name) + 1, 1);
                    strcpy(buf, name);
                    strcpy(current_preset_name, name);
                    //Check if input is valid -> preset with same name already exists
                    if(bank != NULL && cJSON_GetArraySize(bank) < MAX_PRESET_COUNT){
                        if(validatePresetNameInput(buf, bank)){
                            //Input valid, build preset, then save to bank
                            preset = buildPreset(buf);
                            addPreset(bank, preset);
                            saveBank(root, current_bank, DELETE_JSON);
                            root = NULL;
                            bank = NULL;
                            return M_PRESET_P;
                        }else{
                            //name already used
                            menuTFTPrintInputError("Name not valid!");
                        }
                    }else{
                        ESP_LOGE("PRESET", "Bank == NULL");
                    }
                    break;
                default:
                    name[pos] = c_list[c];
                    pos++;
                    if(pos>12)pos=12;
                    menuTFTPrintChar(name, pos, c_list[c], PRINT_NORM);
                    break;
            }
            break;
        case EV_LONG_PRESS:
            root = NULL;
            bank = NULL;
            return M_PRESET_P;
            break;
        default:
            break;
    }

    return 0; // remain in current menu
}

static int preset_bank_new_def_handler(int it_id, int event, void* event_data){
    const char *c_list = "=0123456789abcdefghijklmnopqrstuvwxyz<";
    static int pos = 0, c = 1;
    static char name[10];
    char *buf;
    char nbuf[64];
    static list_t* file_list;
    switch(event){
        case EV_ENTERED_MENU:
            bzero(name, 10);
            pos = 0;
            c = 1;
            menuTFTResetTextWrap();
            file_list = (list_t*) _state_data;
            menuTFTPrintInputMenu("Enter bank name:");
        case EV_FWD:
            c++;
            if(c>37) c = 37;
            menuTFTPrintChar(name, pos, c_list[c], PRINT_NORM);
            break;
        case EV_BWD:
            c--;
            if(c<0)c=0;
            menuTFTPrintChar(name, pos, c_list[c], PRINT_NORM);
            break;
        case EV_SHORT_PRESS:
            switch(c_list[c]){
                case '<':
                    name[pos] = '\0';
                    pos--;
                    if(pos<0)pos=0;
                    menuTFTPrintChar(name, pos, c_list[c], PRINT_NORM);
                    break;
                case '=':
                    if(pos == 0) break;
                    name[pos] = '\0';
                    buf = calloc(strlen(name) + 1, 1);
                    strcpy(buf, name);

                    //Check if input is valid -> bank with same name already exists
                    //Create bank
                    snprintf(nbuf, 64, "/sdcard/banks/%s.JSN",buf);
                    //ESP_LOGI("UI", "Bank New, file list count: %d", file_list->count);

                    if(file_list != NULL){
                        if(file_list->count < MAX_BANK_COUNT){
                            if(fileExists(nbuf) != 0){
                                //File already exists
                                menuTFTPrintInputError("File already exists");
                            }else{
                                //Create bank
                                //ESP_LOGI("UI", "Creating %s ...", nbuf);
                                initBank(nbuf);
                                menuTFTFlushMenuDataRect();
                                file_list = NULL;
                                return M_PRESET_B;
                            }        
                        }else{
                            //reached max bank count
                            menuTFTPrintInputError("Reached max bank count");
                        }
                    }
                    break;
                default:
                    name[pos] = c_list[c];
                    pos++;
                    if(pos>8)pos=8;
                    menuTFTPrintChar(name, pos, c_list[c], PRINT_NORM);
                    break;
            }
            break;
        case EV_LONG_PRESS:
            file_list = NULL;
            return M_PRESET_B;
            break;
    }

    return 0; // remain in current menu
}

void menuProcessEvent(int ev, void * ev_data){
    menusys_process_ev(_ms, ev, ev_data);
}

void initParams(){
    memset(envelopeIndex,0,sizeof(envelopeIndex));

    //Init matrix - every source/amount/destination to 0
    for(int j = 0; j < 8; j++)
    {
        if(j != 0 && j != 1){
            matrix[j].dst = MTX_NONE;
        }else{
            if(j == 0) matrix[j].dst = MTX_V0_PITCH;
            if(j == 1) matrix[j].dst = MTX_V1_PITCH;
        }
        matrix[j].amt = 0;
    }

    //Init Volume, Pitch, ADSR & Delay parameter
    for(int i = 0; i < 2; i++)
    {
        data[i].trig_mode_latch = false;
        disableTrigModeLatch(i);
        data[i].volume = 100;               
        data[i].pan = 0;                    //Q2.14 -1.0 = -16384  +1.0 = 16384 -100 = 100L; 100 = 100R; 0 = C
        data[i].pitch = 12;                 //Shifted +12
        data[i].pitch_cv_active = 0;        //0 = OFF, 1 = ON
        data[i].playback_speed = 16384;       //-1/1
        if(i){
            pbs_state_v1 = 1;
            play_state_v1 = SINGLE;
        }
        else{
            pbs_state_v0 = 1;
            play_state_v0 = SINGLE;
        }
        data[i].dist_drive = 0;            
        data[i].dist_active = 0;            //0 = OFF, 1 = ON
        data[i].delay_send = 0;
        data[i].filter.is_active = 0;       //0 = OFF, 1 = ON
        data[i].filter.base = 255;          //DUMMY 0 - 511
        data[i].filter.width = 255;         //DUMMY 0 - 511
        data[i].filter.q = 0;              //Q10.6; 16 = 0.25; 320 = 5.0; INCR: 0.015625
        data[i].adsr.attack = msLut[112];
        data[i].adsr.decay = msLut[138];
        data[i].adsr.sustain = 100;         //Q1.7; 0 = 0; 128 = 1.0; INCR: 0.0078125
        data[i].adsr.release = msLut[192];
        envelopeIndex[i][0] = 112;
        envelopeIndex[i][1] = 138;
        envelopeIndex[i][2] = 192;
        data[i].play_state.mode = SINGLE;     // 0 = SINGLE, 1 = LOOP, 2 = PIPO
        data[i].play_state.start = 0;       // 0 = 0%, 800 = 100% Q13.3, INCR: 0.125
        data[i].play_state.loop_start = 0;  // 0 = 0%, 800 = 100% Q13.3, INCR: 0.125
        data[i].play_state.loop_end = 800;  // 0 = 0%, 800 = 100% Q13.3, INCR: 0.125
        data[i].play_state.loop_position = 0;
        data[i].play_state.loop_length = data[i].play_state.loop_end - data[i].play_state.loop_start;

    }

    effectData.delay.is_active = 0;         //0 = OFF, 1 = ON
    effectData.delay.mode = 0;
    effectData.delay.time = 250; // 0 - 1000ms
    effectData.delay.pan = 64;  // center
    effectData.delay.feedback = 32;    // percent
    effectData.delay.volume = 50;      // percent

    effectData.extInData.is_active = 0;    // default values ext in off
    effectData.extInData.volume = 100;     // percent
    effectData.extInData.pan = 0;         // center
    effectData.extInData.delay_send = 0;   // don't send anything into delay
    

    //Init Audio-Thread params via. queue
    xQueueSend(v1_queue, (void*) &data[1], portMAX_DELAY);
    xQueueSend(v0_queue, (void*) &data[0], portMAX_DELAY);
    xQueueSend(eff_queue, &effectData, portMAX_DELAY);
    xQueueSend(playbackspeed_state_queue_v0, (void*)&pbs_state_v0, portMAX_DELAY);
    xQueueSend(playbackspeed_state_queue_v1, (void*)&pbs_state_v1, portMAX_DELAY);
    xQueueSend(mode_queue_v0, (void*)&data[0].play_state, portMAX_DELAY);
    xQueueSend(mode_queue_v1, (void*)&data[1].play_state, portMAX_DELAY);

    //Pass matrix data to audio thread
    for(int i = 0; i < 8; i++)
    {
        matrix_event.changed_param = matrix[i].dst;
        matrix_event.source = i;
        matrix_event.amount = matrix[i].amt;
        xQueueSend(m_event_queue, (void*) &matrix_event, portMAX_DELAY);
    }
}

cJSON* buildPreset(const char* name){
    char fn[21], buf[20];
    cJSON *val;

    cJSON *preset = cJSON_CreateObject();

    //add name
    cJSON* presetName =  cJSON_CreateString(name);
    cJSON_AddItemToObject(preset, "presetName", presetName);

    cJSON *paramData = cJSON_CreateObject();
    cJSON_AddItemToObject(preset, "paramData", paramData);
    cJSON *voiceParamArray = cJSON_CreateArray();
    cJSON_AddItemToObject(paramData, "voiceParameters", voiceParamArray);

    for(int i=0; i<2; i++){
        cJSON* paramObject = cJSON_CreateObject();
        val = cJSON_CreateString (itoa((int)data[i].trig_mode_latch, buf, 10));
        cJSON_AddItemToObject(paramObject, "trig_mode_latch", val); 
        val = cJSON_CreateString (itoa(data[i].volume, buf, 10));
        cJSON_AddItemToObject(paramObject, "volume", val);  
        val = cJSON_CreateString (itoa(data[i].pan, buf, 10));
        cJSON_AddItemToObject(paramObject, "pan", val);
        val = cJSON_CreateString (itoa(data[i].pitch, buf, 10));
        cJSON_AddItemToObject(paramObject, "pitch", val);
        val = cJSON_CreateString (itoa(data[i].pitch_cv_active, buf, 10));
        cJSON_AddItemToObject(paramObject, "pitch_cv_active", val);
        val = cJSON_CreateString (itoa(data[i].playback_speed, buf, 10));
        cJSON_AddItemToObject(paramObject, "playback_speed", val);
        val = cJSON_CreateString (itoa(data[i].delay_send, buf, 10));
        cJSON_AddItemToObject(paramObject, "delay_send", val);
        if(i){
            val = cJSON_CreateString (itoa(pbs_state_v1, buf, 10));
            cJSON_AddItemToObject(paramObject, "pbs_state", val);
            val = cJSON_CreateString (itoa(play_state_v1, buf, 10));
            cJSON_AddItemToObject(paramObject, "play_state", val);
        }else{
            val = cJSON_CreateString (itoa(pbs_state_v0, buf, 10));
            cJSON_AddItemToObject(paramObject, "pbs_state", val);
            val = cJSON_CreateString (itoa(play_state_v0, buf, 10));
            cJSON_AddItemToObject(paramObject, "play_state", val);
        }
        val = cJSON_CreateString (itoa(data[i].dist_drive, buf, 10));
        cJSON_AddItemToObject(paramObject, "dist_drive", val);
        val = cJSON_CreateString (itoa(data[i].dist_active, buf, 10));
        cJSON_AddItemToObject(paramObject, "dist_active", val);
        val = cJSON_CreateString (itoa(data[i].filter.is_active, buf, 10));
        cJSON_AddItemToObject(paramObject, "filter_is_active", val);
        val = cJSON_CreateString (itoa(data[i].filter.base, buf, 10));
        cJSON_AddItemToObject(paramObject, "filter_base", val);
        val = cJSON_CreateString (itoa(data[i].filter.width, buf, 10));
        cJSON_AddItemToObject(paramObject, "filter_width", val);
        val = cJSON_CreateString (itoa(data[i].filter.q, buf, 10));
        cJSON_AddItemToObject(paramObject, "filter_q", val);
        val = cJSON_CreateString (itoa(data[i].adsr.attack, buf, 10));
        cJSON_AddItemToObject(paramObject, "adsr_attack", val);
        val = cJSON_CreateString (itoa(data[i].adsr.decay, buf, 10));
        cJSON_AddItemToObject(paramObject, "adsr_decay", val);
        val = cJSON_CreateString (itoa(data[i].adsr.sustain, buf, 10));
        cJSON_AddItemToObject(paramObject, "adsr_sustain", val);
        val = cJSON_CreateString (itoa(data[i].adsr.release, buf, 10));
        cJSON_AddItemToObject(paramObject, "adsr_release", val);
        cJSON *arr = cJSON_CreateArray();
        cJSON_AddItemToObject(paramObject, "envelope_indices", arr);
        val = cJSON_CreateString (itoa(envelopeIndex[i][0], buf, 10));
        cJSON_AddItemToArray(arr, val);
        val = cJSON_CreateString (itoa(envelopeIndex[i][1], buf, 10));
        cJSON_AddItemToArray(arr, val);
        val = cJSON_CreateString (itoa(envelopeIndex[i][2], buf, 10));
        cJSON_AddItemToArray(arr, val);

        val = cJSON_CreateString (itoa(data[i].play_state.mode, buf, 10));
        cJSON_AddItemToObject(paramObject, "play_state_mode", val);
        val = cJSON_CreateString (itoa(data[i].play_state.start, buf, 10));
        cJSON_AddItemToObject(paramObject, "play_state_start", val);
        val = cJSON_CreateString (itoa(data[i].play_state.loop_start, buf, 10));
        cJSON_AddItemToObject(paramObject, "play_state_loop_start", val);
        val = cJSON_CreateString (itoa(data[i].play_state.loop_end, buf, 10));
        cJSON_AddItemToObject(paramObject, "play_state_loop_end", val);
        val = cJSON_CreateString (itoa(data[i].play_state.loop_position, buf, 10));
        cJSON_AddItemToObject(paramObject, "play_state_loop_position", val);
        val = cJSON_CreateString (itoa(data[i].play_state.loop_length, buf, 10));
        cJSON_AddItemToObject(paramObject, "play_state_loop_length", val);

        cJSON_AddItemToArray(voiceParamArray, paramObject);
    }

    cJSON *efx = cJSON_CreateObject();
    cJSON_AddItemToObject(paramData, "effectsParameters", efx);

    //delay parameters
    val = cJSON_CreateString (itoa(effectData.delay.is_active, buf, 10));
    cJSON_AddItemToObject(efx, "delay_is_active", val);
    val = cJSON_CreateString (itoa(effectData.delay.time, buf, 10));
    cJSON_AddItemToObject(efx, "delay_time", val);
    val = cJSON_CreateString (itoa(effectData.delay.pan, buf, 10));
    cJSON_AddItemToObject(efx, "delay_pan", val);
    val = cJSON_CreateString (itoa(effectData.delay.mode, buf, 10));
    cJSON_AddItemToObject(efx, "delay_mode", val);
    val = cJSON_CreateString (itoa(effectData.delay.feedback, buf, 10));
    cJSON_AddItemToObject(efx, "delay_feedback", val);
    val = cJSON_CreateString (itoa(effectData.delay.volume, buf, 10));
    cJSON_AddItemToObject(efx, "delay_volume", val);

    //extin parameters
    val = cJSON_CreateString (itoa(effectData.extInData.is_active, buf, 10));
    cJSON_AddItemToObject(efx, "extin_is_active", val);
    val = cJSON_CreateString (itoa(effectData.extInData.volume, buf, 10));
    cJSON_AddItemToObject(efx, "extin_volume", val);
    val = cJSON_CreateString (itoa(effectData.extInData.pan, buf, 10));
    cJSON_AddItemToObject(efx, "extin_pan", val);
    val = cJSON_CreateString (itoa(effectData.extInData.delay_send, buf, 10));
    cJSON_AddItemToObject(efx, "extin_delay_send", val);

    cJSON *modMatrixArray = cJSON_CreateArray();
    cJSON_AddItemToObject(paramData, "modMatrixParameters", modMatrixArray);
    for(int i = 0; i < 8; i++)
    {
        cJSON* matrixObject = cJSON_CreateObject();
        val = cJSON_CreateString (itoa(matrix[i].amt, buf, 10));
        cJSON_AddItemToObject(matrixObject, "amount", val);  
        val = cJSON_CreateString (itoa(matrix[i].dst, buf, 10));
        cJSON_AddItemToObject(matrixObject, "destination", val);  
        cJSON_AddItemToArray(modMatrixArray, matrixObject);
    }
    return preset;
}

void loadParams(cJSON* preset){

    //Get param data 
    cJSON *paramData = cJSON_GetObjectItemCaseSensitive(preset, "paramData");
    if(paramData == NULL){
        ESP_LOGE("PRESET", "Couldn't fetch paramData from preset");
        return;
    } 

    //Get presetName
    cJSON *nameObject = cJSON_GetObjectItemCaseSensitive(preset, "presetName");
    if(nameObject == NULL){
        ESP_LOGE("PRESET", "Couldn't fetch nameObject from preset");
        return;
    } 
    memset(current_preset_name, 0, sizeof(current_preset_name)/sizeof(char));
    strcpy(current_preset_name, (char*)nameObject->valuestring);

    //Get voice param data
    cJSON *paramObject = cJSON_GetObjectItemCaseSensitive(paramData, "voiceParameters");
    if(paramObject == NULL){
        ESP_LOGE("PRESET", "Couldn't fetch paramObject from preset");
        return;
    } 

    cJSON *val;
    for(int i=0; i<2; i++){
        cJSON *voiceParams = cJSON_GetArrayItem(paramObject, i);
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "trig_mode_latch");
        data[i].trig_mode_latch = (bool) atoi(val->valuestring); 
        // Set trig mode
        if(data[i].trig_mode_latch)
            enableTrigModeLatch(i);
        else
            disableTrigModeLatch(i);
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "volume");
        data[i].volume = atoi(val->valuestring);            
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "pan");
        data[i].pan = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "pitch");
        data[i].pitch = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "pitch_cv_active");
        data[i].pitch_cv_active = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "playback_speed");
        data[i].playback_speed = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "delay_send");
        data[i].delay_send = atoi(val->valuestring);  
        if(i){
            val = cJSON_GetObjectItemCaseSensitive(voiceParams, "pbs_state");
            pbs_state_v1 = atoi(val->valuestring);  
            val = cJSON_GetObjectItemCaseSensitive(voiceParams, "play_state");
            play_state_v1 = atoi(val->valuestring);  
        }else{
            val = cJSON_GetObjectItemCaseSensitive(voiceParams, "pbs_state");
            pbs_state_v0 = atoi(val->valuestring);  
            val = cJSON_GetObjectItemCaseSensitive(voiceParams, "play_state");
            play_state_v0 = atoi(val->valuestring);  
        }
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "dist_drive");    
        data[i].dist_drive = atoi(val->valuestring);      
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "dist_active");       
        data[i].dist_active = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "filter_is_active");
        data[i].filter.is_active = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "filter_base");
        data[i].filter.base = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "filter_width");
        data[i].filter.width = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "filter_q");
        data[i].filter.q = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "adsr_attack");
        data[i].adsr.attack = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "adsr_decay");
        data[i].adsr.decay = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "adsr_sustain");
        data[i].adsr.sustain = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "adsr_release");
        data[i].adsr.release = atoi(val->valuestring);  
    
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "envelope_indices");
        cJSON *val2 = cJSON_GetArrayItem(val, 0);
        envelopeIndex[i][0] = atoi(val2->valuestring);  
        val2 = cJSON_GetArrayItem(val, 1);
        envelopeIndex[i][1] = atoi(val2->valuestring);  
        val2 = cJSON_GetArrayItem(val, 2);
        envelopeIndex[i][2] = atoi(val2->valuestring);  

        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "play_state_mode");
        data[i].play_state.mode = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "play_state_start");
        data[i].play_state.start = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "play_state_loop_start");
        data[i].play_state.loop_start = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "play_state_loop_end");
        data[i].play_state.loop_end = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "play_state_loop_position");
        data[i].play_state.loop_position = atoi(val->valuestring);  
        val = cJSON_GetObjectItemCaseSensitive(voiceParams, "play_state_loop_length");
        data[i].play_state.loop_length = atoi(val->valuestring);  
    }

    //delay
    paramObject = cJSON_GetObjectItemCaseSensitive(paramData, "effectsParameters");
    val = cJSON_GetObjectItemCaseSensitive(paramObject, "delay_is_active");
    effectData.delay.is_active = atoi(val->valuestring);  
    val = cJSON_GetObjectItemCaseSensitive(paramObject, "delay_time");
    effectData.delay.time = atoi(val->valuestring);  
    val = cJSON_GetObjectItemCaseSensitive(paramObject, "delay_pan");
    effectData.delay.pan = atoi(val->valuestring);  
    val = cJSON_GetObjectItemCaseSensitive(paramObject, "delay_mode");
    effectData.delay.mode = atoi(val->valuestring);  
    val = cJSON_GetObjectItemCaseSensitive(paramObject, "delay_feedback");
    effectData.delay.feedback = atoi(val->valuestring);  
    val = cJSON_GetObjectItemCaseSensitive(paramObject, "delay_volume");
    effectData.delay.volume = atoi(val->valuestring);  

    //extin
    val = cJSON_GetObjectItemCaseSensitive(paramObject, "extin_is_active");
    effectData.extInData.is_active = atoi(val->valuestring);  
    val = cJSON_GetObjectItemCaseSensitive(paramObject, "extin_volume");
    effectData.extInData.volume = atoi(val->valuestring);
    val = cJSON_GetObjectItemCaseSensitive(paramObject, "extin_pan");
    effectData.extInData.pan = atoi(val->valuestring);
    val = cJSON_GetObjectItemCaseSensitive(paramObject, "extin_delay_send");
    effectData.extInData.delay_send = atoi(val->valuestring);


    paramObject = cJSON_GetObjectItemCaseSensitive(paramData,"modMatrixParameters");
    for(int i = 0; i < 8; i++)
    {
        cJSON *matrixParams = cJSON_GetArrayItem(paramObject, i);
        val = cJSON_GetObjectItemCaseSensitive(matrixParams, "amount");
        matrix[i].amt = atoi(val->valuestring);     
        val = cJSON_GetObjectItemCaseSensitive(matrixParams, "destination");
        matrix[i].dst = atoi(val->valuestring);  
    }

    //pass param data to audio thread
    xQueueSend(v1_queue, (void*) &data[1], portMAX_DELAY);
    xQueueSend(v0_queue, (void*) &data[0], portMAX_DELAY);
    xQueueSend(eff_queue, &effectData, portMAX_DELAY);
    xQueueSend(playbackspeed_state_queue_v0, (void*)&pbs_state_v0, portMAX_DELAY);
    xQueueSend(playbackspeed_state_queue_v1, (void*)&pbs_state_v1, portMAX_DELAY);
    xQueueSend(mode_queue_v0, (void*)&data[0].play_state, portMAX_DELAY);
    xQueueSend(mode_queue_v1, (void*)&data[1].play_state, portMAX_DELAY);

    //pass matrix data to audio thread
    for(int i = 0; i < 8; i++)
    {
        matrix_event.changed_param = matrix[i].dst;
        matrix_event.source = i;
        matrix_event.amount = matrix[i].amt;
        xQueueSend(m_event_queue, (void*) &matrix_event, portMAX_DELAY);
    }

    //ESP_LOGI("UI", "Successfully loaded preset data");
}

static void initParamsFromPreset(char* presetName, char* bankName){
    cJSON *root = NULL, *bank = NULL, *preset = NULL;
    if(strcasecmp(bankName, "") == 0){    
        //bankName is empty, get first available bank and load preset at index 0        
        list_t* file_list = list_create();
        getFilesInDir(file_list, "/sdcard/banks", ".JSN");
        strcpy(bankName, (char*)list_get_item(file_list, 0)->value);
        list_free(file_list);
    }
    root = getBankRoot(bankName);
    if(root != NULL)bank = cJSON_GetObjectItemCaseSensitive(root, "bank");
    else ESP_LOGE("MENU", "Error loading bank from file");
    if(bank != NULL){
        if(strcmp(presetName, "") == 0){
            preset = cJSON_GetArrayItem(bank, 0);   
            //getting preset at index 0        
            if(preset != NULL)strcpy(presetName, cJSON_GetObjectItemCaseSensitive(preset, "presetName")->valuestring);
            savePresetConfig(presetName, bankName);
        }
        //getPresetByName returns detached object, make sure to free, else memory leak
        preset = getPresetByName(presetName, root);         
        if(preset != NULL){
            loadParams(preset);
            cJSON_Delete(preset);
        }else ESP_LOGE("MENU", "Error preset from bank");
    }else ESP_LOGE("MENU", "Error loading bank");
    cJSON_Delete(root);
}



void initMenu(xQueueHandle ui_queue_v0, xQueueHandle ui_queue_v1, xQueueHandle effect_queue, xQueueHandle cv_queue_v0, xQueueHandle cv_queue_v1, xQueueHandle _mode_queue_v0, xQueueHandle _mode_queue_v1, xQueueHandle matrix_event_queue, xQueueHandle ev_queue){
    v0_queue = ui_queue_v0;
    v1_queue = ui_queue_v1;
    eff_queue = effect_queue;
    playbackspeed_state_queue_v0 = cv_queue_v0;
    playbackspeed_state_queue_v1 = cv_queue_v1;
    mode_queue_v0 = _mode_queue_v0;
    mode_queue_v1 = _mode_queue_v1;
    m_event_queue = matrix_event_queue;
    ui_ev_queue = ev_queue;
    loadPresetConfig(current_preset_name, current_bank);
    initParamsFromPreset(current_preset_name, current_bank);
    initTimeshift(&tz_shift);
    TFT_fillScreen(TFT_BLACK);
    TFT_resetclipwin();
    menuTFTPrintMainMenus();

    // using indentation to somewhat indicate levels of menus
    _ms = menusys_create();
    menusys_new_item(_ms, M_MAIN);
    menusys_item_set_default_cb(_ms, M_MAIN, main_menu_def_handler);

    menusys_new_item(_ms, M_PLAY);
    menusys_item_set_default_cb(_ms, M_PLAY, play_def_handler);
            menusys_new_item(_ms, M_VOICE);
            menusys_item_set_default_cb(_ms, M_VOICE, voice_def_handler);
                menusys_new_item(_ms, M_FILTER);
                menusys_item_set_default_cb(_ms, M_FILTER, filter_def_handler);
                menusys_new_item(_ms, M_ADSR);
                menusys_item_set_default_cb(_ms, M_ADSR, adsr_def_handler);
                menusys_new_item(_ms, M_PLAYMODE);
                menusys_item_set_default_cb(_ms, M_PLAYMODE, playmode_def_handler);
                menusys_new_item(_ms, M_CV_MATRIX);
                menusys_item_set_default_cb(_ms, M_CV_MATRIX, matrix_def_handler);
            menusys_new_item(_ms, M_EFFECTS);
            menusys_item_set_default_cb(_ms, M_EFFECTS, effects_def_handler);
                menusys_new_item(_ms, M_DELAY);
                menusys_item_set_default_cb(_ms, M_DELAY, delay_def_handler);
            menusys_new_item(_ms, M_EXTERNAL_IN);
            menusys_item_set_default_cb(_ms, M_EXTERNAL_IN, extin_def_handler);

    menusys_new_item(_ms, M_SLOT);
    menusys_item_set_default_cb(_ms, M_SLOT, bank_def_handler);
        menusys_new_item(_ms, M_SLOT_TYPESELECT);
        menusys_item_set_default_cb(_ms, M_SLOT_TYPESELECT, typeselect_def_handler);
            menusys_new_item(_ms, M_SLOT_FILEBROWSER);
            menusys_item_set_default_cb(_ms, M_SLOT_FILEBROWSER, filebrowser_def_handler);
                menusys_new_item(_ms, M_SLOT_DECODING);
                menusys_item_set_default_cb(_ms, M_SLOT_DECODING, decoding_def_handler);
            menusys_new_item(_ms, M_SLOT_USER);
            menusys_item_set_default_cb(_ms, M_SLOT_USER, userfilebrowser_def_handler);

    menusys_new_item(_ms, M_BROWSE);
    menusys_item_set_default_cb(_ms, M_BROWSE, browse_def_handler);
        menusys_new_item(_ms, M_BROWSE_TAG);
        //menusys_item_set_default_cb(_ms, M_BROWSE_TAG, browse_tag_def_handler); only partially functional at this point
        menusys_item_set_default_cb(_ms, M_BROWSE_TAG, browse_search_def_handler);
            menusys_new_item(_ms, M_BROWSE_TAG_RESULTS);
            menusys_item_set_default_cb(_ms, M_BROWSE_TAG_RESULTS, browse_tag_results_def_handler);
        menusys_new_item(_ms, M_BROWSE_SEARCH);
        menusys_item_set_default_cb(_ms, M_BROWSE_SEARCH, browse_search_def_handler);
        menusys_new_item(_ms, M_BROWSE_ID);
        menusys_item_set_default_cb(_ms, M_BROWSE_ID, browse_id_def_handler);
            menusys_new_item(_ms, M_BROWSE_ID_RESULT);
            menusys_item_set_default_cb(_ms, M_BROWSE_ID_RESULT, browse_id_res_def_handler);
        menusys_new_item(_ms, M_BROWSE_USER);
        menusys_item_set_default_cb(_ms, M_BROWSE_USER, browse_user_def_handler);

    menusys_new_item(_ms, M_PRESET);
    menusys_item_set_default_cb(_ms, M_PRESET, preset_def_handler);  
        menusys_new_item(_ms, M_PRESET_P);
        menusys_item_set_default_cb(_ms, M_PRESET_P, presets_menu_def_handler); 
            menusys_new_item(_ms, M_PRESET_NEW);
            menusys_item_set_default_cb(_ms, M_PRESET_NEW, preset_new_def_handler);
        menusys_new_item(_ms, M_PRESET_B);
        menusys_item_set_default_cb(_ms, M_PRESET_B, banks_menu_def_handler); 
            menusys_new_item(_ms, M_PRESET_BANK_NEW);
            menusys_item_set_default_cb(_ms, M_PRESET_BANK_NEW, preset_bank_new_def_handler);

    menusys_new_item(_ms, M_MORE);
    menusys_item_set_default_cb(_ms, M_MORE, more_def_handler);
        menusys_new_item(_ms, M_ABOUT);
        menusys_item_set_default_cb(_ms, M_ABOUT, about_def_handler);
        menusys_new_item(_ms, M_SETTINGS);
        menusys_item_set_default_cb(_ms, M_SETTINGS, settings_def_handler);
            menusys_new_item(_ms, M_SETTINGS_INPUT);
            menusys_item_set_default_cb(_ms, M_SETTINGS_INPUT, settings_input_def_handler);

    menusys_all_set_ev_cb(_ms, EV_TIMER_REPEATING_SLOW, timer_handler);
    menusys_set_active_item(_ms, M_MAIN);

}

