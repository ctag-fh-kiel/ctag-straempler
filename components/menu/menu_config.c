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
    //load bank
    cfgRoot = readJSONFileAsCJSON("/sdcard/CONFIG.JSN");
    if(cfgRoot != NULL) settings = cJSON_GetObjectItemCaseSensitive(cfgRoot, "settings");
    if(settings != NULL){
        strcpy(presetName, cJSON_GetObjectItemCaseSensitive(settings, "preset")->valuestring);
        strcpy(bankName, cJSON_GetObjectItemCaseSensitive(settings, "bank")->valuestring);
        cJSON_Delete(cfgRoot);
    }
}
