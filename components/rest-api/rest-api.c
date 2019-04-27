#include "rest-api.h"
#include "index.html.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include <esp_http_server.h>
#include "ui_events.h"
#include "string_tools.h"
#include "fileio.h"

static const char *TAG="REST-API";
static httpd_handle_t server = NULL;
static xQueueHandle ui_ev_queue = NULL;

// increasing max header size in make menuconfig to 2048

static esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err){
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Error 404 not found!");
    return ESP_FAIL;
}

static esp_err_t drop_sample_put_handler(httpd_req_t *req){
    int ret, remaining = req->content_len, total = 0;
    char*  buf;
    size_t buf_len;
    FIL raw_file;
    FRESULT fr;
    UINT bw;
    ui_ev_ts_t ev;
    int file_len_d100 = req->content_len / 100;
    char file_name[32] = "";
    char file_name_jsn[32] = "";
    cJSON *val;
    cJSON *root = cJSON_CreateObject();

    ESP_LOGI(TAG, "Incoming put request, data length %d", remaining);

    buf_len = httpd_req_get_hdr_value_len(req, "Name") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Name", buf, buf_len) == ESP_OK) {
            //ESP_LOGI(TAG, "Found header => Name: %s", buf);
            cleanStringSpace(buf); // clean special characters
            sprintf(file_name, "%s.raw", buf);
            val = cJSON_CreateString (file_name);
            cJSON_AddItemToObject(root, "name", val);
            sprintf(file_name, "%s", buf);
            val = cJSON_CreateString (file_name);
            cJSON_AddItemToObject(root, "id", val);
            sprintf(file_name, "/usr/%s.RAW", buf); // usr folder (not in jsn object)
            sprintf(file_name_jsn, "/sdcard/usr/%s.JSN", buf);
        }
        free(buf);
        //ESP_LOGI(TAG, "User file name to be written are %s and %s", file_name, file_name_jsn);
    }else{
        ESP_LOGE(TAG, "Error: Could not derive file name from http header!");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, 0);
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Description") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Description", buf, buf_len) == ESP_OK) {
            cleanString(buf);
            //ESP_LOGI(TAG, "Found header => Description: %s", buf);
            val = cJSON_CreateString (buf);
            cJSON_AddItemToObject(root, "description", val);
        }
        free(buf);
    }else{
        val = cJSON_CreateString ("");
        cJSON_AddItemToObject(root, "description", val);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Tags") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Tags", buf, buf_len) == ESP_OK) {
            cleanString(buf);
            //ESP_LOGI(TAG, "Found header => Tags: %s", buf);
            val = cJSON_CreateString(buf);
            cJSON_AddItemToObject(root, "tags_s", val);
        }
        free(buf);
    }else{
        val = cJSON_CreateString("");
        cJSON_AddItemToObject(root, "tags_s", val);
    }

    f_open(&raw_file, file_name, FA_CREATE_ALWAYS | FA_WRITE);

    buf = (char*) malloc(4096);

    remaining %= 4;
    if(remaining != 0){
        remaining = 4 - remaining;
        ESP_LOGW(TAG, "File size not on mod 4 boundary, inserting %d zeros!", remaining);
        const char zeros[3] = {0};
        f_write(&raw_file, zeros, remaining, &bw);
    }
    remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, 4096))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                ESP_LOGE(TAG, "Socket Timeout!");
                continue;
            }
            ESP_LOGE(TAG, "Reveiving data file error!");
            f_close(&raw_file);
            f_unlink(file_name);
            free(buf);
            cJSON_Delete(root);
            return ESP_FAIL;
        }
        total += ret;
        remaining -= ret;

        if(ret > 0) f_write(&raw_file, buf, ret, &bw);

        /* Log data received */
        //ESP_LOGI(TAG, "Received %d bytes of data", ret);
        ev.event = EV_DECODING_PROGRESS;
        ev.event_data = (void*)(total / file_len_d100);
        xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
    }

    //ESP_LOGI(TAG, "Received total of %d bytes of data", total);
    //ESP_LOGI(TAG, "JSN %s", cJSON_Print(root));
    // these fields may be populated with user values in future
    val = cJSON_CreateString("myself");
    cJSON_AddItemToObject(root, "username", val);
    val = cJSON_CreateString("local");
    cJSON_AddItemToObject(root, "url", val);
    val = cJSON_CreateString("own license");
    cJSON_AddItemToObject(root, "license", val);
    writeJSONFile(file_name_jsn, cJSON_Print(root));


    
    //ESP_LOGE(TAG, "File size received %d", (int)f_size(&raw_file));
    f_close(&raw_file);
    free(buf);
    cJSON_Delete(root);

    // End response
    httpd_resp_send(req, NULL, 0);

    ev.event = EV_DECODING_DONE;
    xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
    return ESP_OK;
}

httpd_uri_t drop_sample = {
    .uri       = "/drop_sample",
    .method    = HTTP_PUT,
    .handler   = drop_sample_put_handler,
    .user_ctx  = NULL
};

static esp_err_t landing_handler_off(httpd_req_t *req){
    /* send landing page*/
    const char msg[] = "Set your Straempler to user receive mode!";
    const unsigned int msg_len= sizeof(msg); 
    httpd_resp_send(req, msg, msg_len);
    return ESP_OK;
}

static esp_err_t landing_handler_on(httpd_req_t *req){
    /* send landing page*/
    httpd_resp_send(req, index_html, index_html_len);
    return ESP_OK;
}

httpd_uri_t landing_off = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = landing_handler_off,
    .user_ctx  = NULL
};

httpd_uri_t landing_on = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = landing_handler_on,
    .user_ctx  = NULL
};

static httpd_handle_t start_webserver(void){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 4096*2;
    config.core_id = 0;
    config.task_priority = 5;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d', prio %d", config.server_port, config.task_priority);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &landing_off);
        //httpd_register_uri_handler(server, &drop_sample);
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void startRestAPI(xQueueHandle queueui){
    ui_ev_queue = queueui;
    start_webserver();
}

void setRestAPIUserReceiveOn(){
    httpd_unregister_uri(server, "/");
    httpd_register_uri_handler(server, &drop_sample);
    httpd_register_uri_handler(server, &landing_on);
}

void setRestAPIUserReceiveOff(){
    httpd_unregister_uri(server, "/");
    httpd_unregister_uri(server, "/drop_sample");
    httpd_register_uri_handler(server, &landing_off);
}

void stopRestAPI(){
    httpd_stop(server);
}