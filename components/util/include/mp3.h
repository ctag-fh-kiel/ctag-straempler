#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void initMP3Engine(xQueueHandle);
void decodeMP3File(const char *id);

//void decodeMP3(unsigned char* inputData, unsigned int len, FILE* outfile);