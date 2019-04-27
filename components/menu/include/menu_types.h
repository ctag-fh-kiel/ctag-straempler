#pragma once


// has to start counting at 1, return id 0 from callback results in remaining in same menu
typedef enum{
    M_MAIN = 0x01,
    M_PLAY,
    M_SLOT,
    M_SLOT_TYPESELECT,
    M_SLOT_FILEBROWSER,
    M_SLOT_DECODING,
    M_SLOT_USER,
    M_BROWSE,
    M_BROWSE_TAG,
    M_BROWSE_TAG_RESULTS,
    M_BROWSE_SEARCH,
    M_BROWSE_ID,
    M_BROWSE_ID_RESULT,
    M_BROWSE_USER,
    M_MORE,
    M_SETTINGS,
    M_SETTINGS_INPUT,
    M_ABOUT,
    M_VOICE,
    M_VOICE0,
    M_VOICE1,
    M_EXTERNAL_IN,
    M_EFFECTS,
    M_CV_MATRIX,
    M_DELAY,
    M_REVERB,
    M_FILTER,
    M_ADSR,
    M_PLAYMODE,
    M_PRESET,
    M_PRESET_P,
    M_PRESET_B,
    M_PRESET_LOAD,
    M_PRESET_STORE,
    M_PRESET_NEW,
    M_PRESET_RESET,
    M_PRESET_DELETE,
    M_PRESET_BANK_LOAD,
    M_PRESET_BANK_NEW,
    M_PRESET_BANK_DELETE
} menu_ids_t;

typedef enum{
    SID_TRIG_TYPE,
    SID_VOLUME,
    SID_PAN,
    SID_PITCH,
    SID_PITCH_CV_ACTIVE,
    SID_PBSPEED,
    SID_DIST_ACTIVE,
    SID_DIST,
    SID_DELAY_SEND,
    SID_DELAY_ACTIVE, 
    SID_DELAY_MODE,
    SID_DELAY_TIME,
    SID_DELAY_PAN,
    SID_DELAY_FEEDBACK,
    SID_DELAY_VOLUME,
    SID_FILTER_ACTIVE,
    SID_BASE,
    SID_WIDTH,
    SID_Q,
    SID_ATTACK,
    SID_DECAY,
    SID_SUSTAIN,
    SID_RELEASE,
    SID_MODE,
    SID_START,
    SID_LSTART,
    SID_LEND,
    SID_LPOSITION,
    SID_EXTIN_ACTIVE,
    SID_EXTIN_VOLUME,
    SID_EXTIN_PAN,
    SID_EXTIN_DELAY_SEND, 
    SID_WIFI_SSID,
    SID_WIFI_PASSWD,
    SID_APIKEY,
    SID_TIMEZONE
} submenu_ids_t;

typedef enum{
    PRINT_ALL = 0xFF,
    PRINT_FAST,
    PRINT_CLEAR,
    PRINT_UPPER,
    PRINT_NORM
} print_ids_t;

typedef struct{
    int x;
    int y; 
}p2d_t;
