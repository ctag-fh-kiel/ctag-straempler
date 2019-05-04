#include "preset.h"

void initBank(char* fileName){
    char buf[20];
    cJSON *val;

    cJSON *root = cJSON_CreateObject();
    cJSON *rootArray = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "bank", rootArray);

    cJSON *preset = cJSON_CreateObject();
    cJSON_AddItemToArray(rootArray, preset);

    //add name
    val = cJSON_CreateString ("Init");
    cJSON_AddItemToObject(preset, "presetName", val);

    cJSON *paramData = cJSON_CreateObject();
    cJSON_AddItemToObject(preset, "paramData", paramData);
    cJSON *voiceParamArray = cJSON_CreateArray();
    cJSON_AddItemToObject(paramData, "voiceParameters", voiceParamArray);
   
    for(int i=0; i<2; i++){
        cJSON* paramObject = cJSON_CreateObject();
        val = cJSON_CreateString (itoa(false, buf, 10));
        cJSON_AddItemToObject(paramObject, "trig_mode_latch", val); 
        val = cJSON_CreateString (itoa(100, buf, 10));
        cJSON_AddItemToObject(paramObject, "volume", val);  
        val = cJSON_CreateString (itoa(0, buf, 10));
        cJSON_AddItemToObject(paramObject, "pan", val);
        val = cJSON_CreateString (itoa(12, buf, 10));
        cJSON_AddItemToObject(paramObject, "pitch", val);
        val = cJSON_CreateString (itoa(0, buf, 10));
        cJSON_AddItemToObject(paramObject, "pitch_cv_active", val);
        val = cJSON_CreateString (itoa(16384, buf, 10));
        cJSON_AddItemToObject(paramObject, "playback_speed", val);
        val = cJSON_CreateString (itoa(0, buf, 10));
        cJSON_AddItemToObject(paramObject, "delay_send", val);
        if(i){
            val = cJSON_CreateString (itoa(1, buf, 10));
            cJSON_AddItemToObject(paramObject, "pbs_state", val);
            val = cJSON_CreateString (itoa(SINGLE, buf, 10));
            cJSON_AddItemToObject(paramObject, "play_state", val);
        }else{
            val = cJSON_CreateString (itoa(1, buf, 10));
            cJSON_AddItemToObject(paramObject, "pbs_state", val);
            val = cJSON_CreateString (itoa(SINGLE, buf, 10));
            cJSON_AddItemToObject(paramObject, "play_state", val);
        }
        val = cJSON_CreateString (itoa(0, buf, 10));
        cJSON_AddItemToObject(paramObject, "dist_drive", val);
        val = cJSON_CreateString (itoa(0, buf, 10));
        cJSON_AddItemToObject(paramObject, "dist_active", val);
        val = cJSON_CreateString (itoa(0, buf, 10));
        cJSON_AddItemToObject(paramObject, "filter_is_active", val);
        val = cJSON_CreateString (itoa(255, buf, 10));
        cJSON_AddItemToObject(paramObject, "filter_base", val);
        val = cJSON_CreateString (itoa(255, buf, 10));
        cJSON_AddItemToObject(paramObject, "filter_width", val);
        val = cJSON_CreateString (itoa(0, buf, 10));
        cJSON_AddItemToObject(paramObject, "filter_q", val);
        val = cJSON_CreateString (itoa(msLut[112], buf, 10));
        cJSON_AddItemToObject(paramObject, "adsr_attack", val);
        val = cJSON_CreateString (itoa(msLut[138], buf, 10));
        cJSON_AddItemToObject(paramObject, "adsr_decay", val);
        val = cJSON_CreateString (itoa(100, buf, 10));
        cJSON_AddItemToObject(paramObject, "adsr_sustain", val);
        val = cJSON_CreateString (itoa(msLut[192], buf, 10));
        cJSON_AddItemToObject(paramObject, "adsr_release", val);

        cJSON *arr = cJSON_CreateArray();
        cJSON_AddItemToObject(paramObject, "envelope_indices", arr);
        val = cJSON_CreateString (itoa(112, buf, 10));
        cJSON_AddItemToArray(arr, val);
        val = cJSON_CreateString (itoa(138, buf, 10));
        cJSON_AddItemToArray(arr, val);
        val = cJSON_CreateString (itoa(192, buf, 10));
        cJSON_AddItemToArray(arr, val);

        val = cJSON_CreateString (itoa(SINGLE, buf, 10));
        cJSON_AddItemToObject(paramObject, "play_state_mode", val);
        val = cJSON_CreateString (itoa(0, buf, 10));
        cJSON_AddItemToObject(paramObject, "play_state_start", val);
        val = cJSON_CreateString (itoa(0, buf, 10));
        cJSON_AddItemToObject(paramObject, "play_state_loop_start", val);
        val = cJSON_CreateString (itoa(800, buf, 10));
        cJSON_AddItemToObject(paramObject, "play_state_loop_end", val);
        val = cJSON_CreateString (itoa(0, buf, 10));
        cJSON_AddItemToObject(paramObject, "play_state_loop_position", val);
        val = cJSON_CreateString (itoa(800, buf, 10));
        cJSON_AddItemToObject(paramObject, "play_state_loop_length", val);

        cJSON_AddItemToArray(voiceParamArray, paramObject);
    }

    cJSON *efx = cJSON_CreateObject();
    cJSON_AddItemToObject(paramData, "effectsParameters", efx);

    //delay parameters
    val = cJSON_CreateString (itoa(0, buf, 10));
    cJSON_AddItemToObject(efx, "delay_is_active", val);
    val = cJSON_CreateString (itoa(250, buf, 10));
    cJSON_AddItemToObject(efx, "delay_time", val);
    val = cJSON_CreateString (itoa(64, buf, 10));
    cJSON_AddItemToObject(efx, "delay_pan", val);
    val = cJSON_CreateString (itoa(0, buf, 10));
    cJSON_AddItemToObject(efx, "delay_mode", val);
    val = cJSON_CreateString (itoa(32, buf, 10));
    cJSON_AddItemToObject(efx, "delay_feedback", val);
    val = cJSON_CreateString (itoa(50, buf, 10));
    cJSON_AddItemToObject(efx, "delay_volume", val);

    //extin parameters
    val = cJSON_CreateString (itoa(0, buf, 10));
    cJSON_AddItemToObject(efx, "extin_is_active", val);
    val = cJSON_CreateString (itoa(100, buf, 10));
    cJSON_AddItemToObject(efx, "extin_volume", val);
    val = cJSON_CreateString (itoa(0, buf, 10));
    cJSON_AddItemToObject(efx, "extin_pan", val);
    val = cJSON_CreateString (itoa(0, buf, 10));
    cJSON_AddItemToObject(efx, "extin_delay_send", val);

    cJSON *modMatrixArray = cJSON_CreateArray();
    cJSON_AddItemToObject(paramData, "modMatrixParameters", modMatrixArray);
    for(int i = 0; i < 8; i++)
    {
        cJSON* matrixObject = cJSON_CreateObject();
        val = cJSON_CreateString (itoa(0, buf, 10));
        cJSON_AddItemToObject(matrixObject, "amount", val);  
        
        if(i == 0) val = cJSON_CreateString (itoa(MTX_V0_PITCH, buf, 10));
        else if(i == 1) val = cJSON_CreateString (itoa(MTX_V1_PITCH, buf, 10));
        else val = cJSON_CreateString (itoa(MTX_NONE, buf, 10));

        cJSON_AddItemToObject(matrixObject, "destination", val);  
        cJSON_AddItemToArray(modMatrixArray, matrixObject);
    }

    char* s = cJSON_PrintUnformatted(root);
    writeJSONFile(fileName, s);
    free(s);
    cJSON_Delete(root);
    ESP_LOGI("UI", "Successfully stored preset.");
}

