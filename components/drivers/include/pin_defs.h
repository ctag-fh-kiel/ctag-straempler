#pragma once


// pin definitions from ESP WROVER module
// TFT SPI
#define SPI_TFT_CS_PIN 5
#define SPI_TFT_DC_PIN 26
#define SPI_TFT_MOSI_PIN 23
#define SPI_TFT_SCK_PIN 18
#define SPI_TFT_MISO_PIN 19 // could be used as free output pin e.g. for debugging purposes

// ADC + CODEC
#define SPI_ADC_CS_PIN 32
#define SPI_CODEC_CS_PIN 4
#define SPI_ADC_MISO_PIN 0
#define SPI_ADCCODEC_MOSI_PIN 13
#define SPI_ADCCODEC_SCK_PIN 12
#define I2S_BCLK_PIN 21
#define I2S_LRCLK_PIN 25
#define I2S_ADCDAT_PIN 27
#define I2S_DACDAT_PIN 22

// Trigger inputs
#define TRIG0_PIN 39
#define TRIG1_PIN 36

// UI Encoder
#define ENC_BTN_PIN 34
#ifndef CONFIG_SWAP_ENCODER_PINS
    #define ENC_A_PIN 33
    #define ENC_B_PIN 35
#else 
    #define ENC_A_PIN 35
    #define ENC_B_PIN 33
#endif