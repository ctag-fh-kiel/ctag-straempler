#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "wifi.h"
#include "cJSON.h"
#include "mp3.h"
#include "fileio.h"
#include "ui_events.h"
#include "list.h"
#include "esp_vfs_fat.h"

#include "esp_request.h"

static xQueueHandle ui_ev_queue = NULL;
static char freesound_token[48];

FIL _mp3_file;

static int print_list_elements(list_item_t* it){
    printf("%s\n", (char*)it->value);
    return 0;
}

static int if_element(list_item_t* it, void* ptr){
    int res = strcmp((char*)it->value, (char*)ptr);
    if(res==0) return 1;
    return 0;
}

static int tag_callback(request_t *req, char *data, int len)
{
    req_list_t *found = req->response->header;
    static int rcv = 0, sz = 0;
    static list_t *tag_list = NULL;
    static int progress = 0;
    ui_ev_ts_t ev;
    
    while(found->next != NULL) {
        found = found->next;
        if(!strcmp((char*)found->key, "Content-Length")){
            //ESP_LOGI("rcv","Response header %s:%s", (char*)found->key, (char*)found->value);
            sz = atoi((char*)found->value);
            if(rcv == 0){
                progress = 0;
                rcv = sz;
                if(tag_list != NULL) list_free(tag_list);
                tag_list = list_create();
            }
            break;
        }
    }
    if(rcv > 0){
        rcv -= len;
        char *p1 = data;
        do{
            p1 = strstr(p1, "<a href=\"/browse/tags/");
            if(p1!=NULL){
                p1 += 22;
                char *p2 = strchr(p1, '/');
                if(p2!=NULL){
                    int len = p2 - p1;
                    char *buf = calloc(len + 1, 1);
                    memcpy(buf, p1, len);
                    //printf("%s\n", buf);
                    //free(buf);
                    list_add_element_if(tag_list, if_element, (void*)buf);
                    p1 = p2;
                }
            }
        }while(p1!=NULL);
        progress = (sz - rcv) * 100 / sz;
        ev.event = EV_PROGRESS_UPDATE;
        ev.event_data = (void*)progress;
        xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
    } 
    //ESP_LOGI("rcv", "rcv %d", rcv);
    
    if(rcv == 0){
        ev.event = EV_FREESND_TAGLIST;
        ev.event_data = (void*)tag_list;
        xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
    } 
    
    return 0;
}

static void tag_request(void *pvParameters)
{
    const char *path = (const char*) pvParameters;
    char url[1024];
    sprintf(url, "https://freesound.org/browse/tags/%s", path);
    request_t *req;
    wifiWaitForConnected();
    req = req_new(url); 
    req_setopt(req, REQ_SET_METHOD, "GET");
    req_setopt(req, REQ_FUNC_DOWNLOAD_CB, tag_callback);
    
    req_perform(req);
    
    req_clean(req);
    
    vTaskDelete(NULL);
}

static int mp3_callback(request_t *req, char *data, int len)
{
    req_list_t *found = req->response->header;
    static int rcv = 0, progress = 0, sz = 0, oldProgress = 0;
    ui_ev_ts_t ev;

    while(found->next != NULL) {
        found = found->next;
        if(!strcmp((char*)found->key, "Content-Length")){
            //ESP_LOGI("MP3","Response header %s:%s", (char*)found->key, (char*)found->value);
            sz = atoi((char*)found->value);
            if(rcv == 0){
                ESP_LOGI("MP3","File size %d", sz);
                rcv = sz;
                progress = 0;
                //initMP3Engine();
            }
            break;
        }
    }
     
    if(rcv > 0){
        //ESP_LOGI("MP3","Remaining bytes %d", rcv);
        rcv -= len;
        //decodeMP3((unsigned char*)data, len, _mp3_file);
        progress = (sz - rcv) * 100 / sz;
        ev.event = EV_PROGRESS_UPDATE;
        ev.event_data = (void*)progress;
        if(oldProgress != progress)
            xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
        oldProgress = progress;
        UINT bw;
        f_write(&_mp3_file, data, len, &bw);
    } 

    if(rcv == 0){
        f_close(&_mp3_file);
        ESP_LOGI("MP3","Completed");
        //freeMP3Engine();
        ev.event = EV_FREESND_MP3_COMPLETE;
        ev.event_data = NULL;
        xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);    
    }
        
    return 0;
}


