#pragma once
#include "esp_wifi.h"

void initWifi(void);
void wifiWaitForConnected();
void restartWifi(wifi_config_t *cfg);