#ifndef ADCSAMPLER_H
#define ADCSAMPLER_H

#include <driver/adc.h>

#define ADC_BUFFER_SIZE     4096
typedef uint16_t adc_sample_t;

class ADC_Sampler
{
private:
    adc2_channel_t adc_channel_;
    adc_sample_t buffer0_[ADC_BUFFER_SIZE];
    adc_sample_t buffer1_[ADC_BUFFER_SIZE];
    size_t index_;
    bool buffer0_active_;
    bool buffer_full_;
    inline adc_sample_t * get_active_buffer(void);

public:
    ADC_Sampler(adc2_channel_t adc_channel);
    ~ADC_Sampler();
    void init(void);
    void sample(void);
    adc_sample_t * get_full_buffer(void);
    bool buffer_ready(void);
};

#endif