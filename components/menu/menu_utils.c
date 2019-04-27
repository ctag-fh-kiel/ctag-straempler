#include "menu_utils.h"

void incParamValue(param_data_t* data, int index, int vid, bool* pbs_state, matrix_ui_row_t* matrix, xQueueHandle handle){
    switch (index)
    {
        case SID_TRIG_TYPE:
            data->trig_mode_latch ^= true;
            break;
        case SID_VOLUME:
            if((data->volume + 1) >= 200){
                data->volume = 200;
            }else{
                data->volume += 1;
            }
            //ESP_LOGI("UI","Incremented volume %u", data->volume);
            break;
        case SID_PAN:
            if((data->pan + 128) >= 16384){ // 16384 is +1.0
                data->pan = 16384;
            }else{
                data->pan += 128; 
            }
            //ESP_LOGI("UI","Incremented pan %d", data->pan);
            break;
        case SID_PITCH:
            if((data->pitch + 1) >= 24){
                data->pitch = 24;
            }else{
                data->pitch += 1;
            }
            //ESP_LOGI("UI","Incremented volupitchme %u", data->pitch);
            break;
        case SID_PITCH_CV_ACTIVE:
            
            data->pitch_cv_active = !(data->pitch_cv_active);
            if(data->pitch_cv_active == 0){
                if(vid)
                    matrix[1].amt = 0;
                else
                {
                    matrix[0].amt = 0;
                }
                
            }
            else if(data->pitch_cv_active == 1){
                if(vid)
                    matrix[1].amt = 100;
                else
                {
                    matrix[0].amt = 100;
                }
                
            } 
            //ESP_LOGI("UI","Changed pitch_cv_active %u", data->pitch_cv_active);
            break;
        case SID_PBSPEED:
            if((data->playback_speed + 16) >= 16384){
                data->playback_speed = 16384;
            }else{
                data->playback_speed += 16;
                if(data->playback_speed > 1 && ((*pbs_state) == 0)){
                    *pbs_state = 1;
                    xQueueSend(handle, (void*) pbs_state, portMAX_DELAY);
                }
            }
            break;    
        case SID_DIST_ACTIVE:
            data->dist_active = !(data->dist_active);
            //ESP_LOGI("UI","Changed is_active %u", data->dist_active);
            break;    
        case SID_DIST:
            if((data->dist_drive + 1) >=  255){
                data->dist_drive = 255;
            }else{
                data->dist_drive += 1;
            }
            //ESP_LOGI("UI","Incremented dist_drive %u", data->dist_drive);
            break;
        case SID_DELAY_SEND:
            if((data->delay_send + 1) >= 100){       
                data->delay_send = 100;
            }else{
                data->delay_send += 1;
            }
            break;
        default:
            break;
    }
}

void decParamValue(param_data_t* data, int index, int vid, bool* pbs_state, matrix_ui_row_t* matrix, xQueueHandle handle){
    switch (index)
    {
        case SID_TRIG_TYPE:
            data->trig_mode_latch ^= true;
            break;
        case SID_VOLUME:
            if((data->volume - 1) <= 0){
                data->volume = 0;
            }else{
                data->volume -= 1;
            }
            //ESP_LOGI("UI","Decremented volume %u", data->volume);
            break;
        case SID_PAN:
            if((data->pan - 128) <= -16384){ // -16384 is -1.0
                data->pan = -16384;
            }else{
                data->pan -= 128;
            }
            //ESP_LOGI("UI","Decremented pan %d", data->pan);
            break;
        case SID_PITCH:
            if((data->pitch - 1) <= 0){
                data->pitch = 0;
            }else{
                data->pitch -= 1;
            }
            //ESP_LOGI("UI","Decremented pitch %u", data->pitch);
            break;
        case SID_PITCH_CV_ACTIVE:
            data->pitch_cv_active = !(data->pitch_cv_active);
            if(data->pitch_cv_active == 0){
                if(vid)
                    matrix[1].amt = 0;
                else
                {
                    matrix[0].amt = 0;
                }
                
            }
            else if(data->pitch_cv_active == 1){
                if(vid)
                    matrix[1].amt = 100;
                else
                {
                    matrix[0].amt = 100;
                }
                
            }
            //ESP_LOGI("UI","Changed pitch_cv_active %d", data->volume);
            break;
        case SID_PBSPEED:
            if((data->playback_speed - 16) <= -16384){
                data->playback_speed = -16384;
            }else{
                data->playback_speed -= 16;
                if(data->playback_speed < 0 && ((*pbs_state) == 1)){
                    *pbs_state = 0;
                    xQueueSend(handle, (void*) pbs_state, portMAX_DELAY);
                }
            }
            break;    
        case SID_DIST_ACTIVE:
            data->dist_active = !(data->dist_active);
            //ESP_LOGI("UI","Changed is_active %u", data->dist_active);
            break;    
        case SID_DIST:
            if((data->dist_drive -1) <=  0){       
                data->dist_drive = 0;
            }else{
                data->dist_drive -= 1;
            }
            //ESP_LOGI("UI","Decremented distortion %u", data->dist_drive);
            break;
        case SID_DELAY_SEND:
            if((data->delay_send - 1) <= 0){
                data->delay_send = 0;
            }else{
                data->delay_send -= 1;
            }
            break;
        default:
            break;
    }
}

