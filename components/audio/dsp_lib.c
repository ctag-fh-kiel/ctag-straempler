#include "dsp_lib.h"
#include "esp_log.h"

void init_filter(Filter_t* filter, filtertype_t f_type){
    filter->filter_left = (BiQuad_t*) malloc(sizeof(BiQuad_t));
    filter->filter_right = (BiQuad_t*) malloc(sizeof(BiQuad_t));

    filter->filter_left->coeffs = (BiQuadCoeffs_t*) malloc(sizeof(BiQuadCoeffs_t));
    filter->filter_right->coeffs = (BiQuadCoeffs_t*) malloc(sizeof(BiQuadCoeffs_t));
    filter->is_active = 0;
    //Initialise Filter Data and calculate Coeffs
    filter->filter_left->coeffs->a1 = 0.0;
    filter->filter_left->coeffs->a2 = 0.0;
    filter->filter_left->coeffs->b0 = 1.0;
    filter->filter_left->coeffs->b1 = 0.0;
    filter->filter_left->coeffs->b2 = 0.0;

    filter->filter_right->coeffs->a1 = 0.0;
    filter->filter_right->coeffs->a2 = 0.0;
    filter->filter_right->coeffs->b0 = 1.0;
    filter->filter_right->coeffs->b1 = 0.0;
    filter->filter_right->coeffs->b2 = 0.0;

    switch(f_type){
        case lowpass:
            filter->filter_left->cutoff = 20000;
            filter->filter_right->cutoff = 20000;
            break;
        case highpass:
            filter->filter_left->cutoff = 20;
            filter->filter_right->cutoff = 20;
            break;
    }
    

    filter->filter_left->resonance = 0.707;
    filter->filter_right->resonance = 0.707;

    filter->filter_left->sample_rate = 44100;
    filter->filter_right->sample_rate = 44100;

    filter->filter_left->type = f_type;
    filter->filter_right->type = f_type;

    reset_filter(filter);

    calculate_coeffs(filter);
}

void calculate_coeffs(Filter_t *filter){

    

    BiQuad_t* filter_left = filter->filter_left;
    BiQuad_t* filter_right = filter->filter_right;

    float cutoff = filter_left->cutoff;
    float resonance = filter_left->resonance;
    float sample_rate = filter_left->sample_rate;
    float a1 = 0.0;
    float a2 = 0.0;
    float b0 = 1.0;
    float b1 = 0.0;
    float b2 = 0.0;
    float norm = 0.0;
    
    
    float lambda = fasttan(M_PI * cutoff / sample_rate);
    
    switch (filter_left->type) {
        case lowpass:
            norm = 1 / (1 + lambda / resonance + lambda * lambda);
            b0 = lambda * lambda * norm;
            b1 = 2 * b0;
            b2 = b0;
            a1 = 2 * (lambda * lambda - 1) * norm;
            a2 = (1 - lambda / resonance + lambda * lambda) * norm;
            break;
            
        case highpass:
            norm = 1 / (1 + lambda / resonance + lambda * lambda);
            b0 = 1 * norm;
            b1 = -2 * b0;
            b2 = b0;
            a1 = 2 * (lambda * lambda - 1) * norm;
            a2 = (1 - lambda / resonance + lambda * lambda) * norm;
            break;
    }
    
    filter_left->coeffs->a1 = a1;
    filter_left->coeffs->a2 = a2;
    filter_left->coeffs->b0 = b0;
    filter_left->coeffs->b1 = b1;
    filter_left->coeffs->b2 = b2;

    filter_right->coeffs->a1 = a1;
    filter_right->coeffs->a2 = a2;
    filter_right->coeffs->b0 = b0;
    filter_right->coeffs->b1 = b1;
    filter_right->coeffs->b2 = b2;
}

void reset_filter(Filter_t *filter){
    //Reset z^-1 registers to zero

    filter->filter_left->z1 = 0;
    filter->filter_left->z2 = 0;
    filter->filter_right->z1 = 0;
    filter->filter_right->z2 = 0;
}

int32_t round_float_int(float in){
    if(in >= 0.5)
        return (int32_t) in + 1;
    else
        return (int32_t) in;
}

int32_t hard_limit(int32_t val)
{
        if (val > 8388607)
            val = 8388607;
        if (val < -8388608)
            val = -8388608;

        return  val;
}

void filter_sound(Filter_t *filter, float* out_left, float* out_right){

    // Get Filters for both Channels
    BiQuad_t* filter_left = filter->filter_left;
    BiQuad_t* filter_right = filter->filter_right;

    float in_left = *out_left;
    float in_right = *out_right;

    *out_left = in_left * filter_left->coeffs->b0 + filter_left->z1;
    filter_left->z1 = in_left * filter_left->coeffs->b1 + filter_left->z2 - filter_left->coeffs->a1 * *out_left;
    filter_left->z2 = in_left * filter_left->coeffs->b2 - filter_left->coeffs->a2 * *out_left;
        
         
    *out_right = in_right * filter_right->coeffs->b0 + filter_right->z1;
    filter_right->z1 = in_right * filter_right->coeffs->b1 + filter_right->z2 - filter_right->coeffs->a1 * *out_right;
    filter_right->z2 = in_right * filter_right->coeffs->b2 - filter_right->coeffs->a2 * *out_right;
   
}

