#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "fileio.h"
#include "cJSON.h"
#include "storage.h"
#include "preset.h"

static void createConfigFile(){
    cJSON *root = cJSON_CreateObject();
    cJSON *array = cJSON_CreateArray();
    cJSON *val = NULL;
    int i;
    for(i=0; i<2; i++){
        cJSON *obj = cJSON_CreateObject();
        val = cJSON_CreateString("");
        cJSON_AddItemToObject(obj, "name", val);
        val = cJSON_CreateString("");
        cJSON_AddItemToObject(obj, "file", val);
        cJSON_AddItemToArray(array, obj);
    }
    cJSON_AddItemToObject(root, "slots", array);
    cJSON* settings = cJSON_CreateObject();
    val = cJSON_CreateString("myssid");
    cJSON_AddItemToObject(settings, "ssid", val);
    val = cJSON_CreateString("mypasswd");
    cJSON_AddItemToObject(settings, "passwd", val);
    val = cJSON_CreateString("myapikey");
    cJSON_AddItemToObject(settings, "apikey", val);
    val = cJSON_CreateNumber(0);
    cJSON_AddItemToObject(settings, "tz_shift", val);
    val = cJSON_CreateString("");
    cJSON_AddItemToObject(settings, "preset", val);
    val = cJSON_CreateString("");
    cJSON_AddItemToObject(settings, "bank", val);
    cJSON_AddItemToObject(root, "settings", settings);
    writeJSONFile("/sdcard/CONFIG.JSN", cJSON_Print(root));
    cJSON_Delete(root);
}

void mountSDStorage(){
    ESP_LOGI("SD", "Initializing SD card");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // To use 1-line SD mode, uncomment the following line:
    host.flags = SDMMC_HOST_FLAG_1BIT;

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
        esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 10,
        .allocation_unit_size = 8192
    };
    
    sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE("SD", "Failed to mount filesystem. "
                "If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE("SD", "Failed to initialize the card (%d). "
                "Make sure SD card lines have pull-up resistors in place.", ret);
        }
        return;
    }


    // check if directory structure exists, if not, create
    struct stat st = {0};
    if (stat("/sdcard/pool", &st) == -1) {
        mkdir("/sdcard/pool", 0777);
    }

    if (stat("/sdcard/raw", &st) == -1) {
        mkdir("/sdcard/raw", 0777);
    }

    if (stat("/sdcard/banks", &st) == -1) {
        mkdir("/sdcard/banks", 0777);
    }

    if (stat("/sdcard/usr", &st) == -1) {
        mkdir("/sdcard/usr", 0777);
    }

    // check of config file exists, if not, create
    if (stat("/sdcard/CONFIG.JSN", &st) == -1) {
        createConfigFile(); 
    }
    
    // check if default preset bank file exists, if not, create
    if (stat("/sdcard/banks/default.JSN", &st) == -1) {
        initBank("/sdcard/banks/default.JSN");
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
}

void unmountSDStorage(){
    ESP_LOGI("SD", "Initializing SD card");
    esp_vfs_fat_sdmmc_unmount();
}