void incADSRValue(adsr_data_t* data, uint16_t* adsrIndex, int index){
    switch (index)
    {
        case SID_ATTACK:
            if(adsrIndex[0] >= 255){
                adsrIndex[0] = 255;
                data->attack = msLut[adsrIndex[0]];
            }else{
                adsrIndex[0] += 1;
                data->attack = msLut[adsrIndex[0]];
                
            }
            // ESP_LOGI("UI","Incremented attack %u", data->attack);       
            break;
        case SID_DECAY:
            if(adsrIndex[1] >= 255){
                adsrIndex[1] = 255;
                data->decay = msLut[adsrIndex[1]];
            }else{
                adsrIndex[1] += 1;
                data->decay = msLut[adsrIndex[1]];
            }
            // ESP_LOGI("UI","Incremented decay %u", data->decay);         
            break;
        case SID_SUSTAIN:
          if((data->sustain + 1) >= 100){       
                data->sustain = 100;
            }else{
                data->sustain += 1;
            }
            //ESP_LOGI("UI","Incremented sustain %u", data->sustain);
            break;    
        case SID_RELEASE:
            if(adsrIndex[2] >= 255){
                adsrIndex[2] = 255;
                data->release = msLut[adsrIndex[2]];
            }else{
                adsrIndex[2] += 1;
                data->release = msLut[adsrIndex[2]];
            }     
            
            break;    
        default:
            break;
    }
}

void decADSRValue(adsr_data_t* data, uint16_t* adsrIndex, int index){
    switch (index)
    {
        case SID_ATTACK:
            if(adsrIndex[0] <= 0){
                adsrIndex[0] = 0;
                data->attack = msLut[adsrIndex[0]];
            }else{
                adsrIndex[0]--;
                data->attack = msLut[adsrIndex[0]];
            }
            // ESP_LOGI("UI","Decremented attack %u", data->attack);
            break;
        case SID_DECAY:
            if(adsrIndex[1] <= 0){
                adsrIndex[1] = 0;
                data->decay = msLut[adsrIndex[1]];
            }else{
                adsrIndex[1]--;
                data->decay = msLut[adsrIndex[1]];
            }  
            // ESP_LOGI("UI","Decremented decay %u", data->decay);
            break;
        case SID_SUSTAIN:
            if((data->sustain - 1) <= 0){
                data->sustain = 0;
            }else{
                data->sustain -= 1;
            }
            // ESP_LOGI("UI","Decremented sustain %u", data->sustain);
            break;
        case SID_RELEASE:
            if(adsrIndex[2] <= 0){
                adsrIndex[2] = 0;
                data->release = msLut[adsrIndex[2]];
            }else{
                adsrIndex[2]--;
                data->release = msLut[adsrIndex[2]];
            }  
            break;    
        default:
            break;
    }
}

void incFilterValue(filter_data_t* data, int index){
    switch (index)
    {
        case SID_FILTER_ACTIVE:
            data->is_active = !(data->is_active);
            // ESP_LOGI("UI","Changed is_active %u", data->is_active);
            break;
        case SID_BASE:
            if((data->base + 1) >= 511 ){       //Dummy base value 0 - 255
                data->base = 511;
            }else{
                data->base += 1;
            }
            // ESP_LOGI("UI","Incremented base %u", data->base);
            break;
        case SID_WIDTH:
            if((data->width + 1) >= 511){       //Dummy width value 0 - 255
                data->width = 511;
            }else{
                data->width += 1;
            }
            // ESP_LOGI("UI","Incremented width %u", data->width);
            break;
        case SID_Q:
            if((data->q + 1) >= 255){           
                data->q = 255;
            }else{
                data->q += 1;
            }
            //ESP_LOGI("UI","Incremented q %u", data->q);
            break;    
        default:
            break;
    }
}

