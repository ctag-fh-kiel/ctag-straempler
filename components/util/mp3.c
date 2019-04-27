#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#define MIPS
#include "mp3dec.h"
#include "cJSON.h"
#include "fileio.h"
#include "esp_log.h"
#include "ui_events.h"
#include "mp3.h"
#include "esp_vfs_fat.h"

#define MAX_FRAME_SIZE 4096

static xQueueHandle ui_ev_queue = NULL;

typedef struct{
    char fin[64];
    char fout[64];
    int nChannels;
    int mp3FileSize;
}task_param_t;

unsigned char *input;
unsigned char *readPtr;
    
void initMP3Engine(xQueueHandle queueui){
    ui_ev_queue = queueui;
}

static void decode(FIL *mp3File, FIL* rawOut, int sz){
    ui_ev_ts_t ev;
    int toRead = sz, progress = 0;
    ev.event = EV_DECODING_PROGRESS;
    HMP3Decoder decoder = MP3InitDecoder();

    if(decoder == NULL){
        ESP_LOGE("MP3", "No decoder allocated");
        return;
    }

    ESP_LOGI("MP3", "Decoder instantiated");

    UINT nRead = 0; 
    int offset, foundStartOfFrame = 0, oldProgress = 0;
    do
    {
        // Read the input file
        f_read(mp3File, input, MAX_FRAME_SIZE, &nRead);
        toRead -= nRead;
        progress = (sz - toRead) * 100;
        progress /= sz;
        ev.event_data = (void*)progress;
        xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);

        if(nRead == 0)
        {
        // We have reached end of file and a valid MP3 start of frame was not found.
        // Do something.
        break;
        }
        else
        {
        offset = MP3FindSyncWord(input,MAX_FRAME_SIZE);
        //ESP_LOGI("MP3", "Offset: %d", offset);
        if(offset < 0)
        {
        // The input buffer does not contain a start of frame. Read another frame.
        continue;
        }
        else
        {
        // We found a start of frame. offset contains location of the start of frame
        // within input buffer.
        foundStartOfFrame = 1;
        break;
        }
        }
    }while(nRead!=0);

    MP3FrameInfo frameInfo;
    if(foundStartOfFrame == 1)
    {
        int error;
        error = MP3GetNextFrameInfo(decoder, &frameInfo, input);
        ESP_LOGI("MP3", "MP3 Error %d", error);
        /*if(error == MP3_INVALID_FRAME_HEADER)
        {
        // This means that the MP3FindSyncWord function has found the sync word,
        // but this was not a start of frame. This may have happened because
        // the sync word may have found in an ID3 tag.
        //GetAnotherFrame();
        }
        else if(frameInfo.sampRate != 44100)
        {
        // For this example, we want only data which
        // was sampled at 44100 Hz. Ignore this frame.
        //IgnoreThisFrame();
        }
        */
    }

    ESP_LOGI("MP3", "FrameInfo: %d", frameInfo.samprate);
    
    // Decode a MP3 frame. It is assumed that the MP3FindSyncWord() function was used
    // to find a start of frame.
    short output[2 * 1152]; // stereo
    int err;
    int bytesLeft = MAX_FRAME_SIZE;
    readPtr = input;
    do{
        err = MP3Decode(decoder, &readPtr, &bytesLeft, output, 0);
        //ESP_LOGI("MP3", "Error %d", err);
        //ESP_LOGI("MP3", "bytes remaining %d", bytesLeft);
        // bytesLeft will have number of bytes left in the input buffer. Input buffer will
        // point to the first unconsumed byte.
        // This code example shows how the errors can be handled.
        // This may differ between applications.
        memmove(input, readPtr, bytesLeft);
        f_read(mp3File, input + bytesLeft, MAX_FRAME_SIZE - bytesLeft, &nRead);
        toRead -= nRead;
        progress = (sz - toRead) * 100;
        progress /= sz;
        ev.event_data = (void*)progress;
        if(progress != oldProgress){
            xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
        }
        oldProgress = progress;;
        //ESP_LOGI("MP3", "Read %d, size %d, to read %d, progress %d", nRead, sz, toRead, progress);
        readPtr = input;
        bytesLeft += nRead;

        UINT nWrite;
        if(err == ERR_MP3_NONE ){
            // The MP3GetLastFrameInfo() function can be used to obtain information about the
            // frame. The following shows an example.
            // Get information about the last decoded frame. It is assumed that the frame was
            // decoded by calling the decode function.
            MP3FrameInfo mp3frameInfo;
            MP3GetLastFrameInfo(decoder, &mp3frameInfo);
            // Get the size of the output raw audio frame.
            //ESP_LOGI("MP3", "Decoded samples %d", mp3frameInfo.outputSamps );
            if(mp3frameInfo.outputSamps != 0)
            {
                f_write(rawOut, output, mp3frameInfo.outputSamps*2, &nWrite);
                //ESP_LOGI("MP3", "Number of output samples decoded %d", mp3frameInfo.outputSamps)
            }
        }else{
            ESP_LOGI("MP3", "Error %d", err);
        }

    }
    while(!err && bytesLeft > 0);


    MP3FreeDecoder(decoder);
}

