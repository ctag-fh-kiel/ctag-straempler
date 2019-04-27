#include "sampler_fifo.h"
#include "stdio.h"

void fifo_init(fifo_t* f, int32_t* buf, size_t size){
    f->head = 0;
    f->tail = 0;
    f->free_slots = f->size = size;
    f->buffer = buf;
}


int fifo_pop_element(fifo_t* f, int32_t* sample){
    int i = 0;

    if(f->free_slots < f->size) // check if it is possible to get an element
    {
        *sample = f->buffer[f->tail++];
        f->free_slots++;
        
        if(f->tail == f->size) // check for wrap around
            f->tail = 0;
        i++;
    }
    
    return i; //number of elements read, is 0 or 1
}


int fifo_write(fifo_t* f, const int32_t* buf, int n_int_32_elements){
    int i = 0;
    
    for(; i < n_int_32_elements; i++)
    {
        if(f->free_slots > 0) // check if it is possible to write elements to fifo
        {
            f->buffer[f->head++] = buf[i];
            f->free_slots--;
            
            if(f->head == f->size)
                f->head = 0;
        }
        else
            break;
    }
    return i;
}


int fifo_read_element(fifo_t* f, int32_t* sample)
{
    int i = 0;
    
    if(f->free_slots < f->size) //see if any data is available
    {
        *sample = f->buffer[f->tail];  //grab an int32_t element from the buffer
        i++;
    }
    
    return i; //number of elements read, is 0 or 1
}

void fifo_drop_samples(fifo_t* f, int32_t num_samples)
{
    
    for(int i = 0; i < num_samples; i++)
    {
        if(f->free_slots < f->size) //see if any data is available
        {
            f->tail++;  //increment the tail
            f->free_slots++;
            if( f->tail == f->size ) //check for wrap-around
            {
                f->tail = 0;
            }
        }
    }
}