int instance_callback(request_t *req, char *data, int len)
{
    req_list_t *found = req->response->header;
    static int rcv = 0;
    static char *buf = NULL, *pbuf = NULL;
    ui_ev_ts_t ev;

    while(found->next != NULL) {
        found = found->next;
        //  ESP_LOGW("HEADER", "%s:%s", (char*)found->key, (char*)found->value);
        if(!strcmp((char*)found->key, "Content-Length")){
            //ESP_LOGI("FREESOUND","Response header %s:%s", (char*)found->key, (char*)found->value);
            int sz = atoi((char*)found->value);
            if(rcv == 0){
                rcv = sz;
                buf = (char*)malloc(sz+1);
                pbuf = buf;
            }
            break;
        }
    }
  
    if(rcv > 0){
        rcv -= len;
        memcpy(pbuf, data, len);
        pbuf += len;
    } 
    //ESP_LOGI("FREESOUND","rcv %d", rcv);
    
    if(rcv == 0){
        (*pbuf) = '\0';
        //ESP_LOGW("FREESOUND","%s", buf);
        char fnamebuf[64];
        cJSON *root = cJSON_Parse(buf);
        // test is file exists at all at freesound, if not leave
        if(cJSON_GetObjectItem(root,"detail") != NULL){
            if(!strcmp(cJSON_GetObjectItem(root,"detail")->valuestring, "Not found.")){
                cJSON_Delete(root);
                free(buf);
                ev.event = EV_FREESND_NOT_FOUND;
                ev.event_data = NULL;
                xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);    
                return 0;
            };
        }
        // otherwise acquire preview for decoding
        cJSON *previews = cJSON_GetObjectItem(root,"previews");
        int id = cJSON_GetObjectItem(root, "id")->valueint;
        char* mp3_url = cJSON_GetObjectItem(previews,"preview-hq-mp3")->valuestring;
        snprintf(fnamebuf, 64, "/pool/%d.mp3", id);
        ESP_LOGI("ID","mp3 URL is: %s, fname: %s", mp3_url, fnamebuf);
        FRESULT fr;
        fr = f_open(&_mp3_file, fnamebuf, FA_CREATE_ALWAYS | FA_WRITE);
        snprintf(fnamebuf, 64, "/sdcard/pool/%d.jsn", id);
        writeJSONFile(fnamebuf, buf);
        if(fr == 0){
            request_t *req;
            req = req_new(mp3_url); 
            req_setopt(req, REQ_SET_METHOD, "GET");
            req_setopt(req, REQ_FUNC_DOWNLOAD_CB, mp3_callback);
            int status = req_perform(req);
            req_clean(req);
        }else{
            ESP_LOGE("ID","Could not write file %s", fnamebuf);
        }

        cJSON_Delete(root);
        free(buf);
        
    } 
    return 0;
}

