#include <Arduino.h>
#include <esp_task_wdt.h>
#include "ADC_Sampler.h"

SET_LOOP_TASK_STACK_SIZE(16384);

TaskHandle_t sampleTaskHandle;

static adc_sample_t buffers[2*NUM_MICS][ADC_BUFFER_SIZE];
static volatile bool buffer_full[NUM_MICS] = {false, false, false, false, false};
static ADC_Sampler mic0(0, ADC1_CHANNEL_0, buffers, buffer_full);
static ADC_Sampler mic1(1, ADC1_CHANNEL_3, buffers, buffer_full);
static ADC_Sampler mic2(2, ADC1_CHANNEL_4, buffers, buffer_full);
static ADC_Sampler mic3(3, ADC1_CHANNEL_5, buffers, buffer_full);
static ADC_Sampler mic4(4, ADC1_CHANNEL_6, buffers, buffer_full);

void sampleTask(void *param)
{
  // Disable watchdog
  disableCore0WDT();
  while (true)
  {
    ulong times = 10000UL;
    for (ulong i = 0; i < times; i++)
    {
      mic0.sample();
      mic1.sample();
      mic2.sample();
      mic3.sample();
      mic4.sample();
    }
  }
}

size_t getTimeOfPeak(ADC_Sampler mic)
{
  adc_sample_t max;
  size_t maxIndex;
  if (mic.buffer_ready())
  {
    volatile adc_sample_t *buffer = mic.get_full_buffer();
    // Serial.printf("Mic %d: Buffer ready at %p\n", mic.mic_number, buffer);
    max = 0;
    maxIndex = -1;
    for (size_t i = 0; i < ADC_BUFFER_SIZE; i++)
    {
      if (buffer[i] > max && buffer[i] > 4000) {
        max = buffer[i];
        maxIndex = i;
      }
    }
    if (maxIndex != -1) {
      size_t time_us = (maxIndex * NUM_MICS) * AVERAGE_SAMPLE_TIME_US - (mic.mic_number * AVERAGE_SAMPLE_TIME_US);
      // TODO: Apprach this with memory of when the last peak was 
      Serial.printf("Mic %d: ADC value: %d, Index: %d\n", mic.mic_number, max, maxIndex);
      return time_us;
    }
  }
  return -1;
}

void setup()
{
  // Start the serial communication
  Serial.begin(500000);

  // Initialize the ADCs
  mic0.init();
  mic1.init();
  mic2.init();
  mic3.init();
  mic4.init();

  xTaskCreatePinnedToCore(sampleTask, "Sample", 4096, NULL, 1, &sampleTaskHandle, 0);
}

void loop()
{
  size_t mic_0_time = getTimeOfPeak(mic0);
  size_t mic_1_time = getTimeOfPeak(mic1);
  size_t mic_2_time = getTimeOfPeak(mic2);
  size_t mic_3_time = getTimeOfPeak(mic3);
  size_t mic_4_time = getTimeOfPeak(mic4);

  size_t min_mic_time = min(min(min(min(mic_0_time, mic_1_time), mic_2_time), mic_3_time), mic_4_time);

  if (mic_0_time != -1)
  Serial.printf("Time of peak: %d, %d, %d, %d, %d \n", mic_0_time - min_mic_time, mic_1_time - min_mic_time, mic_2_time - min_mic_time, mic_3_time - min_mic_time, mic_4_time - min_mic_time);
}