#include "fill_buffer.h"


void fill_audio_buffer(voice_play_mode_t* playback_engine, audio_b_t* buffer, audio_f_t* file, SemaphoreHandle_t* mutex){
    // ESP_LOGE("Audio Buffer", "start sample: %ud, loop start: %ud, loop end: %d, rpos %ud", playback_engine->sample_start, playback_engine->loop_start, playback_engine->loop_end, file->fpos);
    if(mutex != NULL)
    {
        xSemaphoreTake(*mutex, portMAX_DELAY);
        playback_engine->play_mode(playback_engine, buffer, file);
        xSemaphoreGive(*mutex);
    }
    else
    {
        playback_engine->play_mode(playback_engine, buffer, file);
    }

    
}

void fill_buffer_one_shot(void* _playback_engine, void* _buffer, void* _file){
    UINT n = 0;
    voice_play_mode_t* playback_engine = (voice_play_mode_t*) _playback_engine;
    bool play_dir = playback_engine->is_playback_direction_forward;
    int samps_to_read = SD_BUF_SZ;
    audio_b_t* buffer = (audio_b_t*) _buffer;
    audio_f_t* file = (audio_f_t*) _file;
    
    if(play_dir){ // Playback is forward
        if(file->fpos >= playback_engine->loop_end){ // if file reached the end just load zeros to buffer
            memset(buffer->buf, 0, samps_to_read);
            n = samps_to_read;
        }
        else
        {
            //f_lseek(&(file->fil), file->fpos);
            f_read(&(file->fil), buffer->buf, samps_to_read, &n);
            file->fpos += n;
        }
    }
    else // Playback is backwards
    {
        uint32_t start_pos = file->fpos - SD_BUF_SZ;
        start_pos = ((int)start_pos >= 0) ? start_pos : 0; // make sure to not pass a negative value to fread
        if(file->fpos <= playback_engine->sample_start){ // if file reached the beginning just load zeros to buffer
            memset(buffer->buf, 0, samps_to_read);
            n = samps_to_read;
        }
        else{
            f_lseek(&(file->fil), start_pos);
            f_read(&(file->fil), buffer->buf, samps_to_read, &n);
            reverseBuffer32((n / 4), buffer->buf); // reverse buffer in place
            file->fpos -= n;
        }
    }

    //ESP_LOGE("TAG", "fpos: %ud, fsize: %d", file->fpos, file->fsize);

    buffer->rpos = 0;
    buffer->len = n; // amount of bytes read from buffer
}

void jump_to_start(voice_play_mode_t* playback_engine, audio_b_t* first_buffer,audio_b_t* second_buffer, audio_f_t* file, SemaphoreHandle_t* mutex){
    bool play_dir = playback_engine->is_playback_direction_forward;
    uint32_t fileSize = file->fsize, pointToSeek = 0;
    
    if(play_dir)
    {
        uint32_t sample_start = playback_engine->sample_start;
        pointToSeek = (fileSize - sample_start) < SD_BUF_SZ ? (fileSize - sample_start) : SD_BUF_SZ;
        first_buffer->len = pointToSeek;
        first_buffer->rpos = 0;
        xSemaphoreTake(*mutex, portMAX_DELAY);
        f_lseek(&(file->fil), (pointToSeek + sample_start)); // seek to the position in file from which the sample needs to be loaded to buffers
        file->fpos = sample_start + pointToSeek;
        xSemaphoreGive(*mutex); 
    }
    else 
    {
        uint32_t loop_end = playback_engine->loop_end;
        uint32_t pointToSeek = (loop_end < SD_BUF_SZ) ? loop_end : SD_BUF_SZ;
        first_buffer->len = pointToSeek;
        first_buffer->rpos = 0;
        pointToSeek = (loop_end  - pointToSeek);
        if(pointToSeek <= 0) pointToSeek = 0;
        xSemaphoreTake(*mutex, portMAX_DELAY);
        f_lseek(&(file->fil), pointToSeek); // seek to the position in file from which the sample needs to be loaded to buffers
        file->fpos = pointToSeek;
        xSemaphoreGive(*mutex);
    }
    xSemaphoreTake(*mutex, portMAX_DELAY);
    playback_engine->play_mode(playback_engine, second_buffer, file);
    xSemaphoreGive(*mutex);
}

void refill_first_buf(voice_play_mode_t* playback_engine, audio_b_t* buffer, audio_f_t* file, SemaphoreHandle_t* mutex){
    bool play_dir = playback_engine->is_playback_direction_forward;
    xSemaphoreTake(*mutex, portMAX_DELAY);
    uint32_t file_pos = (uint32_t)f_tell(&(file->fil));

    if(play_dir)
    {
        f_lseek(&(file->fil), playback_engine->sample_start); 
    }
    else
    {
        f_lseek(&(file->fil), playback_engine->loop_end); 
    }

    playback_engine->play_mode(playback_engine, buffer, file);

    f_lseek(&(file->fil), file_pos); // make sure to reset where file was before first buffer was reloaded
    xSemaphoreGive(*mutex);
}


