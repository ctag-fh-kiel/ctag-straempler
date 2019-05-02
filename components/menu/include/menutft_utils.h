#pragma once
#include "stdint.h"
#include "menutft.h"
#include "tft.h"

typedef enum{
    MS_SUCCESS,
    MS_ERROR
} menu_fb_state_t;


//Flush conditions, for additional conditions expand default_conditions or add new array and size
extern const int default_conditions[9];
extern const int default_cond_size;
extern const int adsr_conditions[8];
extern const int adsr_cond_size;
extern const int filter_conditions[6];
extern const int filter_cond_size;
extern const int negpos_conditions[6];
extern const int negpos_cond_size;
extern const int playmode_conditions[4];
extern const int playmode_cond_size;

// Functions
/**
 * @brief Scales value to range/width, use with uint's only
 * 
 * @param val       value to scale
 * @param log_max   logical max, maximum of value
 * @param width     width/maximum of the range to scale to. 
 * @return uint16_t 
 */
uint16_t menuTFTScaleToWidth(uint16_t val, uint16_t log_max, uint16_t width);

/**
 * @brief Scales value to range
 * 
 * @param val       value to scale
 * @param log_max   logical max, maximum of value
 * @param phys_max  maximum of the range to scale to
 * @return int16_t  
 */
int16_t menuTFTScaleToRange(int16_t val, int16_t log_max, int16_t phys_max);

/**
 * @brief Flushes value at row * multiplier if value matches condition
 * 
 * @param value         value to check against condition
 * @param multiplier    multiplier of the y-value
 * @param cond          array of integer to check the value against, when condition is met area will be flushed, if NULL flush every call
 * @param cond_size     size of condition-array
 * @param color         color to flush the area with
 */
void menuTFTFlushValue(int value, int multiplier, const int* cond, const int* cond_size, color_t* color);

/**
 * @brief Flushes at row * multiplier
 * 
 * @param multiplier    y-value = row * multiplier
 * @param color         color to flush the area with 
 */
void menuTFTFlush(int multiplier, color_t* color);

/**
 * @brief Flushes rectangle where menu data is displayed
 * 
 */
void menuTFTFlushMenuDataRect();

/**
 * @brief Returns next character x position, used for input menu
 * 
 * @param str           pointer to input array
 * @param pos           character position of current input value
 * @return int          x position of next character
 */
int menuTFTGetCharPos(char* str, int pos);