int search_callback(request_t *req, char *data, int len)
{
    req_list_t *found = req->response->header;
    static int rcv = 0;
    static char *buf = NULL, *pbuf = NULL;
    while(found->next != NULL) {
        found = found->next;
        if(!strcmp((char*)found->key, "Content-Length")){
            //ESP_LOGI("FREESOUND","Response header %s:%s", (char*)found->key, (char*)found->value);
            int sz = atoi((char*)found->value);
            if(rcv == 0){
                rcv = sz;
                buf = (char*)malloc(sz+1);
                pbuf = buf;
            }
            break;
        }
    }
    
    
    if(rcv > 0){
        rcv -= len;
        memcpy(pbuf, data, len);
        pbuf += len;
    } 
    //ESP_LOGI("FREESOUND","rcv %d", rcv);
    
    if(rcv == 0){
        (*pbuf) = '\0';
        ESP_LOGW("FREESOUND","%s", buf);
        /*
        cJSON *root = cJSON_Parse(buf);
        cJSON *previews = cJSON_GetObjectItem(root,"previews");
        char* mp3_url = cJSON_GetObjectItem(previews,"preview-hq-mp3")->valuestring;
        ESP_LOGI("FREESOUND","mp3 URL is: %s", mp3_url);
        request_t *req;
        req = req_new(mp3_url); 
        req_setopt(req, REQ_SET_METHOD, "GET");
        req_setopt(req, REQ_FUNC_DOWNLOAD_CB, mp3_callback);
        int status = req_perform(req);
        req_clean(req);
        cJSON_Delete(root);
        free(buf);
        */
    } 
    return 0;
}


static void search_request(void *pvParameters)
{
    const char *query = (const char*) pvParameters;
    char url[1024];
    ESP_LOGW("search", "%s", query);
    sprintf(url, "https://freesound.org/apiv2/search/text/?token=%s&filter=tag:%s", freesound_token, query);
    request_t *req;
    wifiWaitForConnected();
    req = req_new(url); 
    req_setopt(req, REQ_SET_METHOD, "GET");
    req_setopt(req, REQ_FUNC_DOWNLOAD_CB, search_callback);
    
    req_perform(req);
    
    req_clean(req);
    
    vTaskDelete(NULL);
}

static void instance_request(void *pvParameters)
{
    const char *id = (const char*) pvParameters;
    char url[1024];
    ui_ev_ts_t ev;
    cJSON *root = NULL;
    ESP_LOGW("id", "%s", id);
    sprintf(url, "https://freesound.org/apiv2/sounds/%s/?token=%s", id, freesound_token);
    ESP_LOGI("fsnd", "%s", url);
    wifiWaitForConnected();

    esp_http_client_config_t config = {
            .url = url,
            .method = HTTP_METHOD_GET
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = 0;

    if ((err = esp_http_client_open(client, 0)) != ESP_OK) {
        ESP_LOGE("FREESOUND", "Failed to open HTTP connection: %s", esp_err_to_name(err));
        ev.event = EV_FREESND_NOT_FOUND;
        ev.event_data = NULL;
        xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
        esp_http_client_cleanup(client);
        vTaskDelete(NULL);
        return;
    }

    int content_length =  esp_http_client_fetch_headers(client);
    int total_read_len = 0, read_len;
    // one buffer at least 512 bytes large
    char *buffer = heap_caps_malloc(content_length + 1 < 512 ? 512 : content_length + 1, MALLOC_CAP_SPIRAM);
    assert(buffer!=NULL);
    while (total_read_len < content_length) {
        read_len = esp_http_client_read(client, &buffer[total_read_len], content_length - total_read_len > 512 ? 512 : content_length - total_read_len);
        if (read_len <= 0) {
            ev.event = EV_FREESND_NOT_FOUND;
            ev.event_data = NULL;
            xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
            heap_caps_free(buffer);
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            vTaskDelete(NULL);
            return;
        }
        total_read_len += read_len;
    }
    buffer[content_length] = 0;
    ESP_LOGI("FSND", "mp3 file info %s", buffer);
    root = cJSON_Parse(buffer);
    char fnamebuf[64];

    // test if file exists at all at freesound, if not leave
    if(cJSON_GetObjectItem(root,"detail") != NULL){
        if(!strcmp(cJSON_GetObjectItem(root,"detail")->valuestring, "Not found.")){
            cJSON_Delete(root);
            ev.event = EV_FREESND_NOT_FOUND;
            ev.event_data = NULL;
            xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
            heap_caps_free(buffer);
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            vTaskDelete(NULL);
            return;
        };
    }

    // otherwise acquire preview for decoding
    cJSON *previews = cJSON_GetObjectItem(root,"previews");
    int i_id = cJSON_GetObjectItem(root, "id")->valueint;
    int progress = 0, oldProgress = 0;
    char* mp3_url = cJSON_GetObjectItem(previews,"preview-hq-mp3")->valuestring;
    snprintf(fnamebuf, 64, "/pool/%d.mp3", i_id);
    ESP_LOGI("ID","mp3 URL is: %s, fname: %s", mp3_url, fnamebuf);
    FRESULT fr;
    fr = f_open(&_mp3_file, fnamebuf, FA_CREATE_ALWAYS | FA_WRITE);
    snprintf(fnamebuf, 64, "/sdcard/pool/%d.jsn", i_id);
    writeJSONFile(fnamebuf, buffer);
    // file open ok?
    assert(fr == 0);

    // set preview url
    esp_http_client_set_url(client, mp3_url);
    if ((err = esp_http_client_open(client, 0)) != ESP_OK) {
        ESP_LOGE("FREESOUND", "Failed to open HTTP connection: %s", esp_err_to_name(err));
        ev.event = EV_FREESND_NOT_FOUND;
        ev.event_data = NULL;
        xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
        heap_caps_free(buffer);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        vTaskDelete(NULL);
        return;
    }
    // get preview data
    content_length = esp_http_client_fetch_headers(client);
    total_read_len = 0;
    while (total_read_len < content_length) {
        read_len = esp_http_client_read(client, buffer, content_length - total_read_len > 512 ? 512 : content_length - total_read_len);
        if (read_len <= 0) {
            ESP_LOGE("FREESOUND", "Error reading freesound data!");
            ev.event = EV_FREESND_NOT_FOUND;
            ev.event_data = NULL;
            xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
            heap_caps_free(buffer);
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            vTaskDelete(NULL);
            return;
        }
        progress = 100 - (content_length - total_read_len) * 100 / content_length;
        ev.event = EV_PROGRESS_UPDATE;
        ev.event_data = (void*)progress;
        if(oldProgress != progress)
            xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
        oldProgress = progress;
        UINT bw;
        f_write(&_mp3_file, buffer, read_len, &bw);
        total_read_len += read_len;
    }

    // close file and finish up
    f_close(&_mp3_file);
    ESP_LOGI("MP3","Completed");
    ev.event = EV_FREESND_MP3_COMPLETE;
    ev.event_data = NULL;
    xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);

    heap_caps_free(buffer);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}

