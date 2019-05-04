#pragma once
#include "cJSON.h"

/**
 * @brief Initializes time shift variable from config file
 * 
 * @param tz_shift      pointer to time shift variable
 */
void initTimeshift(int *tz_shift);

/**
 * @brief compares current in memory wifi-settings with wifi-settings in config.jsn
 * 
 * @param curSettings   pointer to current wifi-settings
 * @return int          1 if changed, 0 otherwise
 */
int wifiSettingsChanged(cJSON* curSettings);

/**
 * @brief Saves current preset/bank names to config
 * 
 * @param preset        name of current preset
 * @param bank          name of current bank
 */
void savePresetConfig(char* preset, char* bank);


/**
 * @brief loads preset/bank names from config
 * 
 * @param preset        name of current preset
 * @param bank          name of current bank
 */
void loadPresetConfig(char* bankName, char* presetName);

/**
 * @brief               validates if config file has correct structure and formatting
 * 
 * @return int          returns -1 if not valid, else 1
 */
int validateConfig();