void saveBank(cJSON* root, char* fileName, preset_action_t action){
    //ESP_LOGI("PRESET", "Saving bank to %s...", fileName);
    char fn[32];
    char* buf;
    buf = cJSON_PrintUnformatted(root);
    if( buf != NULL){   
        sprintf(fn, "/sdcard/banks/%s.JSN", fileName);
        writeJSONFile(fn, buf);
        if(action == DELETE_JSON)cJSON_Delete(root);
        free(buf);
    }else{
        ESP_LOGE("PRESET", "JSON char array is NULL");
    } 
}

int deleteBank(const char* fileName){
    char fn[32];
    sprintf(fn, "/sdcard/banks/%s.JSN", fileName);
    return deleteFile(fn);
}

cJSON* getPresetByName(const char* presetName, cJSON* root){
    char buf[12];
    if(root == NULL) return NULL;
    cJSON* element = NULL;
    cJSON* bank = cJSON_GetObjectItemCaseSensitive(root, "bank");
    cJSON* val;
    int i = 0;
    //Iterate over preset objects in bank array
    cJSON_ArrayForEach(element, bank){
        if(cJSON_IsObject(element) && element != NULL){
            val = cJSON_GetObjectItemCaseSensitive(element, "presetName");
            sprintf(buf, val->valuestring);
            if(strcmp(buf, presetName) == 0){
                return cJSON_DetachItemFromArray(bank, i);
            }
            memset(buf, 0, 12);
            i++;
        }
    }
    cJSON_Delete(bank);
    return NULL;
}

