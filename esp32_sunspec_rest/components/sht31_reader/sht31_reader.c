#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/semphr.h"

#include <esp_system.h>
#include <string.h>
#include <esp_err.h>
#include <sht3x.h>

#include "sht31_reader.h"

void sht31_reader(void *pvParameters);

static sht3x_t dev;
float sht_temperature;
float sht_humidity;

SemaphoreHandle_t xSemaphore_sht = NULL;

void sht31_begin()
{
    ESP_ERROR_CHECK(i2cdev_init());
    memset(&dev, 0, sizeof(sht3x_t));

    ESP_ERROR_CHECK(sht3x_init_desc(&dev, 0x44, 0, 25, 26));
    ESP_ERROR_CHECK(sht3x_init(&dev));

    xSemaphore_sht = xSemaphoreCreateMutex();
    if (xSemaphore_sht != NULL)
    {
        // The semaphore was created successfully.
        // The semaphore can now be used.
    }

    xTaskCreatePinnedToCore(sht31_reader, "sh301x_test", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL, APP_CPU_NUM);
}

float get_sht_temperature()
{
    static float tmp = 0;
    if (xSemaphore_sht != NULL)
    {
        // See if we can obtain the semaphore.  If the semaphore is not available
        // wait 100 ms to see if it becomes free.
        if (xSemaphoreTake(xSemaphore_sht, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // We were able to obtain the semaphore and can now access the
            // shared resource.

            tmp = sht_temperature;

            // We have finished accessing the shared resource.  Release the
            // semaphore.
            xSemaphoreGive(xSemaphore_sht);
        }
    }
    return tmp;
}

float get_sht_humidity()
{
    static float tmp;
    if (xSemaphore_sht != NULL)
    {
        // See if we can obtain the semaphore.  If the semaphore is not available
        // wait 100 ms to see if it becomes free.
        if (xSemaphoreTake(xSemaphore_sht, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // We were able to obtain the semaphore and can now access the
            // shared resource.
            tmp = sht_humidity;
            // We have finished accessing the shared resource.  Release the
            // semaphore.
            xSemaphoreGive(xSemaphore_sht);
        }
    }
    return tmp;
}

void sht31_reader(void *pvParameters)
{

    // get the measurement duration for high repeatability;
    uint8_t duration = sht3x_get_measurement_duration(SHT3X_HIGH);

    while (1)
    {
        // Trigger one measurement in single shot mode with high repeatability.
        ESP_ERROR_CHECK(sht3x_start_measurement(&dev, SHT3X_SINGLE_SHOT, SHT3X_HIGH));
        // Wait until measurement is ready (constant time of at least 30 ms
        // or the duration returned from *sht3x_get_measurement_duration*).
        vTaskDelay(duration);

        if (xSemaphore_sht != NULL)
        {
            if (xSemaphoreTake(xSemaphore_sht, portMAX_DELAY) == pdTRUE)
            {
                // retrieve the values and do something with them
                ESP_ERROR_CHECK(sht3x_get_results(&dev, &sht_temperature, &sht_humidity));
                // printf("SHT3x Sensor: %.2f °C, %.2f %%\n", sht_temperature, sht_humidity);
                xSemaphoreGive(xSemaphore_sht);
            }
        }

        // wait until 2 seconds are over
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