void set_resonance(Filter_t* filter, float resonance){
    filter->filter_left->resonance = resonance;
    filter->filter_right->resonance = resonance;

    calculate_coeffs(filter);
}
void set_cutoff(Filter_t* filter, float cutoff){
    filter->filter_left->cutoff = cutoff;
    filter->filter_right->cutoff = cutoff;

    calculate_coeffs(filter);
}

void set_filter_params(Filter_t* filter, float cutoff, float resonance){
    filter->filter_left->cutoff = cutoff;
    filter->filter_right->cutoff = cutoff;
    filter->filter_left->resonance = resonance;
    filter->filter_right->resonance = resonance;

    calculate_coeffs(filter);
}

void apply_volume(uint16_t* ctrlData, int16_t* buf, voice_t* voice,int len){
    uint16_t data = ctrlData[outer_left] >> 2;

    for(int i = 0; i < len; i++){
        int32_t out;
        out = (buf[i] * data) >> 10;
        buf[i] = (int16_t)out;
    }
}

float fasttan(float x){  
    #define TAN_C1 211.849369664121f     
    #define TAN_C2 -12.5288887278448f     
    #define TAN_C3 269.7350131214121f     
    #define TAN_C4 -71.4145309347748f     
    int octant = (int) x / PI_4;     
    float v = octant == 0 ? (x * FOUR_OVER_PI) : ((PI_2-x) * FOUR_OVER_PI);     
    float v2 = v*v;     
    v = v * (TAN_C1 + TAN_C2 * v2)/(TAN_C3 + v2 * (TAN_C4 + v2));     
    if(octant>0)         
        return 1.0f/v;     
    return v;
}

float fasttanh(float x){     
    float x2 = x*x;
    return x * ( 27.f + x2 ) / ( 27.f + 9.f * x2 );
}

float distort_sound(WaveShaper_t* shaper, float value){
    if(shaper->is_active)
        return fasttanh(value * shaper->amp_fator_positive);
    else
        return value;
}

void set_cutoff_values(voice_t* voice, uint16_t* ctrl_data, int vid)
{
    float cutoff_hp = 0;
    float cutoff_lp = 0;
    float offset = 0;
    int offset_pos = 0;
    if(vid)
    {
        offset_pos = ((ctrl_data[outer_right] >> 2) - 512);
        
        if(offset_pos < 0)
        {
            
            offset = -1 * negative_freq_lut[(-1 * offset_pos)];
        }
        else{
            offset = poti_vals[(offset_pos)];
        }
        cutoff_hp = poti_vals[(ctrl_data[inner_right] >> 3)] + offset;
        cutoff_lp = poti_vals[511 - (ctrl_data[inner_right] >> 3)] + offset;
    }
    else
    {
        offset_pos = ((ctrl_data[inner_left] >> 2) - 512);
        
        if(offset_pos < 0)
        {
            
            offset = -1 * negative_freq_lut[(-1 * offset_pos)];
        }
        else{
            offset = poti_vals[(offset_pos)];
        }
        
        cutoff_hp = poti_vals[(ctrl_data[outer_left] >> 3)] + offset;
        cutoff_lp = poti_vals[511 - (ctrl_data[outer_left] >> 3)] + offset;
    }

    if(cutoff_hp > 18000){
        cutoff_hp = 18000;
    }
    if(cutoff_lp < 20){
        cutoff_lp = 20;
    }
    if(cutoff_lp > 18000){
        cutoff_lp = 18000;
    }
    if(cutoff_hp < 20){
        cutoff_hp = 20;
    }

    
    
    set_cutoff(voice->highpass_filter, cutoff_hp);
    set_cutoff(voice->lowpass_filter, cutoff_lp);
    
}

void set_filter_values_ui(voice_t* voice, uint16_t base, uint16_t width, float q, int vid){
    float cutoff_hp;
    float cutoff_lp;
    
    if(q <= 0.0) q = 1.0;
    
    cutoff_hp = poti_vals[base] - poti_vals[width] / 2;
    cutoff_lp = poti_vals[base] + poti_vals[width] / 2;

    if(cutoff_hp > 18000){
        cutoff_hp = 18000;
    }
    if(cutoff_lp < 20){
        cutoff_lp = 20;
    }
    if(cutoff_lp > 18000){
        cutoff_lp = 18000;
    }
    if(cutoff_hp < 20){
        cutoff_hp = 20;
    }
    
    
    set_filter_params(voice->highpass_filter, cutoff_hp, q);
    set_filter_params(voice->lowpass_filter, cutoff_lp, q);
}

