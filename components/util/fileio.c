#include "fileio.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
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