static void decoder_task(void* pvParams){
    ui_ev_ts_t ev;
    int progress = 0;
    FIL fin, fout;
    FRESULT fr;
    task_param_t *params = (task_param_t*) pvParams;
    fr = f_open(&fin, params->fin, FA_READ);
    ESP_LOGI("MP3", "fin %s, fout %s", params->fin, params->fout);
    if(fr){
        ESP_LOGE("MP3", "Could not open infile %s", params->fin);
        free(pvParams);
        vTaskDelete(NULL);
        return;
    }
    uint32_t mp3FileSize = f_size(&fin);
    ESP_LOGI("MP3", "Size of mp3 file to be decoded %d", mp3FileSize);
    fr = f_open(&fout, params->fout, FA_CREATE_ALWAYS | FA_WRITE);
    if(fr){
        ESP_LOGE("MP3", "Could not open outfile %s", params->fout);
        f_close(&fin);
        free(pvParams);
        vTaskDelete(NULL);
        return;
    }
    
    input = (unsigned char*) malloc(MAX_FRAME_SIZE);
    ev.event = EV_DECODING_PROGRESS;
    ev.event_data = (void*)progress;
    xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
    
    decode(&fin, &fout, mp3FileSize);
    free(input);

    f_close(&fin);
    f_close(&fout);

    /*
    for(;;){
        //ESP_LOGI("", "Tick");
        ev.event = EV_DECODING_PROGRESS;
        ev.event_data = (void*)progress;
        xQueueSend(ui_ev_queue, &ev, 0);
        progress++;
        if(progress == 100) break;
        vTaskDelay(300 / portTICK_RATE_MS);
    }
    */
    ev.event = EV_DECODING_DONE;
    xQueueSend(ui_ev_queue, &ev, portMAX_DELAY);
    free(pvParams);
    vTaskDelete(NULL);
}

void decodeMP3File(const char *id){
    task_param_t *taskParams = (task_param_t*)calloc(1, sizeof(task_param_t));
    char s[64]; cJSON *fPars;
    snprintf(s, 64, "/pool/%s.MP3", id);
    strcpy(taskParams->fin, s);
    snprintf(s, 64, "/raw/%s.RAW", id);
    strcpy(taskParams->fout, s);
    snprintf(s, 64, "/sdcard/pool/%s.JSN", id);
    fPars = readJSONFileAsCJSON(s);
    taskParams->nChannels = cJSON_GetObjectItem(fPars, "channels")->valueint;
    ESP_LOGI("MP3", "Infile %s, outfile %s", taskParams->fin, taskParams->fout);
    ESP_LOGI("MP3", "nChannels %d", taskParams->nChannels);

    xTaskCreatePinnedToCore(decoder_task, "decoder_task", 8192*2, (void*)taskParams, 10, NULL, 0);
}

