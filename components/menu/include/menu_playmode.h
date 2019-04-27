#pragma once
#include "menu_shapes.h"
#include "menutft_utils.h"
#include "tft.h"
#include "menu_types.h"

/**
 * @brief playmode marker indices
 * 
 */
enum{
    I_START = 0x00,
    I_LSTART,
    I_LEND
};

/**
 * @brief Updates start marker position, reprints marker if position has changed
 * @param data              pointer to play_state data         
 * @param loopMarker        array that holds current x positions of loop markers
 * @param prevLoopMarker    array that holds previous x positions of loop markers
 * @param width             pointer to with of marker window
 * @param height            pointer to height of marker
 */
void updateStart(play_state_data_t* data, uint16_t* loopMarker, uint16_t* prevLoopMarker, const int* width, const int* height);

/**
 * @brief Updates loop start marker position, reprints marker if position has changed
 * @param data              pointer to play_state data         
 * @param loopMarker        array that holds current x positions of loop markers
 * @param prevLoopMarker    array that holds previous x positions of loop markers
 * @param width             pointer to with of marker window
 * @param height            pointer to height of marker
 */
void updateLoopStart(play_state_data_t* data, uint16_t* loopMarker, uint16_t* prevLoopMarker, const int* width, const int* height, const int menu_item);

/**
 * @brief Updates loop end marker position, reprints marker if position has changed
 * @param data              pointer to play_state data         
 * @param loopMarker        array that holds current x positions of loop markers
 * @param prevLoopMarker    array that holds previous x positions of loop markers
 * @param width             pointer to with of marker window
 * @param height            pointer to height of marker
 */
void updateLoopEnd(play_state_data_t* data, uint16_t* loopMarker, uint16_t* prevLoopMarker, const int* width, const int* height);

/**
 * @brief Resets previous marker positions
 * 
 * @param prevLoopMarker    array that holds previous x positions of loop markers
 */
void resetPrevLoopMarker(uint16_t* prevLoopMarker);

/**
 * @brief Updates previous loop marker positions to current positions
 * 
 * @param loopMarker        array that holds current x positions of loop markers
 * @param prevLoopMarker    array that holds previous x positions of loop markers
 */
void updatePrevLoopMarker(uint16_t* loopMarker, uint16_t* prevLoopMarker);

/**
 * @brief Clears loop start marker
 * 
 * @param loopMarker        array that holds current x positions of loop markers
 * @param width             pointer to with of marker window
 * @param height            pointer to height of marker
 */
void clearLStartMarker(uint16_t* loopMarker, const int* width, const int* height);