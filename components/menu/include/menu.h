#pragma once
#include "cJSON.h"

void initMenu();
void menuProcessEvent(int ev, void * ev_data);
void initParams();
cJSON* buildPreset();
void loadParams(cJSON *filename);

/*
void fwdMenu();
void bwdMenu();
void selectMenuLong();
void selectMenuShort();
void updateMenuTime();
void updateTagList(void*);
*/