void decFilterValue(filter_data_t* data, int index){
    switch (index)
    {
        case SID_FILTER_ACTIVE:
            data->is_active = !(data->is_active);
            // ESP_LOGI("UI","Changed is_active %d", data->is_active);
            break;
        case SID_BASE:
            if((data->base - 1) <= 0 ){       //Dummy base value 0 - 255
                data->base = 0;
            }else{
                data->base -= 1;
            }
            // ESP_LOGI("UI","Decremented base %u", data->base);
            break;
        case SID_WIDTH:
            if((data->width - 1) <= 0){       //Dummy width value 0 - 255
                data->width = 0;
            }else{
                data->width -= 1;
            }
            // ESP_LOGI("UI","Decremented width %u", data->width);
            break;
        case SID_Q:
            if((data->q - 1) <= 0){           
                data->q = 0;
            }else{
                data->q -= 1;
            }
            //ESP_LOGI("UI","Decremented q %u", data->q);
            break;    
        default:
            break;
    }
}

void incPlaymodeValue(play_state_data_t* data, int index, xQueueHandle mode_handle){
    switch (index)
    {
        case SID_MODE:
            switch(data->mode){
                case SINGLE:
                    data->mode = LOOP;
                    break;
                case LOOP:
                    data->mode = PIPO;
                    break;
                case PIPO:
                    data->mode = SINGLE;
                    data->loop_start = data->start;
                    data->loop_position = data->start;
                    break;
            }
            // ESP_LOGI("UI","Changed mode %d", data->mode);
            break;
        case SID_START:

            if(data->start < data->loop_end -8){
                if((data->start + 1) >= 800){
                    data->start = 800;
                }else{
                    data->start += 1;
                }

                if(data->mode == SINGLE){
                    //ESP_LOGI("UI", "DATA MODE SINGLE");
                    data->loop_start = data->start;
                    data->loop_position = data->start;
                }
            }

            // ESP_LOGI("UI","Incremented start %u", data->start);
            break;
        case SID_LSTART:
            if(data->loop_start < data->loop_end - 8){
                if((data->loop_start + 1) >= 800){
                    data->loop_start = 800;
                    data->loop_position = 800;
                }else{
                    data->loop_start += 1;
                    data->loop_position += 1;
                    data->loop_length--;
                }
                if(data->mode == SINGLE) data->start = data->loop_start;
            }
                
            // ESP_LOGI("UI","Incremented loop_start %u", data->loop_start);
            break;
        case SID_LEND:
            if((data->loop_end + 1) >= 800){
                data->loop_end = 800;
            }else{
                data->loop_end += 1;
                data->loop_length++;
            }
            
                
            
            // ESP_LOGI("UI","Incremented loop_end %u", data->loop_end);
            break;    
        case SID_LPOSITION:
            if(data->loop_end != 800){
                
                if((data->loop_position + 1) >= 800){
                    data->loop_position = 800;
                }else{
                    data->loop_position += 1;
                }
                data->loop_start = data->loop_position;
                if(data->mode == SINGLE){
                    data->start = data->loop_position;
                }

                if((data->loop_end + 1) >= 800){
                    data->loop_end = 800;
                    data->loop_length = data->loop_end - data->loop_start;
                }else{
                    data->loop_end += 1;
                }
            }
            break;
        default:
            break;
    }
    // xQueueSend(mode_handle, data, 0);
}

