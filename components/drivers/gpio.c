#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include <time.h>
#include <sys/time.h>
#include "gpio.h"
#include "pin_defs.h"
#include "ui_events.h"
#include "audio_events.h"

#define ESP_INTR_FLAG_DEFAULT 0

static xQueueHandle ui_event_queue = NULL;

static void IRAM_ATTR gpio_isr_handler_encoder1(void* arg)
{
    ui_ev_ts_t ev;
    uint32_t pin = (uint32_t) arg;
    uint32_t pv = (uint32_t) gpio_get_level(pin);
    uint32_t a = gpio_get_level(ENC_A_PIN);
    uint32_t b = gpio_get_level(ENC_B_PIN);
    static uint32_t check = 0;
    // de bounce
    if(pv) return;
    if(pin == ENC_B_PIN && a){
        check = ENC_B_PIN;
        return;
    }
    if(pin == ENC_A_PIN && b){
        check = ENC_A_PIN;
        return;
    }
    // real event
    if(check == ENC_A_PIN && !a && !b){
        ev.event = EV_ENC1_BWD;
        xQueueSendFromISR(ui_event_queue, &ev, NULL);
    }
    if(check == ENC_B_PIN && !a && !b){
        ev.event = EV_ENC1_FWD;
        xQueueSendFromISR(ui_event_queue, &ev, NULL);
    }
    check = 0;
    return;
}

static void IRAM_ATTR gpio_isr_handler_encoder1_btn1(void* arg)
{
    ui_ev_ts_t ev;
    static ui_ev_t state = EV_ENC1_BT_UP;
    uint32_t pv = gpio_get_level(ENC_BTN_PIN);
    // de-bounce
    if(pv == 0 && state == EV_ENC1_BT_DWN) return;
    if(pv == 1 && state == EV_ENC1_BT_UP) return;
    // real event
    if(pv == 1 && state == EV_ENC1_BT_DWN) ev.event = EV_ENC1_BT_UP;
    if(pv == 0 && state == EV_ENC1_BT_UP) ev.event = EV_ENC1_BT_DWN;
    state = ev.event;
    xQueueSendFromISR(ui_event_queue, &ev, NULL);
}

void initGPIO(xQueueHandle queueui){
    // set queues
    ui_event_queue = queueui;
    
    gpio_config_t io_conf;

    //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
    //bit mask of the pins
    io_conf.pin_bit_mask = ((1ULL<<ENC_A_PIN) | (1ULL<<ENC_B_PIN) | (1ULL<<ENC_BTN_PIN));
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    //change gpio intrrupt type for one pin
    gpio_set_intr_type(ENC_BTN_PIN, GPIO_INTR_ANYEDGE);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for encoder 1
    gpio_isr_handler_add(ENC_A_PIN, gpio_isr_handler_encoder1, (void*) ENC_A_PIN);
    gpio_isr_handler_add(ENC_B_PIN, gpio_isr_handler_encoder1, (void*) ENC_B_PIN);
    //hook isr handler for encoder 1 button
    gpio_isr_handler_add(ENC_BTN_PIN, gpio_isr_handler_encoder1_btn1, (void*) ENC_BTN_PIN);

    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = ((1ULL<<TRIG0_PIN)| (1ULL<<TRIG1_PIN));
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    /* debug pin
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = 1ULL<<SPI_TFT_MISO_PIN;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
    */
}
