#pragma once

typedef enum{
    EV_ENTERED_MENU = -1,
    EV_NONE = 0x0,
    EV_ENC1_FWD = 0x01,
    EV_ENC1_BWD = 0x02,
    EV_ENC1_BT_DWN = 0x04,
    EV_ENC1_BT_UP = 0x08,
    EV_TIMER_REPEATING_SLOW = 0x10,
    EV_TIMER_REPEATING_FAST,
    EV_TIMER_ONE_SHOT,
    EV_TIMER_COMPLETE,
    EV_TIMER_MENU_FB,
    EV_FWD,
    EV_BWD,
    EV_SHORT_PRESS,
    EV_LONG_PRESS,
    EV_FREESND_TAGLIST,
    EV_FREESND_MP3_COMPLETE,
    EV_FREESND_NOT_FOUND,
    EV_PROGRESS_UPDATE,
    EV_DECODING_PROGRESS,
    EV_DECODING_DONE, 
    EV_UPDATE_V0_POS,
    EV_UPDATE_V1_POS
} ui_ev_t;

typedef struct{
    ui_ev_t event;
    void * event_data;
    //struct timeval timestamp;
} ui_ev_ts_t;

typedef struct {
    xQueueHandle ui_evt_queue;
    void *user_data;
} ui_handler_param_t;