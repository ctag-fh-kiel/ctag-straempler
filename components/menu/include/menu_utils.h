#pragma once
#include "audio.h"
#include "audio_luts.h"
#include "audio_utils.h"
#include "menu_types.h"
#include "list.h"



/**
 * @brief increments/decrements ui - general voice parameters at certain index
 * 
 * @param data      pointer to data
 * @param index     index of submenu parameter
 * @param vid       voice id
 * @param pbs_state pointer to array that holds current playback speed state 
 * @param handle    QueueHandle to playback speed queue
 */
void incParamValue(param_data_t* data, int index, int vid, bool* pbs_state , matrix_ui_row_t* matrix, xQueueHandle handle);
void decParamValue(param_data_t* data, int index, int vid, bool* pbs_state, matrix_ui_row_t* matrix, xQueueHandle handle);

/**
 * @brief increments/decrements ui - adsr parameters at certain index
 * 
 * @param data      pointer to data
 * @param adsrIndex pointer to array that holds the current indexes, used for LUT
 * @param index     index of parameter 
 */
void incADSRValue(adsr_data_t* data, uint16_t* adsrIndex, int index);
void decADSRValue(adsr_data_t* data, uint16_t* adsrIndex, int index);

/**
 * @brief increments/decrements ui - filter parameters at certain index
 * 
 * @param data      pointer to filter data
 * @param index     index of submenu parameter
 */
void incFilterValue(filter_data_t* data, int index);
void decFilterValue(filter_data_t* data, int index);

/**
 * @brief increments/decrements ui - playmode parameters at certain index
 * 
 * @param data      pointer to playmode data
 * @param index     index of submenu parameter
 */
void incPlaymodeValue(play_state_data_t* data, int index, xQueueHandle mode_handle);
void decPlaymodeValue(play_state_data_t* data, int index, xQueueHandle mode_handle);

/**
 * @brief increments/decrements amount value of matrix destination
 * 
 * @param matrix    pointer to matrix data
 * @param source    current selected source 
 */
void incItemAmount(matrix_ui_row_t* matrix, int source);
void decItemAmount(matrix_ui_row_t* matrix, int source);

/**
 * @brief increments/decrements current showing destination
 * 
 * @param matrix    pointer to matrix data 
 * @param source    current selected source
 */
void incDestination(matrix_ui_row_t* matrix, int source);
void decDestination(matrix_ui_row_t* matrix, int source);

/**
 * @brief   increments/decrements ui - delay parameters at certain index
 * 
 * @param delay     pointer to delay data
 * @param index     index of submenu parameter
 */

void incDelayValue(delay_data_t* delay, int index);
void decDelayValue(delay_data_t* delay, int index);


/**
 * @brief   increments/decrements ui - extin parameters at certain index
 * 
 * @param extin     pointer to delay data
 * @param index     index of submenu parameter
 */

void incExtInValue(ext_in_data_t* extin, int index);
void decExtInValue(ext_in_data_t* extin, int index);


/**
 * @brief increments/decrements setting parameters at certain index
 * 
 * @param tz        pointer to time_shift variable
 * @param index     index of submenu parameter
 */
void incSettingsItem(int *tz, int index);
void decSettingsItem(int *tz, int index);
