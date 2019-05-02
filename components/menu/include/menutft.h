#pragma once
#include "cJSON.h"
#include "audio.h"
#include "menu_types.h"
#include "menu_shapes.h"
#include "list.h"
#include "menu_envelope.h"
#include "menu_playmode.h"
#include "timer_utils.h"
#include "menu_utils.h"

//Printing menu functions
void menuTFTPrintMenu(const char** items, const int* n_items);
void menuTFTPrintMenuH(const char** items, const int* n_items);
void menuTFTPrintMenuHSpaced(const char** items, const int* n_items);
void menuTFTPrintMainMenus();
void menuTFTPrintAbout();
void menuTFTPrintVoiceMenu();
void menuTFTPrintADSRMenu();
void menuTFTPrintPlaymodeMenu();
void menuTFTPrintMatrixMenu();
void menuTFTPrintSlotMenu(const cJSON *slotData, int activeSlot);
void menuTFTPrintSettings(const cJSON *data);
void menuTFTPrintBrowseTextMenu();
void menuTFTPrintLoadingTagMenu();
void menuTFTPrintSelectIDMenu();
void menuTFTPrintPresetMenu(const char** items, const int* n_items);
void menuTFTPrintPresetLayout();
void menuTFTPrintInputMenu(char* title);
void menuTFTPrintPresetList(cJSON* bank, print_ids_t action);
void menuTFTPrintBankList(list_t* list, print_ids_t action);
void menuTFTPrintUserFileMenu();


//Selecting menu item functions
void menuTFTSelectMenuItem(int* activeSlot, int selected, const char** items, const int* n_items);
void menuTFTSelectMenuItemH(int* activeSlot, int selected, const char** items, const int* n_items);
void menuTFTSelectMenuItemHSpaced(int* activeSlot, int selected, const char** items, const int* n_items);
void menuTFTSelectMainMenu(int, int);
void menuTFTSelectVoiceMenu(int, int);
void menuTFTSelectMatrixItem(int active, int select, int column);
void menuTFTSelectPreset(int* activeSlot, cJSON* rootArray, int selected);
void menuTFTSelectPresetBank(int* activeSlot, list_t* list, int selected);


//Print parameter values
void menuTFTPrintVoiceValues(param_data_t* data, int);
void menuTFTPrintADSRValues(adsr_data_t* data, int);
void menuTFTPrintFilterValues(filter_data_t* data, int);
void menuTFTPrintPlaymodeValues(play_state_data_t* data, int);
void menuTFTPrintMatrixAmount(matrix_ui_row_t* matrix, int);
void menuTFTPrintMatrixDestination(matrix_ui_row_t* matrix, int);
void menuTFTPrintDelayValues(delay_data_t* data, int);
void menuTFTPrintExtInValues(ext_in_data_t* data, int);


//Additional UI
void menuTFTPrintTime(int*);
void menuTFTPrintTimezone(const char** items, const int* n_items, int *shift);
void menuTFTPrintPlaymodeIndicators(play_state_data_t* data, int);
void menuTFTInitPlaymodeIndicators();
void menuTFTPrintADSRCurve(adsr_data_t* data);
void menuTFTPrintMatrixRowIndicator(int select, int row);
void menuTFTInitAdsrCurve();
void menuTFTPrintCurrentSettings(char*, char*, param_data_t*);
void menuTFTPrintCurrentPresetSettings(char* title, char* data);
void menuTFTFeedbackMenuItemHSpaced(int *cnt, int* pos, const char** items, const int* n_items, int state);

//Utility
void menuTFTUpdateProgress(char *text, int progress);
void menuTFTPrintBrowseTagList(list_t*);
int menuTFTHighlightNextEl();
int menuTFTHighlightPrevEl();
void menuTFTPrintCharFix(char, int);
void menuTFTPrintChar(char* str, int pos, char c, print_ids_t id);
int menuTFTPrintAllCharSettings(char*);
void menuTFTPrintFileBrowser(int currentFile, int maxFiles, const cJSON* desc);
void menuTFTPrintDecoding();
void menuTFTAnimateFileBrowser(const cJSON* desc);
int printSubStringIfTooWide(char *s, int x, int y, int pos);
int printTags(list_item_t* it);
void menuTFTUpdatePlayState();
void menuTFTPrintInputError(char*);
void menuTFTClearListItem(int* activeSlot);
void menuTFTResetTextWrap();