int getPresetIndex(const char* presetName, cJSON* root){
    char buf[12];
    if(root == NULL) return -1;
    cJSON* element = NULL;
    cJSON* bank = cJSON_GetObjectItemCaseSensitive(root, "bank");
    int i = 0;

    //Iterate over preset objects in bank array
    cJSON_ArrayForEach(element, bank){
        if(cJSON_IsObject(element) && element != NULL){
            sprintf(buf, cJSON_GetObjectItemCaseSensitive(element, "presetName")->valuestring);
            if(strcasecmp(buf, presetName) == 0){
                ESP_LOGI("PRESET", "Found preset, returning...");
                return i;
            } 
            i++;
        }
    }    
    return -1;
}

char* getPresetNameAtIndex(cJSON* array, int* preset_index){
    cJSON* element = NULL;
    cJSON* bank = array;
    int i = 0;

    //Iterate over preset objects in bank array
    if(bank != NULL){
        cJSON_ArrayForEach(element, bank){
            if(cJSON_IsObject(element) && element != NULL){
                if(i == *preset_index) return cJSON_GetObjectItemCaseSensitive(element, "presetName")->valuestring;
                i++;
            }
        }  
    }
    return NULL;
}

void addPreset(cJSON* bank, cJSON* preset){
    cJSON_AddItemToArray(bank, preset);
}

void deletePreset(cJSON* bank, int* preset_index){
    cJSON_DeleteItemFromArray(bank, *preset_index);
}

cJSON* getBankRoot(char* bank_name){
    char fn[32];
    //ESP_LOGI("PRESET", "Getting root object from file %s...", bank_name);
    sprintf(fn, "/sdcard/banks/%s.JSN", bank_name);
    cJSON* root = readJSONFileAsCJSON(fn);
    if(root != NULL) return root;
    ESP_LOGE("PRESET", "Root is NULL");
    return NULL;
}

int validatePresetNameInput(char* presetName, cJSON* bank){
    cJSON* val;
    cJSON* element = NULL;
    char buf[12];
    //Iterate over preset objects in bank array
    if(bank != NULL){
        cJSON_ArrayForEach(element, bank){
            if(cJSON_IsObject(element)){
                val = cJSON_GetObjectItemCaseSensitive(element, "presetName");
                sprintf(buf, val->valuestring);
                //if(strcmp(buf, presetName) == 0) return 0;
                if(strcasecmp(buf, presetName) == 0) return 0;
            }
        }
    }
    return 1;
}