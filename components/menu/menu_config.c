#include "menu_config.h"
#include "string.h"
#include "fileio.h"
#include "esp_log.h"
#include <stdio.h>

void initTimeshift(int *tz_shift){
    cJSON *root = NULL, *settings = NULL, *val = NULL;
    root = readJSONFileAsCJSON("/sdcard/CONFIG.JSN");
    if(root != NULL){
        settings = cJSON_GetObjectItemCaseSensitive(root, "settings");
        val = cJSON_GetObjectItemCaseSensitive(settings, "tz_shift");
        *tz_shift = val->valueint;
    }else ESP_LOGE("MENU", "Error loading tz_shift from config");
    cJSON_Delete(root);
}

int wifiSettingsChanged(cJSON* curSettings){
    cJSON *root = NULL, *settings = NULL, *val = NULL;
    root = readJSONFileAsCJSON("/sdcard/CONFIG.JSN");
    if(root != NULL){
        cJSON* element = NULL;
        settings = cJSON_GetObjectItemCaseSensitive(root, "settings");
        cJSON* val;
        int i = 0;
        cJSON_ArrayForEach(element, settings){
            if(cJSON_IsString(element) && element != NULL){
                val = cJSON_GetArrayItem(curSettings, i);
                if(val != NULL){
                    if(strcmp(element->string, "ssid") == 0 || strcmp(element->string, "passwd") == 0){
                        if(strcmp(element->valuestring, val->valuestring) != 0){
                            cJSON_Delete(root);
                            return 1;
                        }
                    }
                }
                i++;
            }
        }
    }
    cJSON_Delete(root);
    return 0;
}

void savePresetConfig(char* preset, char* bank){
    cJSON *root = NULL, *settings = NULL, *val = NULL;
    root = readJSONFileAsCJSON("/sdcard/CONFIG.JSN");
    if(root != NULL) settings = cJSON_GetObjectItemCaseSensitive(root, "settings");
    if(settings != NULL){
        cJSON_ReplaceItemInObjectCaseSensitive(settings, "preset", cJSON_CreateString(preset));
        cJSON_ReplaceItemInObjectCaseSensitive(settings, "bank", cJSON_CreateString(bank));
        writeJSONFile("/sdcard/CONFIG.JSN", cJSON_Print(root));
        cJSON_Delete(root);
    }
}

void loadPresetConfig(char* presetName, char* bankName){
    cJSON *cfgRoot = NULL, *settings = NULL;
    cfgRoot = readJSONFileAsCJSON("/sdcard/CONFIG.JSN");
    if(cfgRoot != NULL) settings = cJSON_GetObjectItemCaseSensitive(cfgRoot, "settings");
    if(settings != NULL){
        strcpy(presetName, cJSON_GetObjectItemCaseSensitive(settings, "preset")->valuestring);
        strcpy(bankName, cJSON_GetObjectItemCaseSensitive(settings, "bank")->valuestring);
        cJSON_Delete(cfgRoot);
    }
}


int validateConfig(){
    //validate config with early exiting
    cJSON *cfgRoot = NULL, *settings = NULL, *slots = NULL, *val = NULL;
    //Getting config json 
    cfgRoot = readJSONFileAsCJSON("/sdcard/CONFIG.JSN");
    if(!cJSON_IsObject(cfgRoot)) return -1;
    //Check struture of config file
    //Getting slots json
    slots = cJSON_GetObjectItem(cfgRoot, "slots");
    if(!cJSON_IsArray(slots)) return -1;
    //Checking slots
    for(int i = 0; i < 2; i++)
    {
        cJSON* subval = NULL;
        val = cJSON_GetArrayItem(slots, i);
        if(!cJSON_IsObject(val)) return -1;
        subval = cJSON_GetObjectItemCaseSensitive(val, "name");
        if(!cJSON_IsString(subval)) return -1;
        subval = cJSON_GetObjectItemCaseSensitive(val, "file");
        if(!cJSON_IsString(subval)) return -1;
    }
    //Getting settings json
    settings = cJSON_GetObjectItemCaseSensitive(cfgRoot, "settings");
    if(!cJSON_IsObject(settings)) return -1;
    //Check individual settings
    val = cJSON_GetObjectItemCaseSensitive(settings, "ssid");
    if(!cJSON_IsString(val)) return -1;
    val = cJSON_GetObjectItemCaseSensitive(settings, "passwd");
    if(!cJSON_IsString(val)) return -1;
    val = cJSON_GetObjectItemCaseSensitive(settings, "apikey");
    if(!cJSON_IsString(val)) return -1;
    val = cJSON_GetObjectItemCaseSensitive(settings, "tz_shift");
    if(!cJSON_IsNumber(val)) return -1;
    val = cJSON_GetObjectItemCaseSensitive(settings, "preset");
    if(!cJSON_IsString(val)) return -1;
    val = cJSON_GetObjectItemCaseSensitive(settings, "bank");
    if(!cJSON_IsString(val)) return -1;
    cJSON_Delete(cfgRoot);
    return 1;
}