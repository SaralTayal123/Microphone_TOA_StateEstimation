#include "ADC_Sampler.h"

ADC_Sampler::ADC_Sampler(adc1_channel_t adc_channel)
{
    adc_channel_ = adc_channel;
    index_ = 0;
    buffer0_active_ = true;
    buffer_full_ = false;
}


ADC_Sampler::~ADC_Sampler()
{
}

inline adc_sample_t * ADC_Sampler::get_active_buffer(void)
{
    return (buffer0_active_ ? buffer0_ : buffer1_);
}

void ADC_Sampler::init(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(adc_channel_, ADC_ATTEN_DB_11);
}

void ADC_Sampler::sample(void)
{
    adc_sample_t sample = adc1_get_raw(adc_channel_);
    adc_sample_t * buffer = get_active_buffer();
    buffer[index_++] = sample;
    if (index_ >= ADC_BUFFER_SIZE) {
        index_ = 0;
        buffer0_active_ = !buffer0_active_;
        buffer_full_ = true;
    }
}

adc_sample_t * ADC_Sampler::get_full_buffer(void)
{
    uint16_t * buffer = NULL;
    if (buffer_full_)
    {
        if (buffer0_active_)
        {
            buffer = buffer1_;
        }
        else
        {
            buffer = buffer0_;
        }
        buffer_full_ = false;
    }
    return buffer;
}

bool ADC_Sampler::buffer_ready(void)
{
    return buffer_full_;
}