inline void apply_voice_fx(float* buf, int len, voice_t* voice, uint16_t* ctrl_data, int vid, float vol){
    float sample_left, sample_right;
    set_cutoff_values(voice, ctrl_data, vid);
    float amp = 1.0f;
    

    for(int i = 0; i < len/2; i++)
    {
        sample_left = buf[2*i];
        sample_right = buf[2*i+1];


        // float dist_amp = (ctrl_data[outer_left] >> 3) / (float) 511 * 5.0f;
        
        // if(dist_amp < 0.2)
        //     dist_amp = 0.2;

        
        // sample_left = fasttanh(dist_amp * sample_left);
        // sample_right = fasttanh(dist_amp * sample_right);
        

        if(voice->lowpass_filter->is_active)             
            filter_sound(voice->lowpass_filter, &sample_left, &sample_right);         
        if(voice->highpass_filter->is_active)             
            filter_sound(voice->highpass_filter, &sample_left, &sample_right);
        // float amp = (ctrl_data[outer_left] >> 3) / (float) 511 * 2;

        
        
        // stereo_balance(ctrl_data, &sample_left, &sample_right);

        // sample_left *= vol;
        // sample_right *= vol;

        buf[2*i] = sample_left;
        buf[2*i+1] = sample_right;
    }
}

void stereo_balance(float pan_val, float* sample_left, float* sample_right){
    if(pan_val < 0.0f)
    {
        pan_val *= -1.0f;

        *sample_left *= 1.0f;
        *sample_right *= (1.f - pan_val);
    }
    else if(pan_val > 0.0f)
    {
        *sample_left *= (1.f - pan_val);
        *sample_right *= 1.0f;
    }
    else
    {
        *sample_left *= 1.0f;
        *sample_right *= 1.0f;
    }
}

void init_shaper(WaveShaper_t* shaper){
    shaper->is_active = 0;
    set_distortion_amp(shaper, 0.2);
}

void set_distortion_amp(WaveShaper_t* shaper, float value){
    shaper->amp_fator_negative = value;
    shaper->amp_fator_positive = value;
}

void init_pan(PanPos_t* pan){
    pan->angle = 0;
    pan->position = 0;
}

void set_position(PanPos_t* pan, float pos){
    pan->position = pos * PIOVR_2;
    pan->angle = pos * 0.5;
}

// Taken from https://github.com/francoisbecker/fb-utils/blob/master/include/fbu/math_utils.hpp

inline float fast_sin(float x)
    {
        float x2 = x * x;
        return x * (0.9992431032114582f + x2 * (- 0.16534994905656308f + x2 * (0.007873446865159112f - 0.00013964149131248947f * x2)));
}

void process_left_channel(PanPos_t* pan, float* left_channel)
{
	(*left_channel) *= ROOT_2_OVR_2 * (fast_sin((M_PI / 2) - pan->angle) - fast_sin(pan->angle));
}

void process_right_channel(PanPos_t* pan, float* right_channel)
{
	(*right_channel) *= ROOT_2_OVR_2 * (fast_sin((M_PI / 2) - pan->angle) + fast_sin(pan->angle));
	
}

    inline void buffer_copy(float* dst, int dst_start, float* src, int src_start, int len){
    for(int i = 0; i < len; i++){
        dst[dst_start + i] = src[src_start + i];
    }
}

inline void buffer_copy_stereo(float* dst, int32_t dst_start,float* src, int32_t src_start, int32_t len, int32_t channel){
    for(int i = 0; i < len / 2; i++){
        int index_dst = 2 * i + dst_start + channel;
        int index_src = 2 * i + src_start + channel;
        dst[index_dst] = src[index_src];
    }
}

void buffer_add(float* dst, int dst_start, float* src, int src_start, int len){
    for(int i = 0; i < len ; i++){
        dst[dst_start + i] += src[src_start + i];
    }
}

int32_t modulateTimeParameter(int index, uint16_t* ctrl_data, matrix_row_t* matrix, uint32_t ui_param, uint32_t max_time){
    float amount = 0.0;
    float input_value = 0.0;
    int src = -1;
    uint32_t ui_time = ui_param;
    

    for(size_t i = 0; i < 8; i++)
    {
        if(matrix[i].dst == index){
            amount = matrix[i].amt;
            if(amount == 0.0) return ui_time;
            src = i;
            break;
        }
    }

    if(src >= 0 && src < 8){
        
        input_value = FixedToFloat_9((ctrl_data[src] >> 3));
        input_value *= max_time;

        amount *= max_time;

        if(input_value + ui_time >= amount){
            return amount;
        }
        else
        {
            return input_value + ui_time;
        }
    }
    
    return ui_time;
}


float modulate_unipolar(float center, float max_min, float src, float amt){
    float limit = (max_min - center) * amt;

    float add = (limit) * src;

    float output = center + add;

    // ESP_LOGE("All", "center: %f, max_min: %f, src:%f, amt: %f, output: %f", center, max_min, src, amt, output);

    if(max_min > 0.0f){
        if(output > max_min) return max_min;
    }
    else{
        if(output < max_min) return max_min;
    }

    return output;
}