void decPlaymodeValue(play_state_data_t* data, int index, xQueueHandle mode_handle){
    switch (index)
    {
        case SID_MODE:
            switch(data->mode){
                case SINGLE:
                    data->mode = PIPO;
                    break;
                case LOOP:
                    data->mode = SINGLE;
                    data->loop_start = data->start;
                    data->loop_position = data->start;
                    break;
                case PIPO:
                    data->mode = LOOP;
                    break;
            }
            
            // ESP_LOGI("UI","Changed mode %d", data->mode);
            break;
        case SID_START:
            if((data->start - 1) <= 0){
                data->start = 0;
            }else{
                data->start -= 1;
            }
            if(data->mode == SINGLE){
                data->loop_start = data->start;
                data->loop_position = data->start;
            }
            // ESP_LOGI("UI","Decremented start %u", data->start);
            break;
        case SID_LSTART:
            if((data->loop_start - 1) <= 0){
                data->loop_start = 0;
                data->loop_position = 0;
            }else{
                data->loop_start -= 1;
                data->loop_position -= 1;
                data->loop_length++;
            }
            if(data->mode == SINGLE){
                data->start = data->loop_start;
                data->loop_position = data->loop_start;
            }
            //ESP_LOGI("UI","Looplength %u", data->loop_length);
            break;
        case SID_LEND:
            if(data->loop_end > data->loop_start + 8 && data->loop_end > data->start + 8){
                if((data->loop_end - 1) <= 0){
                    data->loop_end = 0;
                }else{
                    data->loop_end -= 1;
                    data->loop_length--;
                }
            }
            
            // ESP_LOGI("UI","Decremented loop_end %u", data->loop_end);
            break;    
        case SID_LPOSITION:
            //Loop Position
            if(data->loop_start != 0 && data->loop_position != 0 && data->loop_end > data->start+ 8){
                if((data->loop_position - 1) <= 0){
                    data->loop_position = 0;
                }else{
                    data->loop_position -= 1;
                }

                data->loop_start = data->loop_position;
                if(data->mode == SINGLE){
                    data->start = data->loop_position;
                }
                if((data->loop_end - 1) <= 0){
                    data->loop_end = 0;
                }else{
                    data->loop_end -= 1;
                }
            }

            break;
        default:
            break;
    }
    // xQueueSend(mode_handle, data, 0);
}

void incItemAmount(matrix_ui_row_t* matrix, int source){
    if(matrix[source].dst != MTX_NONE){
        uint8_t* tmp = &(matrix[source].amt);
        if((*tmp  + 1) >= 100){
            *tmp  = 100;
        }else{
            (*tmp)++;
        }
        //ESP_LOGI("UI", "Incr Source: %d, Amount: %d, Param: %d", source, *tmp , matrix[source].dst);
    }

}

void decItemAmount(matrix_ui_row_t* matrix, int source){
    if(matrix[source].dst != MTX_NONE){
        uint8_t* tmp = &(matrix[source].amt);
        if((*tmp - 1) <= 0){
            *tmp = 0;
        } else{
            (*tmp)--;
        }
       //ESP_LOGI("UI", "Decr Source: %d, Amount: %d , Param: %d", source, *tmp , matrix[source].dst);
    }

}

void incDestination(matrix_ui_row_t* matrix, int source){
    int j = 1;
    if(source == 0 || source == 1){
        return; 
    }
    
    for(int i = 0; i < 8; i++){
        if(source == i) continue;

        if(matrix[source].dst + j == matrix[i].dst){
            j++;
            i = -1;
        }  
    }
    if(matrix[source].dst + j >= 37) return;
    matrix[source].dst += j;
    //ESP_LOGI("UI", "Incr - Source: %d, Amount: %d , Dest: %d", source, matrix[source].amt, matrix[source].dst);
}

void decDestination(matrix_ui_row_t* matrix, int source){
    int j = 1;
    if(source == 0 || source == 1){
        return; 
    }

    for(int i = 0; i < 8; i++){
        if(source == i) continue;
        if(matrix[source].dst - j <= 0 || matrix[source].dst - j > 37){
            matrix[source].dst = MTX_NONE;
            return;
        } 
        if(matrix[source].dst - j == matrix[i].dst){
            j++;
            i = -1;
        }  
    }
    matrix[source].dst -= j;
    //ESP_LOGI("UI", "Decr - Source: %d, Amount: %d , Dest: %d", source, matrix[source].amt, matrix[source].dst);
}

