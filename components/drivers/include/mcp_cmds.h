#include "driver/spi_master.h"
//#include "spi_master_lobo.h"

spi_transaction_t mcp_trans[] = {
    {
        .flags = SPI_TRANS_USE_RXDATA  | SPI_TRANS_USE_TXDATA,
        .length = 24,
        .rxlength = 24,
        .tx_data[0] = 0x06,
        .tx_data[1] = 0x00,
        .tx_data[2] = 0x00,
        .tx_data[3] = 0x00
    },
    {
        .flags = SPI_TRANS_USE_RXDATA  | SPI_TRANS_USE_TXDATA,
        .length = 24,
        .rxlength = 24,
        .tx_data[0] = 0x06,
        .tx_data[1] = 64,
        .tx_data[2] = 0x00, 
        .tx_data[3] = 0x00
    },
    {
        .flags = SPI_TRANS_USE_RXDATA  | SPI_TRANS_USE_TXDATA,
        .length = 24,
        .rxlength = 24,
        .tx_data[0] = 0x06,
        .tx_data[1] = 128,
        .tx_data[2] = 0x00,
        .tx_data[3] = 0x00
    },
    {
        .flags = SPI_TRANS_USE_RXDATA  | SPI_TRANS_USE_TXDATA,
        .length = 24,
        .rxlength = 24,
        .tx_data[0] = 0x07,
        .tx_data[1] = 192,
        .tx_data[2] = 0x00,
        .tx_data[3] = 0x00
    },
    {
        .flags = SPI_TRANS_USE_RXDATA  | SPI_TRANS_USE_TXDATA,
        .length = 24,
        .rxlength = 24,
        .tx_data[0] = 0x07,
        .tx_data[1] = 0x00,
        .tx_data[2] = 0x00,
        .tx_data[3] = 0x00
    },
    {
        .flags = SPI_TRANS_USE_RXDATA  | SPI_TRANS_USE_TXDATA,
        .length = 24,
        .rxlength = 24,
        .tx_data[0] = 0x07,
        .tx_data[1] = 64,
        .tx_data[2] = 0x00,
        .tx_data[3] = 0x00
    },
    {
        .flags = SPI_TRANS_USE_RXDATA  | SPI_TRANS_USE_TXDATA,
        .length = 24,
        .rxlength = 24,
        .tx_data[0] = 0x07,
        .tx_data[1] = 128,
        .tx_data[2] = 0x00,
        .tx_data[3] = 0x00
    },
    {
        .flags = SPI_TRANS_USE_RXDATA  | SPI_TRANS_USE_TXDATA,
        .length = 24,
        .rxlength = 24,
        .tx_data[0] = 0x07,
        .tx_data[1] = 192,
        .tx_data[2] = 0x00,
        .tx_data[3] = 0x00
    }
};

/*
        trans[i].flags = SPI_TRANS_USE_RXDATA  | SPI_TRANS_USE_TXDATA;
        trans[i].length = 24;
        trans[i].rxlength = 24;
        trans[i].tx_data[0] = 0x6 | (i >> 2);
        trans[i].tx_data[1] = i << 6;
        trans[i].tx_data[2] = 0x00; trans[i].tx_data[3] = 0x00;
*/