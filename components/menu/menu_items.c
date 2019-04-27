#include "menu_items.h"

const char* toggle_items[] = {"OFF", "ON"};
const int n_toggle_items = sizeof(toggle_items) / sizeof(int);

const char* trig_items[] = {"Gate", "Latch"};
const int n_trig_items = sizeof(trig_items) / sizeof(int);

const char* main_menus[] = {"Play", "Slot", "Sample", "Preset", "More"};
const int n_main_menus = sizeof(main_menus)/sizeof(int);

const char* browse_menus[] = {"Tag", "ID", "Text", "User"};
const int n_browse_menus = sizeof(browse_menus)/sizeof(int);

const char* play_menus[] = {"Voice 0", "Voice 1", "Effects", "CV Matrix"};
const int n_play_menus = sizeof(play_menus)/sizeof(int);

const char* effect_menus[] = {"Delay", "External In"};
const int n_effect_menus = sizeof(effect_menus) / sizeof(int);

const char* voice_menus[] = {"Trig Type", "Volume", "Pan", "Pitch", "Pitch CV", "Playback Speed", "Distortion", "Drive", "Delay Send", "Filter", "ADSR", "Playmodes"};
const int n_voice_menus = sizeof(voice_menus) / sizeof(int);

const char* externel_in_menus[] = {"EXTIN Active", "Volume", "Pan", "Delay Send"};
const int n_externel_in_menus = sizeof(externel_in_menus) / sizeof(int);

const char* filter_menus[] = {"Filter Active", "Base" , "Width", "Q"};
const int n_filter_menus = sizeof(filter_menus)/ sizeof(int);

const char* adsr_menus[] = {"Attack", "Decay", "Sustain", "Release"};
const int n_adsr_menus = sizeof(adsr_menus) / sizeof(int);

const char* playmode_menus[] = {"Mode", "Start", "Loop Start", "Loop End", "Loop Position"};
const char* playmode_modes[] = {"Single Shot", "Loop", "Ping Pong"};
const int n_playmode_menus = sizeof(playmode_menus) / sizeof(int);
const int n_play_modes = sizeof(playmode_modes) / sizeof(int);

const char* delay_menus[] = {"Delay Active", "Delay Mode", "Delay Time", "Delay Pan", "Delay Feedback", "Delay Volume"};
const int n_delay_menus = sizeof(delay_menus) / sizeof(int);

const char* delay_mode_items[] = {"Stereo", "Ping Pong"};
const int n_delay_mode_items = sizeof(toggle_items) / sizeof(int);

const char* cv_matrix_items[] = {"SOURCE", "AMOUNT", "DESTINATION"};
const int n_cv_matrix_items = sizeof(cv_matrix_items) / sizeof(int);

const char* matrix_parameter_items[] = {"None","V0 Volume", "V0 Pan", "V0 Pitch", "V0 PB Speed", "V0 Dist Drive", "V0 Dly Send", "V0 F. Base", "V0 F. Width", "V0 F. Q", "V0 Attack", 
                                                "V0 Decay", "V0 Sustain", "V0 Release", "V0 Start", "V0 Loop Start", "V0 Loop End", "V1 Volume", "V1 Pan", "V1 Pitch", "V1 PB Speed",
                                                "V1 Dist Drive", "V1 Dly Send", "V1 F. Base", "V1 F. Width", "V1 F. Q", "V1 Attack", "V1 Decay", "V1 Sustain", "V1 Release", "V1 Start",
                                                "V1 Loop Start", "V1 Loop End", "Dly Time", "Dly Pan", "Dly Feedback", "Dly Volume"};
const int n_matrix_parameters = sizeof(matrix_parameter_items) / sizeof(int);

const char* preset_menus[] = {"Presets", "Banks"};
const int n_preset_menus = sizeof(preset_menus)/sizeof(int);

const char* preset_choices[] = {"Load", "Save", "New", "Delete", "Reset"};
const int n_preset_choices = sizeof(preset_choices)/sizeof(int);

const char* bank_choices[] = {"Load", "New", "Delete"};
const int n_bank_choices = sizeof(bank_choices)/sizeof(int);

const char* choices[] = {"Actually No:(", "Totally Sure!"};
const int n_choices = sizeof(choices) / sizeof(int);

const char* more_menus[] = {"Settings", "About"};
const int n_more_menus = sizeof(more_menus) / sizeof(int);

const char* slot_menus[] = {"Freesound", "User"};
const int n_slot_menus = sizeof(slot_menus)/sizeof(int);

const char* settings_menus[] = {"SSID", "Password", "Api Key", "Timezone"};
const int n_settings_menus = sizeof(settings_menus)/sizeof(int);