void init_play_mode(voice_play_mode_t* pm){
    pm->is_pipo_playback_forward = 0;
    pm->is_playback_direction_forward = 1;
    pm->loop_end = 0;
    pm->loop_start = 0;
    pm->sample_start = 0;
    pm->play_mode = &fill_buffer_one_shot;
    pm->phase = 0;
    pm->pitch_increment = 1.0f;
    pm->last_cummulated_sample = 0;
    pm->mode = SINGLE;
}


/**
 * @brief refills in loop mode when global playback is forward
 * 
 * @param playback_engine holds the playback function
 * @param buffer audio buffer which gets filled
 * @param file audio file from where data should be read
 */

void fill_buffer_fwd_loop(voice_play_mode_t* playback_engine, audio_b_t* buffer, audio_f_t* file){
    int *loop_start = (int*)&(playback_engine->loop_start);
    int *loop_end = (int*)&(playback_engine->loop_end);
    UINT n = 0;

    if(file->fpos + SD_BUF_SZ <= *loop_end) // we have not reached the loop end playing forward
    {
        f_lseek(&(file->fil), file->fpos);
        f_read(&(file->fil), buffer->buf, SD_BUF_SZ, &n);
        file->fpos += n;
    }else if(file->fpos >= *loop_end){
        f_lseek(&(file->fil), *loop_start);
        file->fpos = *loop_start;
        f_read(&(file->fil), buffer->buf, SD_BUF_SZ, &n);
        file->fpos += n;
    }else
    {
        int samps_remain = *loop_end - file->fpos;
        f_lseek(&(file->fil), file->fpos);
        f_read(&(file->fil), buffer->buf, samps_remain, &n);
        file->fpos = *loop_start;
        f_lseek(&(file->fil), *loop_start);
        UINT tempn;
        f_read(&(file->fil), buffer->buf + samps_remain, (SD_BUF_SZ - samps_remain), &tempn);
        n += tempn;
        file->fpos += tempn;
    }

    buffer->rpos = 0;
    buffer->len = n; // amount of bytes read from buffer
}


/**
 * @brief refills in loop mode when global playback is backwards
 * 
 * @param playback_engine holds the playback function
 * @param buffer audio buffer which gets filled
 * @param file audio file from where data should be read
 */

void fill_buffer_bwd_loop(voice_play_mode_t* playback_engine, audio_b_t* buffer, audio_f_t* file){
    int loop_start = playback_engine->loop_start;
    int loop_end = playback_engine->loop_end;
    UINT n = 0;
    int file_pos = file->fpos;

    if(file_pos - SD_BUF_SZ >= loop_start){
        int read_pos = file_pos - SD_BUF_SZ;
        //ESP_LOGI("BUF", "read_pos %d, filesize %d", read_pos, (int)f_size(file->f));
        f_lseek(&(file->fil), read_pos);
        f_read(&(file->fil), buffer->buf, SD_BUF_SZ, &n);
        reverseBuffer32((n / 4), buffer->buf);
        file->fpos -= n;
        if(file->fpos <= 0){
            file->fpos = loop_end;
        }
    }else if(file_pos <= loop_start){
        int seek_pos = (loop_end - SD_BUF_SZ) < 0 ? 0 : (loop_end - SD_BUF_SZ);
        f_lseek(&(file->fil), seek_pos);
        f_read(&(file->fil), buffer->buf, SD_BUF_SZ, &n);
        reverseBuffer32((n / 4), buffer->buf);
        if(seek_pos == 0){
            f_lseek(&(file->fil), loop_end);
            file->fpos = loop_end;
        }else{
            f_lseek(&(file->fil), seek_pos);
            file->fpos = seek_pos;
        }
        
    }else{
        int samps_to_reverse = file->fpos - loop_start;
        int samps_remain = SD_BUF_SZ - samps_to_reverse;
        f_lseek(&(file->fil), loop_start);
        f_read(&(file->fil), buffer->buf, samps_to_reverse, &n);
        f_lseek(&(file->fil), (loop_end - samps_remain) > 0 ? loop_end - samps_remain : 0);
        UINT tempn;
        f_read(&(file->fil), buffer->buf + samps_to_reverse, samps_remain, &tempn);
        n += tempn;
        reverseBuffer32((n / 4), buffer->buf);
        f_lseek(&(file->fil), loop_end);
        file->fpos = loop_end;
            
    }

    buffer->rpos = 0;
    buffer->len = n; // amount of bytes read from buffer
}

void fill_buffer_loop(void* _playback_engine, void* _buffer, void* _file){
    voice_play_mode_t* playback_engine = (voice_play_mode_t*) _playback_engine;
    audio_b_t* buffer = (audio_b_t*) _buffer;
    audio_f_t* file = (audio_f_t*) _file;
    bool play_dir = playback_engine->is_playback_direction_forward;
    
    if(play_dir){
        fill_buffer_fwd_loop(playback_engine, buffer, file);
    }
    else{
        fill_buffer_bwd_loop(playback_engine, buffer, file);
    }
}


