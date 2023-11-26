#ifndef ADCSAMPLER_H
#define ADCSAMPLER_H

#include <driver/adc.h>

#define ADC_BUFFER_SIZE         4096UL
#define AVERAGE_SAMPLE_TIME_US  45UL    // Each ADC sample takes 45us
typedef uint16_t adc_sample_t;

class ADC_Sampler
{
private:
    adc1_channel_t adc_channel_;
    adc_sample_t buffer0_[ADC_BUFFER_SIZE];
    adc_sample_t buffer1_[ADC_BUFFER_SIZE];
    volatile size_t index_;
    volatile bool buffer0_active_;
    volatile bool buffer_full_;
    inline adc_sample_t * get_active_buffer(void);

public:
    size_t mic_number;
    ADC_Sampler(size_t number, adc1_channel_t adc_channel);
    ~ADC_Sampler();
    void init(void);
    void sample(void);
    adc_sample_t * get_full_buffer(void);
    bool buffer_ready(void);
};

#endif