#pragma once

#include "audio_types.h"
#include "fileio.h"
#include "cJSON.h"
#include "audio_luts.h"
#include "string.h"

#define MAX_BANK_COUNT 30
#define MAX_PRESET_COUNT 30

//specifies if json object should be deleted
typedef enum{
    NO_ACTION,
    DELETE_JSON
} preset_action_t;

/**
 * @brief Initializes preset bank with one Init - Preset
 * 
 * @param fileName      path and name of bank   default folder: /sdcard/banks/<name>.JSN
 */
void initBank(char* fileName);

/**
 * @brief Saves bank to bank file
 * 
 * @param root          root JSON object
 * @param fileName      name of bank
 */
void saveBank(cJSON* root, char* fileName, preset_action_t action);

/**
 * @brief deletes bank file
 * 
 * @param fileName      path to file
 * @return int          return 1 if successful, 0 otherwise
 */
int deleteBank(const char* fileName);

/**
 * @brief Gets preset object from bank
 * 
 * @param presetName    preset name 
 * @param root          root object of bank file
 * @param preset_index  pointer to current preset index
 * @return cJSON*       returns detached cJSON object if successful, NULL otherwise *MAKE SURE TO ASSIGN TO POINTER, ELSE MEMORY LEAK*
 */
cJSON* getPresetByName(const char* presetName, cJSON* root);

/**
 * @brief Gets preset index in array by name
 * 
 * @param presetName    name of preset
 * @param root          pointer to root object of bank file
 * @return int          returns index of preset in bank array, -1 when not found or error
 */
int getPresetIndex(const char* presetName, cJSON* root);

/**
 * @brief Get preset name at giving index in bank array
 * 
 * @param bank          pointer to bank array
 * @param preset_index  pointer to preset index
 * @return char*        pointer to array of name or NULL if not found
 */
char* getPresetNameAtIndex(cJSON* array, int* preset_index);

/**
 * @brief Saves preset object to bank array
 * 
 * @param bank          pointer to cJSON bank array
 * @param preset        pointer to cJSON preset object
 */
void addPreset(cJSON* bank, cJSON* preset);

/**
 * @brief Deletes preset from giving bank array
 * 
 * @param bank          pointer to CJSON bank array
 * @param preset_index  pointer to current preset index
 */
void deletePreset(cJSON* bank, int* preset_index);

/**
 * @brief Get bank root object from file
 * 
 * @param bank_name     name of bank
 * @return cJSON*       root object of file
 */
cJSON* getBankRoot(char* bank_name);

/**
 * @brief Iterates over presets in bank and validates if presetName is unused
 * 
 * @param presetName    name to check
 * @return int          1 if input valid, 0 if input already used
 */
int validatePresetNameInput(char* presetName, cJSON* bank);
