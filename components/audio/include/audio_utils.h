#pragma once
#include "stdint.h"
#include "stdlib.h"
#include <sys/stat.h>
#include <stdio.h>
#include "esp_log.h"

/**
 * @brief In-Place algorithm to reverse 32bit buffer
 * 
 * @param size - How many elements the buffer has
 * @param buf - Pointer to the buffer that is being reversed
 */
void reverseBuffer32(size_t size, int32_t *buf);

/**
 * @brief In-Place algorithm to reverse 16bit buffer
 * 
 * @param size - How many elements the buffer has
 * @param buf - Pointer to the buffer that is being reversed
 */
void reverseBuffer16(size_t size, int16_t *buf);

/**
 * @brief In-Place algorithm to reverse 8bit buffer
 * 
 * @param size - How many elements the buffer has
 * @param buf - Pointer to the 8bit buffer that is being reversed
 */
void reverseBuffer8(size_t size, int8_t *buf);

/**
 * @brief Function to make odd values even, rounding up 
 * 
 * @param val - Value to round up
 * @return uint32_t - Returns normalized value
 */
uint32_t normEncData(uint32_t val);

/**
 * @brief rounds to the nearest value dividable by 32
 * 
 * @param val 
 * @return uint32_t 
 */
uint32_t norm32(uint32_t val);

/**
 * @brief Signs value 
 * 
 * @param x - Value to sign
 * @return int 
 */
int sign(int x);

/**
 * @brief Hardlimiting buffer values
 * 
 * @param src Source buffer
 * @param dst Destination buffer
 * @param len Length of buffer
 */
void accumulateBufferHardLimit(const int16_t *src, int16_t *dst, int len);

/**
 * @brief 
 * 
 * @param buf 
 * @param len 
 * @param f 
 * @param sh 
 */
void multShiftBuffer(int16_t *buf, int len, int f, int sh);

/**
 * @brief 
 * 
 * @param buf 
 * @param len 
 * @param pre 
 * @param f 
 * @param sh 
 */
void multShiftBufferLI(int16_t *buf, int len, int pre, int f, int sh);

/**
 * @brief 
 * 
 * @param buf 
 * @param len 
 * @param v 
 */
void fillValue(int16_t *buf, int len, int v);

/**
 * @brief Checks if LoopStart and LoopEnd are in same 8KB block
 * 
 * @param lStart loopStart 
 * @param lEnd loopEnd
 * @return int return 1 when values are in same 8092KB block, 0 otherwise
 */
int checkBlock(uint32_t *lStart, uint32_t *lEnd);

/**
 * @brief Get block count for position
 * 
 * @param pos position to check
 * @return int number of block pos is in
 */
int getBlockCount(int32_t* pos);

/**
 * @brief If value would be negativ will return 0
 * 
 * @param val value to check
 * @return int32_t 0 or val
 */
int32_t nullCheck(int32_t val);

/**
 * @brief Calculates reading posistion in buffer, when loopEnd reached & start in same block
 * 
 * @param b block count
 * @param loopStart 
 * @return uint32_t reading position of current buffer
 */
uint32_t calcRpos(int b, uint32_t* loopStart );

/**
 * @brief Shifts arr of size by val to right
 * 
 * @param arr data to shift
 */
void shiftRI(uint16_t* arr, int val, int size);

uint32_t get_audio_file_size(FILE* f);