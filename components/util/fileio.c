#include "fileio.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "preset.h"
#include "menu_config.h"
#include "esp_log.h"
#include "errno.h"
#include "esp_system.h"
#include "esp_heap_caps.h"

/*
static int print_list_elements(list_item_t* it){
    printf("%s\n", (char*)it->value);
    return 0;
}
*/

static void createSilenceFile(){
    ESP_LOGI("SD", "Creating silence file");
    FILE * f = fopen("/sdcard/SILENCE.RAW", "wb");
    uint32_t zero = 0;
    for(int i=0;i<8192*4;i++){
        fwrite(&zero, sizeof(uint32_t), 1, f);
    }
    fclose(f);
}

static void createConfigFile(){
    ESP_LOGI("SD", "Creating config file");
    createSilenceFile();
    cJSON *root = cJSON_CreateObject();
    cJSON *array = cJSON_CreateArray();
    cJSON *val = NULL;
    int i;
    for(i=0; i<2; i++){
        cJSON *obj = cJSON_CreateObject();
        val = cJSON_CreateString("SILENCE");
        cJSON_AddItemToObject(obj, "name", val);
        val = cJSON_CreateString("/SILENCE.RAW");
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

void checkSDStructure(){
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

    // check if config file exists & has valid structure, else create/rewrite
    if (stat("/sdcard/CONFIG.JSN", &st) == -1 || validateConfig() == -1) {
        ESP_LOGE("SD", "Config not found/invalid");
        createConfigFile(); 
    }
    
    // check if default preset bank file exists, if not, create
    if (stat("/sdcard/banks/default.JSN", &st) == -1) {
        initBank("/sdcard/banks/default.JSN");
    }
}

void repairAudioFileAssigment(int id){
    cJSON* cfgData = readJSONFileAsCJSON("/sdcard/CONFIG.JSN");
    cJSON *slots = cJSON_GetObjectItem(cfgData, "slots");
    cJSON *fileObj = cJSON_GetArrayItem(slots, id);
    char *name = cJSON_GetObjectItemCaseSensitive(fileObj, "name")->valuestring;
    char filename[128];
    struct stat st = {0};
    // clean up all file in doubt
    // check if raw file exists
    snprintf(filename, 128, "/sdcard/RAW/%s.RAW", name);
    if (stat(filename, &st) == -1){
        ESP_LOGE("SD repair", "Audio raw file %s not found", filename);
    }else{
        remove(filename);
    }
    // check if mp3 file exists
    snprintf(filename, 128, "/sdcard/POOL/%s.MP3", name);
    if (stat(filename, &st) == -1){
        ESP_LOGE("SD repair", "Audio mp3 file %s not found", filename);
    }else{
        remove(filename);
    }
    // check if descriptor file exists
    snprintf(filename, 128, "/sdcard/POOL/%s.JSN", name);
    if (stat(filename, &st) == -1){
        ESP_LOGE("SD repair", "Audio jsn file %s not found", filename);
    }else{
        remove(filename);
    }
    // check if user file exists
    snprintf(filename, 128, "/sdcard/USR/%s.RAW", name);
    if (stat(filename, &st) == -1){
        ESP_LOGE("SD repair", "Audio usr file %s not found", filename);
    }else{
        remove(filename);
    }
    // check if user file descriptor exists
    snprintf(filename, 128, "/sdcard/USR/%s.JSN", name);
    if (stat(filename, &st) == -1){
        ESP_LOGE("SD repair", "Audio usr file %s not found", filename);
    }else{
        remove(filename);
    }
    // re-create initial silence file
    createSilenceFile();
    // set file name config to initial silence file
    cJSON_ReplaceItemInObjectCaseSensitive(fileObj, "name", cJSON_CreateString("SILENCE"));
    cJSON_ReplaceItemInObjectCaseSensitive(fileObj, "file", cJSON_CreateString("/SILENCE.RAW"));
    writeJSONFile("/sdcard/CONFIG.JSN", cJSON_Print(cfgData));
}

void printHeapInfo(const char* c){
    printf("%s Free external Heap - %d\n", c, heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    printf("%s Free internal Heap - %d\n",c, heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
}

int getFileSize(const char *file){
    struct stat st;
    stat(file, &st);
    return st.st_size;
}

int fileExists(const char *file){
    struct stat st = {0};
    // check of config file exists, if not, create
    if (stat(file, &st) == -1) {
        return 0;
    }
    return st.st_size;
}

// reads json file and returns cJSON structure, adds tags_s item containing tags as string
cJSON* readJSONFileAsCJSON(const char *fileName){
    struct stat st;
    int sz, cnt;
    char *buf;
    FILE *fin;
    cJSON *data;
    stat(fileName, &st);
    sz = st.st_size;
    //ESP_LOGI("FILEIO", "Trying to read %s as CJSON File | %s, Filesize: %d", fileName, strerror(errno), sz);

    fin = fopen(fileName, "rb");
    if(fin == NULL){
        ESP_LOGE("FILEIO", "Could not open file %s for reading", fileName);
        return NULL;
    }
    buf = (char*) heap_caps_malloc(sz, MALLOC_CAP_SPIRAM);

    if(buf == NULL){
        ESP_LOGE("FILEIO", "Could not allocate memory");
        fclose(fin);
        return NULL;
    }

    cnt = fread(buf, 1, sz, fin);
    if(cnt != sz){
        ESP_LOGE("FILEIO", "Error reading from file");
        free(buf);
        fclose(fin);
        return NULL;
    }

    fclose(fin);
    data = cJSON_Parse(buf);
    heap_caps_free(buf);

    return data;
}

void parseJSONAudioTags(cJSON* data){
    // adding string with tags
    cJSON *tags = cJSON_GetObjectItem(data, "tags");
    int n = cJSON_GetArraySize(tags), i, len=0;
    if(n == 0){
        cJSON_AddStringToObject(data, "tags_s", "### no tags ###");
        return;
    }

    char *wrPtr;
    char *buf;
    for(i=0;i<n;i++){
        len += strlen(cJSON_GetArrayItem(tags, i)->valuestring) + 2;
    }
    buf = calloc(1, len);
    wrPtr = buf;
    for(i=0;i<n;i++){
        len = strlen(cJSON_GetArrayItem(tags, i)->valuestring);
        memcpy(wrPtr, cJSON_GetArrayItem(tags, i)->valuestring, len);
        wrPtr += len;
        *wrPtr++ = ',';
        *wrPtr++ = ' ';
    }
    wrPtr -= 2;
    *wrPtr = '\0';
    cJSON_AddStringToObject(data, "tags_s", buf);
    free(buf);
}

void writeJSONFile(const char *fileName, const char* data){
    FILE *fout = fopen(fileName, "wb");
    int len;
    if(fout == NULL){
        ESP_LOGE("FILEIO", "Could not open file %s for writing", fileName);
        return;
    }

    //ESP_LOGI("FILEIO", "Write - Free heap: %u", esp_get_free_heap_size());
    if(data != NULL){
        len = strlen(data);
        //ESP_LOGI("FILEIO", "File string length: %d Byte", len);
        fwrite(data, 1, len, fout);

        fclose(fout);
    }else{
        ESP_LOGE("FILEIO", "data is NULL");
    }

}

// returns list of file names without extension
void getFilesInDir(list_t *list, const char *directory, const char *ext){
    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    //ESP_LOGI("FILEIO", "In getFileInDir");
    if (d != NULL)
    {
        //ESP_LOGI("FILEIO", "Dir openend");
        while((dir = readdir(d)) != NULL){
            if(dir->d_type != DT_REG) break;
            //ESP_LOGI("FILEIO", "Got file %s", dir->d_name);
            int len = strlen(dir->d_name);
            if(strncmp(dir->d_name - 4 + len, ext, 4) == 0){
                void *s = calloc(len - 3, 1);
                memcpy(s, (void*) dir->d_name, len - 4);
                list_add_element(list, s);
            }
        }
        closedir(d);
    }
}

int deleteFile(const char* file){
    struct stat st = {0};
    // check if bank file exists, 
    if (stat(file, &st) == 0) {
        //delete file
        //ESP_LOGI("FILEIO", "Deleting file %s", file);
        if(remove(file) == 0) return 1;
    }
    return 0;  
}



