#include <Arduino.h>
#include <chrono>
#include "ADC_Sampler.h"

#define NUM_MICS    5UL

TaskHandle_t sampleTaskHandle;
TaskHandle_t processTaskHandle;

ADC_Sampler mic0(0, ADC1_CHANNEL_0);
ADC_Sampler mic1(1, ADC1_CHANNEL_3);
ADC_Sampler mic2(2, ADC1_CHANNEL_4);
ADC_Sampler mic3(3, ADC1_CHANNEL_5);
ADC_Sampler mic4(4, ADC1_CHANNEL_6);

void sampleTask(void *param)
{
  while (true)
  {
    // ulong total = 0;
    // ulong times = 10000UL;
    // for (ulong i = 0; i < times; i++)
    // {
    //   auto start = std::chrono::system_clock::now();
      Serial.println("Sampling");
      mic0.sample();
      mic1.sample();
      mic2.sample();
      mic3.sample();
      mic4.sample();
    //   auto end = std::chrono::system_clock::now();
    //   std::chrono::duration<double> diff = end - start;
    //   auto us = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
    //   total += us;
    // }
    // Serial.printf("%d\n", total / times);
  }
}

void getTimeOfPeak(ADC_Sampler mic)
{
  Serial.printf("Hello from mic %d\n", mic.mic_number);
  // adc_sample_t max;
  // size_t maxIndex;
  // if (mic.buffer_ready())
  // {
  //   Serial.printf("Mic %d: Buffer ready\n", mic.mic_number);
  //   adc_sample_t *buffer = mic.get_full_buffer();
  //   Serial.printf("Mic %d: Buffer at %p\n", mic.mic_number, buffer);
  //   max = buffer[0];
  //   maxIndex = 0;
  //   for (size_t i = 1; i < ADC_BUFFER_SIZE; i++)
  //   {
  //     if (buffer[i] > max) {
  //       max = buffer[i];
  //       maxIndex = i;
  //     }
  //   }
  //   size_t time_us = (maxIndex * NUM_MICS - mic.mic_number) * AVERAGE_SAMPLE_TIME_US;
  //   Serial.printf("%d: %d, %d\n", mic.mic_number, max, time_us);
  // }
}

void processTask(void *param)
{
  Serial.println("Peaks task Starting");
  while(true) {
    getTimeOfPeak(mic0);
    // getTimeOfPeak(mic1);
    // getTimeOfPeak(mic2);
    // getTimeOfPeak(mic3);
    // getTimeOfPeak(mic4);
    vTaskDelay(1);
  }
}

void setup()
{
  // Start the serial communication
  Serial.begin(115200);

  // Initialize the ADCs
  mic0.init();
  mic1.init();
  mic2.init();
  mic3.init();
  mic4.init();
  Serial.println("Init finished. Starting scheduler");

  xTaskCreatePinnedToCore(sampleTask, "Sample", 2048, NULL, 1, &sampleTaskHandle, 0);
  xTaskCreatePinnedToCore(processTask, "Peaks", 16484, NULL, 1, &processTaskHandle, 1);
}

void loop()
{
}
