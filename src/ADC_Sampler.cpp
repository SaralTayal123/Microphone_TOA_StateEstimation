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

int local_adc1_read(int channel)
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

ADC_Sampler::ADC_Sampler(size_t number, adc1_channel_t adc_channel)
{
    mic_number = number;
    adc_channel_ = adc_channel;
    index_ = 0;
    buffer0_active_ = true;
    buffer_full_ = false;
}


ADC_Sampler::~ADC_Sampler()
{
    // if (buffer0_ != NULL)
    // {
    //     free(buffer0_);
    // }
    // if (buffer1_ != NULL)
    // {
    //     free(buffer1_);
    // }
}

inline adc_sample_t * ADC_Sampler::get_active_buffer(void)
{
    return (buffer0_active_ ? buffer0_ : buffer1_);
}

void ADC_Sampler::init(void)
{
    // buffer0_ = (adc_sample_t *) malloc(sizeof(adc_sample_t)*ADC_BUFFER_SIZE);
    // buffer1_ = (adc_sample_t *) malloc(sizeof(adc_sample_t)*ADC_BUFFER_SIZE);
    if (buffer0_ == NULL || buffer1_ == NULL)
    {
        Serial.printf("Mic %d: Buffer allocation failed\n", mic_number);
        while(true);
    }
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(adc_channel_, ADC_ATTEN_DB_11);
}

void ADC_Sampler::sample(void)
{
    adc_sample_t * buffer = get_active_buffer();
    local_adc1_RTC_control();
    buffer[index_++] = local_adc1_read(adc_channel_);
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