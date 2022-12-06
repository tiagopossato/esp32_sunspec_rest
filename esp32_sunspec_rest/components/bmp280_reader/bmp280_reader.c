#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include "bmp280.h"
#include <string.h>
#include "bmp280_reader.h"

void bmp280_reader(void *pvParameters);
SemaphoreHandle_t xSemaphore_bmp = NULL;
float bmp_pressure, bmp_temperature, bmp_humidity;

void bmp280_begin()
{
    ESP_ERROR_CHECK(i2cdev_init());
    xSemaphore_bmp = xSemaphoreCreateMutex();
    if (xSemaphore_bmp != NULL)
    {
        // The semaphore was created successfully.
        // The semaphore can now be used.
    }
    xTaskCreatePinnedToCore(bmp280_reader, "bmp280_test", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL, APP_CPU_NUM);
}

float get_bmp_temperature()
{
    static float tmp;
    if (xSemaphore_bmp != NULL)
    {
        // See if we can obtain the semaphore.  If the semaphore is not available
        // wait 100 ms to see if it becomes free.
        if (xSemaphoreTake(xSemaphore_bmp, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // We were able to obtain the semaphore and can now access the
            // shared resource.

            tmp = bmp_temperature;

            // We have finished accessing the shared resource.  Release the
            // semaphore.
            xSemaphoreGive(xSemaphore_bmp);
        }
    }
    return tmp;
}

float get_bmp_pressure()
{
    static float tmp;
    if (xSemaphore_bmp != NULL)
    {
        // See if we can obtain the semaphore.  If the semaphore is not available
        // wait 100 ms to see if it becomes free.
        if (xSemaphoreTake(xSemaphore_bmp, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // We were able to obtain the semaphore and can now access the
            // shared resource.
            tmp = bmp_pressure;
            // We have finished accessing the shared resource.  Release the
            // semaphore.
            xSemaphoreGive(xSemaphore_bmp);
        }
    }
    return tmp;
}

void bmp280_reader(void *pvParameters)
{
    bmp280_params_t params;
    bmp280_init_default_params(&params);
    bmp280_t dev;
    memset(&dev, 0, sizeof(bmp280_t));

    ESP_ERROR_CHECK(bmp280_init_desc(&dev, 0x76, 0, 22, 23));
    ESP_ERROR_CHECK(bmp280_init(&dev, &params));

    bool bme280p = dev.id == BME280_CHIP_ID;
    printf("BMP280: found %s\n", bme280p ? "BME280" : "BMP280");

    while (1)
    {

        if (xSemaphore_bmp != NULL)
        {
            if (xSemaphoreTake(xSemaphore_bmp, portMAX_DELAY) == pdTRUE)
            {
                if (bmp280_read_float(&dev, &bmp_temperature, &bmp_pressure, &bmp_humidity) != ESP_OK)
                {
                    xSemaphoreGive(xSemaphore_bmp);
                    printf("Temperature/bmp_pressure reading failed\n");
                    continue;
                }
                xSemaphoreGive(xSemaphore_bmp);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
        // printf("Pressure: %.2f Pa, Temperature: %.2f C", bmp_pressure, bmp_temperature);
        // if (bme280p)
        //     printf(", Humidity: %.2f\n", bmp_humidity);
        // else
        //     printf("\n");
    }
}
