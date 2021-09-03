#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <time.h>
#include <sys/time.h>
#include "ui.h"
#include "ui_events.h"
#include "storage.h"
#include "gpio.h"
#include "pin_defs.h"
#include "tft.h"
#include "menu.h"
#include "freesound.h"
#include "mp3.h"
#include "wifi.h"
#include "c_timeutils.h"
#include "timer_utils.h"
#include "audio.h"
#include "rest-api.h"

#define SPI_BUS TFT_HSPI_HOST

static TaskHandle_t *ui_task;
static xQueueHandle ui_ev_queue = NULL;
static xQueueHandle ui_param_queue_v1 = NULL;
static xQueueHandle ui_param_queue_v0 = NULL;
static xQueueHandle effect_param_queue = NULL;
static xQueueHandle pbs_state_queue_v0 = NULL;
static xQueueHandle pbs_state_queue_v1 = NULL;
static xQueueHandle mode_queue_v0 = NULL;
static xQueueHandle mode_queue_v1 = NULL;
static xQueueHandle matrix_event_queue = NULL;

static void ui_ev_loop(void* pvParams)
{
    ui_handler_param_t *params = pvParams;
    xQueueHandle ui_evt_queue = params->ui_evt_queue;
    ui_ev_ts_t ev;
    struct timeval lastBtnEv;
    uint32_t last = 0;  

    gettimeofday(&lastBtnEv, NULL);

    for(;;) {
        if(xQueueReceive(ui_evt_queue, &ev, portMAX_DELAY)) {
            
            static ui_ev_t btn_state = EV_NONE, btn_serviced = 0; 
            switch(ev.event){
                case EV_ENC1_FWD:
                    menuProcessEvent(EV_FWD, NULL);
                    break;
                case EV_ENC1_BWD:
                    menuProcessEvent(EV_BWD, NULL);
                    break;
                case EV_ENC1_BT_DWN:
                    last = timeval_durationBeforeNow(&lastBtnEv);
                    //ESP_LOGI("UI", "DOWN %u", last);
                    if(last < 120 || btn_state == EV_ENC1_BT_DWN) break;
                    gettimeofday(&lastBtnEv, NULL);
                    btn_state = EV_ENC1_BT_DWN;
                    btn_serviced = 0;
                    setTimerSingleShot(500, ui_ev_queue);       
                    break;
                case EV_ENC1_BT_UP:
                    last = timeval_durationBeforeNow(&lastBtnEv);
                    //ESP_LOGI("UI", "UP %u", last);
                    if(last < 20 || btn_state == EV_ENC1_BT_UP) break;
                    gettimeofday(&lastBtnEv, NULL);
                    btn_state = EV_ENC1_BT_UP;
                    if(!btn_serviced){
                        btn_serviced = 1;
                        menuProcessEvent(EV_SHORT_PRESS, NULL);
                    }
                    break;
                case EV_TIMER_ONE_SHOT:
                    // ESP_LOGI("UI", "TIMER ONE SHOT, serviced: %d", btn_serviced);
                    if(!btn_serviced){
                        btn_serviced = 1;
                        menuProcessEvent(EV_LONG_PRESS, NULL);
                    }
                    menuProcessEvent(EV_TIMER_COMPLETE, NULL);
                    break;
                default:
                    menuProcessEvent(ev.event, ev.event_data);
                    break;
            }
        }
    }
    vTaskDelete(NULL);
}