static void init_token(){
    cJSON* root = NULL;
    root = readJSONFileAsCJSON("/sdcard/CONFIG.JSN");
    //printf("%s\n", cJSON_Print(root));
    if(root != NULL){
        cJSON *settings = cJSON_GetObjectItemCaseSensitive(root, "settings");
        if(settings != NULL){
            cJSON *val;
            val = cJSON_GetObjectItemCaseSensitive(settings, "apikey");
            memset(freesound_token, 0, sizeof(freesound_token) / sizeof(char));
            strcpy(freesound_token, val->valuestring);
            return;
        }else ESP_LOGE("FREESOUND", "settings == NULL");
    }else ESP_LOGE("FREESOUND", "root == NULL");
    cJSON_Delete(root);
}

void freesoundGetTags(const char *path){
    void *params = (void*) path;
    xTaskCreatePinnedToCore(&tag_request, "tag_request", 8192, params, 5, NULL, 0);
}

void freesoundSearch(const char *query){
    void *params = (void*) query;
    xTaskCreatePinnedToCore(&search_request, "search_request", 8192, params, 5, NULL, 0);
}


void freesoundGetInstance(const char *id){
    void *params = (void*) id;
    
    xTaskCreatePinnedToCore(&instance_request, "instance_request", 8192, params, 5, NULL, 0);
}


void freesoundInit(xQueueHandle queueui){
    ui_ev_queue = queueui;
    init_token();
}

void freesoundSetToken(const char *token){
    memset(freesound_token, 0, sizeof(freesound_token) / sizeof(char));
    strcpy(freesound_token, token);
}