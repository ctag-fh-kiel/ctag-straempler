#pragma once
#include "menu_shapes.h"
#include "menutft_utils.h"
#include "tft.h"

/**
 * @brief curve indices
 * 
 */
enum{
    I_ATTACK = 0x00,
    I_DECAY,
    I_SUSTAIN,
    I_RELEASE
};

/**
 * @brief Updates attack point, redraws if value has changed
 * 
 * @param curve             array of current adsr points
 * @param prevCurve         array of previous adsr points
 * @param data              adsr data 0 - 10000ms
 * @param startPoint        starting point of envelope
 * @param width             width of adsr window
 * @param height            heigth of adsr window
 */
void updateAttackPoint(vec2d_t* curve, vec2d_t* prevCurve, adsr_data_t* data, vec2d_t* startPoint, const int* width, const int* height);

/**
 * @brief Updates decay point, redraws if value has changed
 * 
 * @param curve             array of current adsr points
 * @param prevCurve         array of previous adsr points
 * @param data              adsr data 0 - 10000ms
 * @param width             width of adsr window
 * @param height            heigth of adsr window
 */
void updateDecayPoint(vec2d_t* curve, vec2d_t* prevCurve, adsr_data_t* data, vec2d_t* startPoint, const int* width, const int* height);

/**
 * @brief Updates sustain point, redraws if value has changed
 * 
 * @param curve             array of current adsr points
 * @param prevCurve         array of previous adsr points
 * @param data              adsr data 0 - 10000ms
 * @param width             width of adsr window
 * @param height            heigth of adsr window
 */
void updateSustainPoint(vec2d_t* curve, vec2d_t* prevCurve, adsr_data_t* data, const int* width, const int* height);


/**
 * @brief Updates release point, redraws if value has changed
 * 
 * @param curve             array of current adsr points
 * @param prevCurve         array of previous adsr points
 * @param data              adsr data 0 - 10000ms
 * @param width             width of adsr window
 * @param height            heigth of adsr window
 */
void updateReleasePoint(vec2d_t* curve, vec2d_t* prevCurve, adsr_data_t* data, const int* width, const int* height);


/**
 * @brief updates the previous curve values to current values
 * 
 * @param curve             array holding current curve points
 * @param prevCurve         array holding previous curve points
 */
void updatePrevCurve(vec2d_t* curve, vec2d_t* prevCurve);


/**
 * @brief Resets previous curve values
 * 
 * @param prevCurve         array holding previous curve points
 */
void resetPrevCurve(vec2d_t* prevCurve);

/**
 * @brief Prints indicators at every transitional point of the envelope
 * 
 * @param curve             array of current adsr points
 * @param startPoint        starting point of the envelope
 * @param radius            radius of the indicators
 * @param color             color of the indicators
 */
void printIndicatorPoints(vec2d_t* curve, vec2d_t* startPoint, int radius, color_t color);