/*
void decodeMP3(unsigned char* inputData, unsigned int len, FILE* outfile){
    
    static int bytesLeft = 0;
    static int ent = 0, left = 0;
    // first fill up frame buffer
    int cpySz, inpRemain;
    cpySz = len > MP3_FR_SZ - bytesLeft ? MP3_FR_SZ - bytesLeft : len;
    inpRemain = len - cpySz;
    memcpy((void*)&_mp3_frame_buf[bytesLeft], inputData, cpySz);
    bytesLeft += cpySz;
    ESP_LOGW("MP3", "Entered %d inpRemain %d bytesLeft %d", ent++, inpRemain, bytesLeft);
    if(bytesLeft < MP3_FR_SZ) return; // i.e. frame buffer not yet full
    
    // if frame buffer is filled, start decode
    if(_isInitialized == 0){
        // find sync
        int offset;
        do{
            offset = MP3FindSyncWord(_mp3_frame_buf, MP3_FR_SZ);
            ESP_LOGI("MP3", "Offset: %d", offset);
            if(offset < 0){
                ESP_LOGW("MP3", "No frame sync found in MP3 stream!");
                bytesLeft = 0; // discard entire frame buffer
                // still something to read from input?
                if(inpRemain > 0){
                    cpySz = inpRemain > MP3_FR_SZ ? MP3_FR_SZ : inpRemain;
                    memcpy(_mp3_frame_buf, (void*)&inputData[inpRemain], cpySz);
                    inpRemain -= cpySz;
                    bytesLeft += cpySz;
                }
                if(bytesLeft < MP3_FR_SZ) return; // i.e. buffer not full
            }
        }while(offset < 0);
    }


    // get info of first frame
    MP3FrameInfo frameInfo;
    int err;
    if(_isInitialized == 0){
        _isInitialized = 1;
        err = MP3GetNextFrameInfo(_decoder, &frameInfo, _mp3_frame_buf);
        ESP_LOGI("MP3", "Frame Info Err %d", err);
        ESP_LOGI("MP3", "Sample Rate: %d", frameInfo.samprate);
    }

 
    // decode frame 16 bit stereo
    short output[2 * 1152]; // stereo
    unsigned char* readPtr;
    int bytesLeftAfterDecode;
    do{
        readPtr = _mp3_frame_buf;
        ESP_LOGI("MP3", "Before bytes in buf %d input %d", bytesLeft, inpRemain);
        err = MP3Decode(_decoder, &readPtr, &bytesLeftAfterDecode, output, 0);
        ESP_LOGI("MP3", "Decode Err %d", err);
        ESP_LOGI("MP3", "After bytes in buf %d input %d", bytesLeftAfterDecode, inpRemain);
        //if(!err){
            bytesLeft = bytesLeftAfterDecode;
            memmove(_mp3_frame_buf, readPtr, bytesLeft);
        //}
        // fill up more in framebuffer if more input available
        if(inpRemain > 0){
            cpySz = inpRemain > MP3_FR_SZ - bytesLeft ? MP3_FR_SZ - bytesLeft : inpRemain;
            memcpy((void*)&_mp3_frame_buf[bytesLeft], (void*)&inputData[inpRemain], cpySz);
            inpRemain -= cpySz;
            bytesLeft += cpySz;
        }
        if(err == ERR_MP3_NONE){
            int nOutputSamples;
            MP3GetLastFrameInfo(_decoder, &frameInfo);
            nOutputSamples = frameInfo.outputSamps;
            ESP_LOGI("MP3", "Decoded amount of samples %d", nOutputSamples);
        }
    }while(!err && bytesLeft == MP3_FR_SZ);
    ESP_LOGW("MP3", "Left %d inpRemain %d bytesLeft %d", left++, inpRemain, bytesLeft);
}
*/
