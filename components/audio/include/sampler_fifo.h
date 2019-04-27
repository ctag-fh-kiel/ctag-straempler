#pragma once
#include "stdint.h"
#include "stdlib.h"

/**
 * @struct fifo_t A Fifo-Struct
 * @brief Struct to represent a circular Fifo Buffer
 * @var buffer The Buffer where Data is stored
 * @var head The current write position of the Buffer
 * @var tail The current read position of the Buffer
 * @var size The size of the Buffer
 */

typedef struct{
    int32_t* buffer;
    uint32_t head;
    uint32_t tail;
    size_t size;
    uint32_t free_slots;
} fifo_t;

/**
 * @brief Function to initialize a given Fifo-Struct
 * 
 * @param f Fifo Struct which should be initialized
 * @param buf A Pointer to a Buffer which will be used as the internal Ring-Buffer
 * @param size The size of Elements which the Fifo Buffer can store
 */

void fifo_init(fifo_t* f, int32_t* buf, size_t size);

/**
 * @brief Function to read a Sample from a given Fifo and delete it in next call of fifo_write()
 * 
 * @param f Fifo from which the element should be rad
 * @param sample Pointer to the Sample where the Fifo-Element should be written
 * @return int Returns the number of elements read from Buffer (can be 0 for none or 1 for one)
 */

int fifo_pop_element(fifo_t* f, int32_t* sample);

/**
 * @brief Function to write a number of bytes into a Fifo
 * 
 * @param f The Fifo where Data should be written to
 * @param buf The Buffer where Data is copied from
 * @param n_int_32_elements Number of int32_t elements to copy
 * @return int Returns the number of int32_t elements succesfully written to Fifo
 */

int fifo_write(fifo_t* f, const int32_t* buf, int n_int_32_elements);

/**
 * @brief Function to read a Sample from a given Fifo
 * 
 * @param f Fifo from which the element should be rad
 * @param sample Pointer to the Sample where the Fifo-Element should be written
 * @return int Returns the number of elements read from Buffer (can be 0 for none or 1 for one)
 */

int fifo_read_element(fifo_t* f, int32_t* sample);

/**
 * @brief Function to drop n samples from a given fifo without reading it
 * 
 * @param f Fifo from which elements should be dropped
 * @param num_samples the number of elements to be dropped
 */


void fifo_drop_samples(fifo_t* f, int32_t num_samples);