/**
 * @brief refills in pipo mode when global playback is forward
 * 
 * @param playback_engine holds the playback function
 * @param buffer audio buffer which gets filled
 * @param file audio file from where data should be read
 */

void fill_buffer_fwd_pipo(voice_play_mode_t* playback_engine, audio_b_t* buffer, audio_f_t* file){
    int loop_start = playback_engine->loop_start;
    int loop_end = playback_engine->loop_end;
    UINT n = 0;

    if(file->fpos + SD_BUF_SZ <= loop_end) // we have not reached the loop end playing forward
    {
        f_lseek(&(file->fil), file->fpos);
        f_read(&(file->fil), buffer->buf, SD_BUF_SZ, &n);
        file->fpos += n;
    }else if(file->fpos > loop_end){
        int seek_pos = (loop_end - SD_BUF_SZ) < 0 ? (loop_end - loop_start) : (loop_end - SD_BUF_SZ);
        f_lseek(&(file->fil), seek_pos);
        f_read(&(file->fil), buffer->buf, SD_BUF_SZ, &n);
        reverseBuffer32((n / 4), buffer->buf);
        if(seek_pos == loop_start){
            f_lseek(&(file->fil), loop_end);
            file->fpos = loop_end;
        }else{
            f_lseek(&(file->fil), seek_pos);
            file->fpos = seek_pos;
        }
        playback_engine->is_pipo_playback_forward = false;
    }else
    {
        int samps_remain = loop_end - file->fpos;
        int samps_to_reverse = (SD_BUF_SZ - samps_remain);
        int seek_pos = loop_end - samps_to_reverse;
        if(seek_pos < 0) seek_pos = 0;
        
        f_read(&(file->fil), buffer->buf, samps_remain, &n);
        f_lseek(&(file->fil), seek_pos);
        UINT tempn;
        f_read(&(file->fil), (buffer->buf + samps_remain), samps_to_reverse, &tempn);
        n += tempn;
        f_lseek(&(file->fil), seek_pos);
        reverseBuffer32((samps_to_reverse / 4), (buffer->buf + samps_remain));
        file->fpos = seek_pos;
        playback_engine->is_pipo_playback_forward = false;
    }

    buffer->rpos = 0;
    buffer->len = n; // amount of bytes read from buffer
}

/**
 * @brief refills in pipo mode when global playback is backwards
 * 
 * @param playback_engine holds the playback function
 * @param buffer audio buffer which gets filled
 * @param file audio file from where data should be read
 */

void fill_buffer_bwd_pipo(voice_play_mode_t* playback_engine, audio_b_t* buffer, audio_f_t* file){
    int loop_start = playback_engine->loop_start;
    int loop_end = playback_engine->loop_end;
    UINT n = 0;
    int file_pos = file->fpos;

    if(file_pos - SD_BUF_SZ >= loop_start){
        int read_pos = file_pos - SD_BUF_SZ;
        f_lseek(&(file->fil), read_pos);
        f_read(&(file->fil), buffer->buf, SD_BUF_SZ, &n);
        reverseBuffer32((n / 4), buffer->buf);
        file->fpos -= n;
        if(file->fpos <= 0){
            file->fpos = loop_end;
            playback_engine->is_pipo_playback_forward = true;
        }
            
        
    }else if(file_pos < loop_start){
        f_lseek(&(file->fil), loop_start);
        f_read(&(file->fil), buffer->buf, SD_BUF_SZ, &n);
        file->fpos = loop_start + n;
        playback_engine->is_pipo_playback_forward = 1;
    }else{
        int samps_to_reverse = file->fpos - loop_start;
        int samps_remain = SD_BUF_SZ - samps_to_reverse;
        
        f_lseek(&(file->fil), loop_start);
        f_read(&(file->fil), buffer->buf, samps_to_reverse, &n);
        reverseBuffer32((n / 4), buffer->buf);
        UINT tempn;
        f_read(&(file->fil), buffer->buf + samps_to_reverse, samps_remain, &tempn);
        n += tempn;
        playback_engine->is_pipo_playback_forward = 1;
        file->fpos = loop_start;      
    }

    buffer->rpos = 0;
    buffer->len = n; // amount of bytes read from buffer

}

void fill_buffer_pipo(void* _playback_engine, void* _buffer, void* _file){
    voice_play_mode_t* playback_engine = (voice_play_mode_t*) _playback_engine;
    audio_b_t* buffer = (audio_b_t*) _buffer;
    audio_f_t* file = (audio_f_t*) _file;
    bool play_dir = playback_engine->is_playback_direction_forward;
    bool play_dir_pipo = playback_engine->is_pipo_playback_forward;

    if((play_dir && play_dir_pipo) || (!play_dir && play_dir_pipo)){
        fill_buffer_fwd_pipo(playback_engine, buffer, file);
    } else if((!play_dir && !play_dir_pipo) || (play_dir && !play_dir_pipo)){
        fill_buffer_bwd_pipo(playback_engine, buffer, file);
    }
}
