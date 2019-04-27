#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "ui_events.h"
#include "timer_utils.h"

xQueueHandle ui_ev_queue = NULL;
TimerHandle_t xSingleShotTimerHandle = NULL;
TimerHandle_t xTopmenuFeedbackTimerHandle = NULL;
void *_timer_data = NULL;

// The callback function assigned to the one-shot timer.  In this case the
// parameter is not used.
static void timer_callback( TimerHandle_t pxTimer )
{
    ui_ev_ts_t ev;
    ev.event = EV_TIMER_ONE_SHOT;
    //ESP_LOGI("TIMER", "TIMER UP SENDING EVENT");
    xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
}

static void feedback_timer_callback(TimerHandle_t pxTimer){
    static int cnt = 0;
    ui_ev_ts_t ev;
    ev.event = EV_TIMER_MENU_FB;
    ev.event_data = (void*) &cnt;
    xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
    cnt++;
    if(cnt == 4 && xTopmenuFeedbackTimerHandle != NULL){
        xTimerStop(xTopmenuFeedbackTimerHandle, 0);
        cnt = 0;
    }
}

void stopTimerSingleShot(){
    if(xSingleShotTimerHandle != NULL) xTimerStop(xSingleShotTimerHandle, 0);
}

void setTimerSingleShot(int delay, xQueueHandle queueui){
    ui_ev_queue = queueui;
    
    if(xSingleShotTimerHandle == NULL) xSingleShotTimerHandle = xTimerCreate("timer", delay / portTICK_PERIOD_MS, pdFALSE, (void*) 0, timer_callback);
    
    if( xSingleShotTimerHandle == NULL )
    {
        ESP_LOGE("TIMER", "COULD NOT CREATE SINGLE SHOT TIMER");
    }
    else
    {
        if( xTimerStart( xSingleShotTimerHandle, 0 ) != pdPASS )
        {
            ESP_LOGE("TIMER", "COULD NOT START SINGLE SHOT TIMER");
        }
        else{
            //ESP_LOGI("TIMER", "STARTED TIMER");
        }
    }
}

void setTimerTopmenuFeedback(int delay, xQueueHandle queueui){
    ui_ev_queue = queueui;
    if(xTopmenuFeedbackTimerHandle == NULL) xTopmenuFeedbackTimerHandle = xTimerCreate("timer_fb", delay / portTICK_PERIOD_MS, pdTRUE, (void*) 1, feedback_timer_callback);
    
    if( xTopmenuFeedbackTimerHandle == NULL )
    {
        ESP_LOGE("TIMER", "COULD NOT CREATE MENU FEEDBACK TIMER");
    }
    else
    {
        if( xTimerStart( xTopmenuFeedbackTimerHandle, 0 ) != pdPASS )
        {
            ESP_LOGE("TIMER", "COULD NOT START MENU FEEDBACK TIMER");
        }
        else{
            //ESP_LOGI("TIMER", "STARTED TIMER");
        }
    }   
}