void incDelayValue(delay_data_t* delay, int index){
    switch(index){
        case SID_DELAY_ACTIVE:
            delay->is_active = !(delay->is_active);
            // ESP_LOGI("UI","Changed is_active %d", delay->is_active);
            break;
        case SID_DELAY_MODE:
            delay->mode = !(delay->mode);
            break;
        case SID_DELAY_TIME:
            if((delay->time + 2) >= 1500){
                delay->time = 1500;
            }else{
                delay->time += 2;
            }
            // ESP_LOGI("UI","Incremented delay_time_left %u", delay->delay_time_left);
            break;
        case SID_DELAY_PAN:
            if((delay->pan + 1) >= 128){   //Q1.7 128 = 1.0
                delay->pan = 128;
            }else{
                delay->pan += 1;
            }
            // ESP_LOGI("UI","Incremented delay_time_right %u", delay->delay_time_right);
            break;
        case SID_DELAY_FEEDBACK:
            if((delay->feedback + 1) >= 100){   
                delay->feedback = 100;
            }else{
                delay->feedback += 1;
            }
            // ESP_LOGI("UI","Incremented feedback %u", delay->feedback);
            break;
        case SID_DELAY_VOLUME:
            if((delay->volume + 1) >= 200){   //Q2.6 128 = 2.0
                delay->volume = 200;
            }else{
                delay->volume += 1;
            }
            // ESP_LOGI("UI","Incremented delay_volume %u", delay->delay_volume);
            break;
    }
}

void decDelayValue(delay_data_t* delay, int index){
        switch(index){
        case SID_DELAY_ACTIVE:
            delay->is_active = !(delay->is_active);
            // ESP_LOGI("UI","Changed is_active %d", delay->is_active);
            break;
        case SID_DELAY_MODE:
            delay->mode = !(delay->mode);
            break;
        case SID_DELAY_TIME:
            if((delay->time - 2) < 2){
                delay->time = 2;
            }else{
                delay->time -= 2;
            }
            // ESP_LOGI("UI","Decremented delay_time_left %u", delay->delay_time_left);
            break;
        case SID_DELAY_PAN:
            if((delay->pan - 1) < 0){
                delay->pan = 0;
            }else{
                delay->pan -= 1;   
            }
            // ESP_LOGI("UI","Decremented delay_time_right %u", delay->delay_time_right);
            break; 
        case SID_DELAY_FEEDBACK:
            if((delay->feedback - 1) < 0){
                delay->feedback = 0;
            }else{
                delay->feedback -= 1;   
            }
            // ESP_LOGI("UI","Decremented feedback %u", delay->feedback);
            break;
        case SID_DELAY_VOLUME:
            if((delay->volume - 1) < 0){
                delay->volume = 0;
            }else{
                delay->volume -= 1;
            }
            // ESP_LOGI("UI","Decremented delay_volume %u", delay->delay_volume);
            break;
    }
}

void incExtInValue(ext_in_data_t* extin, int index){
    switch(index){
        case SID_EXTIN_ACTIVE:
            extin->is_active = !(extin->is_active);
            break;
        case SID_EXTIN_PAN:
            if((extin->pan + 128) >= 16384){ // 16384 is +1.0
                extin->pan = 16384;
            }else{
                extin->pan += 128; 
            }
            break;
        case SID_EXTIN_DELAY_SEND:
            if((extin->delay_send + 1) >= 100){   
                extin->delay_send = 100;
            }else{
                extin->delay_send += 1;
            }
            break;
        case SID_EXTIN_VOLUME:
            if((extin->volume + 1) >= 200){   //Q2.6 128 = 2.0
                extin->volume = 200;
            }else{
                extin->volume += 1;
            }
            break;
    }
}


void decExtInValue(ext_in_data_t* extin, int index){
        switch(index){
        case SID_EXTIN_ACTIVE:
            extin->is_active = !(extin->is_active);
            break;
        case SID_EXTIN_PAN:
            if((extin->pan - 128) <= -16384){ // -16384 is -1.0
                extin->pan = -16384;
            }else{
                extin->pan -= 128;
            }
            break; 
        case SID_EXTIN_DELAY_SEND:
            if((extin->delay_send - 1) < 0){
                extin->delay_send = 0;
            }else{
                extin->delay_send -= 1;   
            }
            break;
        case SID_EXTIN_VOLUME:
            if((extin->volume - 1) < 0){
                extin->volume = 0;
            }else{
                extin->volume -= 1;
            }
            break;
    }
}

void incSettingsItem(int *tz, int index){
    switch (index)
    {
        case SID_TIMEZONE:
            if(*tz + 1 >= 12) *tz = 12;
            else (*tz)++;
            break;
        default:
            break;
    }
}

void decSettingsItem(int *tz, int index){
    switch (index)
    {
        case SID_TIMEZONE:
            if(*tz - 1 <= -12) *tz = -12;
            else (*tz)--;   
            break;
        default:
            break;
    }
}