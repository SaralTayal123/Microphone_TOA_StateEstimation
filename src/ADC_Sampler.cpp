#include "ADC_Sampler.h"
#include <Arduino.h>
#include <soc/sens_reg.h>
#include <soc/sens_struct.h>

void local_adc1_RTC_control(void)
{
    SENS.sar_read_ctrl.sar1_dig_force = false;      //RTC controller controls the ADC, not digital controller
    SENS.sar_meas_start1.meas1_start_force = true;  //RTC controller controls the ADC,not ulp coprocessor
    SENS.sar_meas_start1.sar1_en_pad_force = true;  //RTC controller controls the data port, not ulp coprocessor
    SENS.sar_touch_ctrl1.xpd_hall_force = true;     // RTC controller controls the hall sensor power,not ulp coprocessor
    SENS.sar_touch_ctrl1.hall_phase_force = true;   // RTC controller controls the hall sensor phase,not ulp coprocessor
}

uint16_t local_adc1_read(int channel)
{
    uint16_t adc_value;
    SENS.sar_meas_start1.sar1_en_pad = (1 << channel); // only one channel is selected
    while (SENS.sar_slave_addr1.meas_status != 0);
    SENS.sar_meas_start1.meas1_start_sar = 0;
    SENS.sar_meas_start1.meas1_start_sar = 1;
    while (SENS.sar_meas_start1.meas1_done_sar == 0);
    adc_value = SENS.sar_meas_start1.meas1_data_sar;
    return adc_value;
}

ADC_Sampler::ADC_Sampler(size_t number, adc1_channel_t adc_channel,
        adc_sample_t buffers[2*NUM_MICS][ADC_BUFFER_SIZE],
        volatile bool buffer_full[NUM_MICS])
{
    mic_number = number;
    adc_channel_ = adc_channel;
    buffer0_ = (adc_sample_t *) &(buffers[2*number]);
    buffer1_ = (adc_sample_t *) &(buffers[2*number+1]);
    index_ = 0;
    buffer0_active_ = true;
    buffer_full_ = &(buffer_full[number]);
}


ADC_Sampler::~ADC_Sampler()
{
}

inline volatile adc_sample_t * ADC_Sampler::get_active_buffer(void)
{
    return (buffer0_active_ ? buffer0_ : buffer1_);
}

void ADC_Sampler::init(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(adc_channel_, ADC_ATTEN_DB_11);
}

void ADC_Sampler::sample(uint32_t * mic0_start_time)
{
    volatile adc_sample_t * buffer = get_active_buffer();
    local_adc1_RTC_control();
    if (mic_number == 0 && index_ == 0)
    {
        *mic0_start_time = micros();
    }
    buffer[index_].adc_sample = local_adc1_read(adc_channel_);
    uint32_t elapsed_time_us = micros() - *mic0_start_time;
    buffer[index_].sample_time_us = (uint16_t) elapsed_time_us;
    if (elapsed_time_us != buffer[index_].sample_time_us)
    {
        // Sample_time_us overflowed
        Serial.printf("%d,%d: %u %u\n", mic_number, index_, elapsed_time_us, buffer[index_].sample_time_us);
    }
    index_++;
    if (index_ >= ADC_BUFFER_SIZE) {
        index_ = 0;
        buffer0_active_ = !buffer0_active_;
        *buffer_full_ = true;
    }
}

volatile adc_sample_t * ADC_Sampler::get_full_buffer(void)
{
    volatile adc_sample_t * buffer = NULL;
    // Serial.printf("Mic %d, before buffer full: %d\n", mic_number, *buffer_full_);
    if (*buffer_full_)
    {
        buffer = buffer0_active_ ? buffer1_ : buffer0_;
        *buffer_full_ = false;
        // Serial.printf("Mic %d, after buffer full: %d\n", mic_number, *buffer_full_);
    }
    return buffer;
}

volatile bool ADC_Sampler::buffer_ready(void)
{
    return *buffer_full_;
}