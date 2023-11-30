#include <Arduino.h>
#include <esp_task_wdt.h>
#include "ADC_Sampler.h"

SET_LOOP_TASK_STACK_SIZE(8192);

TaskHandle_t sampleTaskHandle;

static adc_sample_t buffers[2*NUM_MICS][ADC_BUFFER_SIZE];
static volatile bool buffer_full[NUM_MICS] = {false, false, false, false, false};
static bool peak_in_prev[NUM_MICS] = {false, false, false, false, false};
static ADC_Sampler mic0(0, ADC1_CHANNEL_0, buffers, buffer_full);
static ADC_Sampler mic1(1, ADC1_CHANNEL_3, buffers, buffer_full);
static ADC_Sampler mic2(2, ADC1_CHANNEL_4, buffers, buffer_full);
static ADC_Sampler mic3(3, ADC1_CHANNEL_5, buffers, buffer_full);
static ADC_Sampler mic4(4, ADC1_CHANNEL_6, buffers, buffer_full);

void sampleTask(void *param)
{
  static uint32_t mic0_start_time = 0;
  // Disable watchdog
  disableCore0WDT();
  while (true)
  {
    mic0.sample(&mic0_start_time);
    mic1.sample(&mic0_start_time);
    mic2.sample(&mic0_start_time);
    mic3.sample(&mic0_start_time);
    mic4.sample(&mic0_start_time);
  }
}

size_t getTimeOfPeak(ADC_Sampler mic)
{
  bool *_peak_in_prev = &(peak_in_prev[mic.mic_number]);
  uint16_t max;
  size_t maxIndex;
  if (mic.buffer_ready())
  {
    volatile adc_sample_t *buffer = mic.get_full_buffer();
    // Serial.printf("Mic %d: Buffer ready at %p\n", mic.mic_number, buffer);
    max = 0;
    maxIndex = -1;
    if (!(*_peak_in_prev)) {     
      for (size_t i = 0; i < ADC_BUFFER_SIZE; i++)
      {
        if (buffer[i].adc_sample > max)
        {
          max = buffer[i].adc_sample;
        }
        if (maxIndex == -1 && (buffer[i].adc_sample > 4000 || buffer[i].adc_sample < 100)) {
          // max = buffer[i].adc_sample;
          maxIndex = i;
        }
      }
    } else {
      // Serial.printf("Mic %d peak in prev is true\n", mic.mic_number);
      // if (mic.mic_number == 0){
      //   Serial.printf("Buffer for Mic %d\n", mic.mic_number);
      //   for (size_t i = 0; i < ADC_BUFFER_SIZE; i++)
      //   {
      //     Serial.println(buffer[i].adc_sample);
      //   }
      // }
    }

    if (maxIndex != -1) {
      // Serial.printf("Mic %d peak in prev is being set to true\n", mic.mic_number);
      *_peak_in_prev = true;
      size_t time_us = buffer[maxIndex].sample_time_us;
      // Serial.printf("Mic %d: ADC value: %d, Index: %d\n", mic.mic_number, max, maxIndex);
      return time_us;
    } else {
      *_peak_in_prev = false;
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
  ulong start_time = micros();
  size_t mic_0_time = getTimeOfPeak(mic0);
  size_t mic_1_time = getTimeOfPeak(mic1);
  size_t mic_2_time = getTimeOfPeak(mic2);
  size_t mic_3_time = getTimeOfPeak(mic3);
  size_t mic_4_time = getTimeOfPeak(mic4);
  ulong end_time = micros();
  
  // size_t min_mic_time = min(min(min(min(mic_0_time, mic_1_time), mic_2_time), mic_3_time), mic_4_time);
  if (mic_0_time != -1 && mic_1_time != -1 && mic_2_time != -1 && mic_3_time != -1 && mic_4_time != -1)
  {
    // Serial.println("Time taken: " + String(end_time - start_time) + " us");
    Serial.printf("%d, %d, %d, %d, %d\n", mic_0_time, mic_1_time, mic_2_time, mic_3_time, mic_4_time);
  }
}