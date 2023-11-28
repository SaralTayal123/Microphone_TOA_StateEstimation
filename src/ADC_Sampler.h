#ifndef ADCSAMPLER_H
#define ADCSAMPLER_H

#include <driver/adc.h>

#define ADC_BUFFER_SIZE 512UL // Decreased so that us fits within uint16_t
#define NUM_MICS        5UL
typedef struct {
    uint16_t adc_sample;
    uint16_t sample_time_us;
} adc_sample_t;

class ADC_Sampler
{
private:
    adc1_channel_t adc_channel_;
    volatile adc_sample_t *buffer0_;
    volatile adc_sample_t *buffer1_;
    volatile size_t index_;
    volatile bool buffer0_active_;
    inline volatile adc_sample_t * get_active_buffer(void);

public:
    volatile bool *buffer_full_;
    size_t mic_number;
    ADC_Sampler(
        size_t number, adc1_channel_t adc_channel,
        adc_sample_t buffers[2*NUM_MICS][ADC_BUFFER_SIZE],
        volatile bool buffer_full[NUM_MICS]
    );
    ~ADC_Sampler();
    void init(void);
    void sample(uint32_t * mic0_start_time);
    volatile adc_sample_t * get_full_buffer(void);
    volatile bool buffer_ready(void);
};

#endif