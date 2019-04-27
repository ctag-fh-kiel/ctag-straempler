#include "modulation.h"

void init_ui_params(ui_param_holder_t* params){
    params->delay_send = 0.0f;
    params->dist_amp = 1.0f;
    params->filter_base_index = 255;
    params->filter_width_index = 255;
    params->pan = 0.0f;
    params->is_pitch_cv_active = false;
    params->is_shaper_active = false;
    params->playback_speed = 1;
    params->resonance = 1.0f;
    params->volume = 1.0f;
    params->pitch = 12;
    params->attack_time = 44100.0f;
    params->decay_time = 22050.0f;
    params->release_time = 88200.0f;
    params->sustain_level = 0.7f;
    params->sample_start = 0;
    params->loop_start = 0;
    params->loop_end = 0;
    params->mode = SINGLE;
}

void parse_play_direction(xQueueHandle* queue, TaskHandle_t* task_handle, voice_t* voice, int vid){
    bool dir;

    if(xQueueReceive(*queue, &dir, 0)){
        
        if(dir != voice->playback_engine.is_playback_direction_forward)
        {
            voice->playback_engine.is_playback_direction_forward = dir;
            xTaskNotify(*task_handle, 6 << vid, eSetBits); // trigger buffer reload
        }
    }

    
}


void parse_voice_param_data(xQueueHandle* queue, voice_t* voice, ui_param_holder_t* ui_params, param_data_t* params){
    if(xQueueReceive(*queue ,params, 0)){
            
            //Scale fixed params appropriately
            ui_params->volume = params->volume * 0.01;
            ui_params->is_pitch_cv_active = params->pitch_cv_active;
            if(!ui_params->is_pitch_cv_active){
                ui_params->playback_speed = params->pitch;
                voice->playback_engine.pitch_increment = pitch_lut_24[params->pitch];
                ui_params->playback_speed = fabs(FixedToFloat_14(params->playback_speed));
                voice->playback_engine.pitch_increment *= ui_params->playback_speed;
            }
            ui_params->delay_send = params->delay_send*0.01;

            ui_params->attack_time = params->adsr.attack;
            ui_params->decay_time = params->adsr.decay;
            ui_params->sustain_level = params->adsr.sustain;
            ui_params->release_time = params->adsr.release;

            voice->adsr.attack_time = params->adsr.attack * 44.1f;
            voice->adsr.decay_time = params->adsr.decay * 44.1f ;
            voice->adsr.sustain_level = params->adsr.sustain * 0.01;
            voice->adsr.release_time = params->adsr.release * 44.1;
            
            ui_params->is_shaper_active = params->dist_active;
            ui_params->pan = FixedToFloat_14(params->pan);


            if(ui_params->is_shaper_active)
                ui_params->dist_amp = dist_q_lut[params->dist_drive];

            voice->highpass_filter->is_active = params->filter.is_active;
            voice->lowpass_filter->is_active = voice->highpass_filter->is_active;
            ui_params->filter_base_index = params->filter.base;
            ui_params->filter_width_index = params->filter.width;
            ui_params->resonance = dist_q_lut[params->filter.q];   
     }
}

void parse_play_state_data(xQueueHandle* queue, play_state_data_t* play_state_data, ui_param_holder_t* ui_params, TaskHandle_t* task_handle, voice_t* voice, audio_f_t* file, void (**play_modes)(void*,void*,void*), int vid){
    
    if(xQueueReceive(*queue, play_state_data, 0))
    {
        
        uint32_t length = file->fsize;
                
        float tmp = (FixedToFloat_3(play_state_data->start) / (float) 100) * length;
        uint32_t temp_comp = tmp / 4;
        temp_comp *= 4;
        if(temp_comp != voice->playback_engine.sample_start || temp_comp != ui_params->sample_start)
        {

            ui_params->sample_start = temp_comp;
            voice->playback_engine.sample_start = temp_comp;
        }
                    
        tmp = (FixedToFloat_3(play_state_data->loop_start) / (float) 100) * length;
        temp_comp = tmp / 4;
        temp_comp *= 4;
        if(temp_comp != voice->playback_engine.loop_start || temp_comp != ui_params->loop_start)
        {
            ui_params->loop_start = temp_comp;
            voice->playback_engine.loop_start = temp_comp;
        }

        tmp = (FixedToFloat_3(play_state_data->loop_end) / (float) 100) * length;
        temp_comp = tmp / 4;
        temp_comp *= 4;
    
        if(temp_comp != voice->playback_engine.loop_end || temp_comp != ui_params->loop_end)
        {
            ui_params->loop_end = temp_comp;
            voice->playback_engine.loop_end = temp_comp;
        }

        if((play_state_data->mode >= 0) && (play_state_data->mode < 3)){
            voice->playback_engine.play_mode = play_modes[play_state_data->mode];
            ui_params->mode = play_state_data->mode;
            voice->playback_engine.mode = play_state_data->mode;
        }
        else
        {
            ESP_LOGE("Parse Playback Data", "Error, index of playmode data must be between 0 and 2");
        }

    }
}

