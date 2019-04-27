#pragma once

#define EV_VOICE0 0
#define EV_VOICE1 1

typedef enum{
    EV_TRG_NONE = 0x00,
    EV_TRG_DOWN = 0x01,
    EV_TRG_UP = 0x02
} audio_ev_t;