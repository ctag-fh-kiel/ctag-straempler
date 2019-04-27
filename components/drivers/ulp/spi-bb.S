#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/soc_ulp.h"
#include "stack.S"

    // Macros for pin access
    .macro read_MISO 
        READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S + 11, 1) 
    .endm

    .macro clear_SCLK
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + 15, 1, 1)
    .endm
    .macro set_SCLK
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + 15, 1, 1)
    .endm
    .macro toggle_SCLK
        set_SCLK
        clear_SCLK
    .endm

    .macro clear_MOSI
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + 14, 1, 1)
    .endm
    .macro set_MOSI
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + 14, 1, 1)
    .endm

    .macro clear_CS_WM8731
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + 10, 1, 1)
    .endm
    .macro set_CS_WM8731
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + 10, 1, 1)
    .endm

    .macro clear_CS_MCP3208
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + 9, 1, 1)
    .endm
    .macro set_CS_MCP3208
        WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + 9, 1, 1)
    .endm

    .macro transmitWM8731 cmd
        move r2, \cmd
        psr 
        jump transmitSPIWordToWM8731
    .endm


    .set	WM8731_RESET, 0x1E00
    .set    WM8731_SET_MASTER, 0x0E4E
    .set    WM8731_SET_CLOCK, 0x1020
    .set    WM8731_SET_POWERON, 0x0C00
    .set    WM8731_SET_ENABLE, 0x1201
    .set    WM8731_ACTIVATE_LIN, 0x0017
    .set    WM8731_ACTIVATE_RIN, 0x0117
    .set    WM8731_SELECT_CHANNELS, 0x0810
    .set    WM8731_ACTIVATE_LOUT, 0x0A00
    .set    WM8731_SET_LHPLEV, 0x0468
    .set    WM8731_SET_RHPLEV, 0x0668

	/* Define variables, which go into .bss section (zero-initialized data) */
	.bss
	.global stack
stack:
	.skip 100
	.global stackEnd
stackEnd:
	.long 0

	.global adc_data
adc_data:
	.skip 64 					/* 128bit, 8 x 4(byte) */

    .global adc_data_offset
adc_data_offset:
    .long 0


	/* Code goes into .text section */
	.text
	.global entry
entry:
    jump initWM8731
    
    
    .global exit
exit:
	halt


	.global initWM8731
initWM8731:
    // initialize stack operations
    move r3, stackEnd
    // transmit configuration data to audio codec
    transmitWM8731 WM8731_RESET
    transmitWM8731 WM8731_SET_MASTER
    transmitWM8731 WM8731_SET_CLOCK
    transmitWM8731 WM8731_SET_POWERON
    transmitWM8731 WM8731_SET_ENABLE
    transmitWM8731 WM8731_SELECT_CHANNELS
    transmitWM8731 WM8731_ACTIVATE_LIN
    transmitWM8731 WM8731_ACTIVATE_RIN
    transmitWM8731 WM8731_ACTIVATE_LOUT
    transmitWM8731 WM8731_SET_LHPLEV
    transmitWM8731 WM8731_SET_RHPLEV

    // now do the MCP3208
    // r1 will contain adc_data_offset
    move r0, adc_data_offset
    move r1, 0
    st r1, r0, 0
	jump readMCP3208Continous

    // SPI transmission routine for WM8731
    // r2 has word to be transmitted, r3 used for stack
    .global transmitSPIWordToWM8731
transmitSPIWordToWM8731:
    stage_rst
    clear_SCLK
    clear_MOSI
    clear_CS_WM8731
write_loop1:
    clear_SCLK
    and r0, r2, 0x8000
    lsh r2, r2, 1
    jumpr loop1_bit0, 1, lt
	set_MOSI
	jump loop1_bit1
loop1_bit0:
	clear_MOSI
loop1_bit1:
	set_SCLK
	stage_inc 1
	jumps write_loop1, 16, lt
	clear_SCLK
    set_CS_WM8731
    ret

    .global readMCP3208Continous
readMCP3208Continous:
    // read channel 0
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    clear_MOSI
    toggle_SCLK
    toggle_SCLK
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop0:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop0, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2 // 4096 minus ADC value due to electronic design
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 0

    // read channel 1
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    clear_MOSI
    toggle_SCLK
    toggle_SCLK
    set_MOSI
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop1:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop1, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 4

    // read channel 2
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    clear_MOSI
    toggle_SCLK
    set_MOSI
    toggle_SCLK
    clear_MOSI
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop2:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop2, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 8

   // read channel 3
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    clear_MOSI
    toggle_SCLK
    set_MOSI
    toggle_SCLK
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop3:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop3, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 12

   // read channel 4
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    toggle_SCLK
    clear_MOSI
    toggle_SCLK
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop4:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop4, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 16

   // read channel 5
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    toggle_SCLK
    clear_MOSI
    toggle_SCLK
    set_MOSI
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop5:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop5, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 20

   // read channel 6
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    toggle_SCLK
    toggle_SCLK
    clear_MOSI
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop6:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop6, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 24

   // read channel 7
    clear_CS_MCP3208
    // start bit & single ended
    set_MOSI
    toggle_SCLK
    toggle_SCLK
    // set address bits
    toggle_SCLK
    toggle_SCLK
    toggle_SCLK 
    // wait cycle
    toggle_SCLK 
    // read data
    move r2, 0
    move r3, 4096
    toggle_SCLK // null bit
    stage_rst
adc_read_loop7:
    lsh r2, r2, 1
    set_SCLK // msb
    read_MISO
    or r2, r2, r0 
    clear_SCLK
    stage_inc 1
    jumps adc_read_loop7, 12, lt
    // done
    set_CS_MCP3208
    // store data
    sub r2, r3, r2
    move r0, adc_data
    add r0, r0, r1 // add offset
    st r2, r0, 28

    // set offset in storage
    move r0, adc_data_offset
    st r1, r0, 0
    // send interrupt and restart loop
    wake
    // change offset
    //add r1, r1, 32
    //and r1, r1, 0x20 // remove all bits except for 32
    wait 500
    //wait 530 // needs 725.623 us sample period to match 1/44100 * 32 I2S buffer size
    jump readMCP3208Continous



