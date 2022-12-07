#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/semphr.h"

#include <esp_system.h>
#include <string.h>
#include <esp_err.h>
#include <esp_log.h>
#include <sht3x.h>

#include "sht31_reader.h"

// #define SHT31_ERROR_CHECK_WITHOUT_ABORT

void sht31_reader(void *pvParameters);

float sht_temperature, sht_humidity;

SemaphoreHandle_t xSemaphore_sht = NULL;

void sht31_begin()
{
    ESP_ERROR_CHECK(i2cdev_init());

    xSemaphore_sht = xSemaphoreCreateMutex();
    if (xSemaphore_sht != NULL)
    {
        xTaskCreatePinnedToCore(sht31_reader, "sh301x_test", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL, APP_CPU_NUM);
    }
}

float get_sht_temperature()
{
    static float tmp = -127;
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
    static float tmp = -127;
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
    sht3x_t dev;
    memset(&dev, 0, sizeof(sht3x_t));
    esp_err_t res;
    uint8_t tries = 0;

    do
    {
        res = sht3x_init_desc(&dev, 0x44, 0, 25, 26);

        if (res != ESP_OK)
        {
            if (tries++ > 3)
            {
                ESP_LOGE("SHT31", "failed to init descriptor");
                ESP_ERROR_CHECK_WITHOUT_ABORT(res);
                sht_temperature = -12.7;
                sht_humidity = -127;
                vTaskDelete(NULL);
            }
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    } while (res != ESP_OK);

    tries = 0;
    do
    {
        res = sht3x_init(&dev);
        if (res != ESP_OK)
        {
            if (tries++ > 3)
            {
                ESP_LOGE("SHT31", "failed to init device");
                ESP_ERROR_CHECK_WITHOUT_ABORT(res);
                sht_temperature = -12.7;
                sht_humidity = -127;
                vTaskDelete(NULL);
            }
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    } while (res != ESP_OK);

    // get the measurement duration for high repeatability;
    uint8_t duration = sht3x_get_measurement_duration(SHT3X_HIGH);

    while (1)
    {
        tries = 0;
        do
        {
            // Trigger one measurement in single shot mode with high repeatability.
            res = sht3x_start_measurement(&dev, SHT3X_SINGLE_SHOT, SHT3X_HIGH);
            if (res != ESP_OK)
            {
                if (tries++ > 3)
                {
                    ESP_LOGE("SHT31", "failed to start measurement");
                    ESP_ERROR_CHECK_WITHOUT_ABORT(res);
                    sht_temperature = -12.7;
                    sht_humidity = -127;
                    vTaskDelete(NULL);
                }
                vTaskDelay(pdMS_TO_TICKS(2000));
            }
        } while (res != ESP_OK);

        // Wait until measurement is ready (constant time of at least 30 ms
        // or the duration returned from *sht3x_get_measurement_duration*).
        vTaskDelay(duration);

        tries = 0;
        do
        {
            res = ESP_OK;
            if (xSemaphoreTake(xSemaphore_sht, portMAX_DELAY) == pdTRUE)
            {
                // retrieve the values
                res = sht3x_get_results(&dev, &sht_temperature, &sht_humidity);
                xSemaphoreGive(xSemaphore_sht);
            }
            if (res != ESP_OK)
            {
                if (tries++ > 3)
                {
                    ESP_LOGE("SHT31", "failed to read values");
                    ESP_ERROR_CHECK_WITHOUT_ABORT(res);
                    sht_temperature = -12.7;
                    sht_humidity = -127;
                    vTaskDelete(NULL);
                }
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        } while (res != ESP_OK);
        // wait until 2 seconds are over
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}