void configDisplay(){
    esp_err_t ret;
    // set up TFT
    TFT_PinsInit();
    // ====  CONFIGURE SPI DEVICES(s)  ====================================================================================

    spi_lobo_device_handle_t spi;
    
    spi_lobo_bus_config_t buscfg={
        .miso_io_num=SPI_TFT_MISO_PIN,				// set SPI MISO pin
        .mosi_io_num=SPI_TFT_MOSI_PIN,				// set SPI MOSI pin
        .sclk_io_num=SPI_TFT_SCK_PIN,				// set SPI CLK pin
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz = 6*1024,
    };
    spi_lobo_device_interface_config_t devcfg={
        .clock_speed_hz=8000000,                // Initial clock out at 8 MHz
        .mode=0,                                // SPI mode 0
        .spics_io_num=-1,                       // we will use external CS pin
        .spics_ext_io_num=SPI_TFT_CS_PIN,           // external CS pin
        .flags=LB_SPI_DEVICE_HALFDUPLEX,           // ALWAYS SET  to HALF DUPLEX MODE!! for display spi
    };
    vTaskDelay(500 / portTICK_RATE_MS);
    // ==== Initialize the SPI bus and attach the LCD to the SPI bus ====

	ret=spi_lobo_bus_add_device(SPI_BUS, &buscfg, &devcfg, &spi);
    assert(ret==ESP_OK);
	printf("SPI: display device added to spi bus (%d)\r\n", SPI_BUS);
	disp_spi = spi;

	// ==== Test select/deselect ====
	ret = spi_lobo_device_select(spi, 1);
    assert(ret==ESP_OK);
	ret = spi_lobo_device_deselect(spi);
    assert(ret==ESP_OK);

	printf("SPI: attached display device, speed=%u\r\n", spi_lobo_get_speed(spi));
    printf("SPI: bus uses native pins: %s\r\n", spi_lobo_uses_native_pins(spi) ? "true" : "false");
    
    printf("SPI: display init...\r\n");
    TFT_display_init();
    printf("OK\r\n");
	
	// ---- Detect maximum read speed ----
	max_rdclock = find_rd_speed();
	printf("SPI: Max rd speed = %u\r\n", max_rdclock);

    // ==== Set SPI clock used for display operations ====
	spi_lobo_set_speed(spi, DEFAULT_SPI_CLOCK);
    printf("SPI: Changed speed to %u\r\n", spi_lobo_get_speed(spi));
    TFT_setGammaCurve(DEFAULT_GAMMA_CURVE);
	TFT_setRotation(LANDSCAPE_FLIP);
	TFT_setFont(DEFAULT_FONT, NULL);
    TFT_resetclipwin();
    _fg = TFT_CYAN;
    //TFT_print("Freesound Sampler", CENTER, CENTER); 
    struct stat st = {0};
    if (stat("/sdcard/bootlogo.bmp", &st) != -1) {
        //ESP_LOGE("Boot", "logo file %s", "/sdcard/bootlogo.bmp");
        TFT_bmp_image(CENTER, CENTER, 0, "/sdcard/bootlogo.bmp", NULL, 0);
        vTaskDelay(3000 / portTICK_RATE_MS);
    }
    
}

static void timerRepeatSlow(){
    ui_ev_ts_t ev;
    for(;;){
        //ESP_LOGI("", "Tick");
        ev.event = EV_TIMER_REPEATING_SLOW;
        xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

static void timerRepeatFast(){
    ui_ev_ts_t ev;
    for(;;){
        //ESP_LOGI("", "Tick");
        ev.event = EV_TIMER_REPEATING_FAST;
        xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
        vTaskDelay(300 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void initUI(){
    //Init queue size
    ui_param_queue_v0 = xQueueCreate(1, sizeof( param_data_t));
    ui_param_queue_v1 = xQueueCreate(1, sizeof( param_data_t));
    effect_param_queue = xQueueCreate(1, sizeof(effect_data_t));
    pbs_state_queue_v0 = xQueueCreate(1, sizeof(bool));
    pbs_state_queue_v1 = xQueueCreate(1, sizeof(bool));
    mode_queue_v0 = xQueueCreate(1, sizeof(play_state_data_t));
    mode_queue_v1 = xQueueCreate(1, sizeof(play_state_data_t));
    matrix_event_queue = xQueueCreate(10, sizeof(matrix_event_t));
    ui_ev_queue = xQueueCreate(64, sizeof(ui_ev_ts_t));
    ui_handler_param_t *params = calloc(1, sizeof(ui_handler_param_t));
    params->ui_evt_queue = ui_ev_queue;
    params->user_data = NULL;

    mountSDStorage();
    configDisplay();
    initGPIO(ui_ev_queue);
    
    initAudio(ui_param_queue_v0, ui_param_queue_v1, effect_param_queue, 
        pbs_state_queue_v0, pbs_state_queue_v1, mode_queue_v0, mode_queue_v1, 
        matrix_event_queue, ui_ev_queue);

    initMenu(ui_param_queue_v0, ui_param_queue_v1, effect_param_queue, pbs_state_queue_v0, pbs_state_queue_v1, mode_queue_v0, mode_queue_v1, matrix_event_queue, ui_ev_queue);

    //xTaskCreatePinnedToCore(ui_task, "ui_task", usStackDepth, params, 10, gpio_task, 1);
    xTaskCreatePinnedToCore(ui_ev_loop, "ui_ev_loop", 4096*2, params, 11, ui_task, 0);
    
    //initGPIO(ui_ev_queue, au_q);
    xTaskCreatePinnedToCore(timerRepeatSlow, "timerRepeatSlow", 2048, NULL, 10, NULL, 0);
    xTaskCreatePinnedToCore(timerRepeatFast, "timerRepeatFast", 2048, NULL, 10, NULL, 0);
    
    initWifi();
    freesoundInit(ui_ev_queue);
    initMP3Engine(ui_ev_queue);
    startRestAPI(ui_ev_queue);
}