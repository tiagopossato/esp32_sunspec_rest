#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_log.h>
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
        xTaskCreatePinnedToCore(bmp280_reader, "bmp280_test", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL, APP_CPU_NUM);
    }
}

float get_bmp_temperature()
{
    static float tmp = -127;
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
    static float tmp = -127;
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
    esp_err_t res;
    bmp280_t dev;
    memset(&dev, 0, sizeof(bmp280_t));
    uint8_t tries = 0;
    do
    {
        res = bmp280_init_desc(&dev, 0x76, 0, 22, 23);
        if (res != ESP_OK)
        {
            if (tries++ > 3)
            {
                ESP_LOGE("BMP280", "failed to init descriptor");
                ESP_ERROR_CHECK_WITHOUT_ABORT(res);
                bmp_pressure = -12700;
                bmp_temperature = -12.7;
                bmp_humidity = -127;
                vTaskDelete(NULL);
            }
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    } while (res != ESP_OK);

    tries = 0;
    do
    {
        res = bmp280_init(&dev, &params);
        if (res != ESP_OK)
        {
            if (tries++ > 3)
            {
                ESP_LOGE("BMP280", "failed to init device");
                ESP_ERROR_CHECK_WITHOUT_ABORT(res);
                bmp_pressure = -12700;
                bmp_temperature = -12.7;
                bmp_humidity = -127;
                vTaskDelete(NULL);
            }
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    } while (res != ESP_OK);

    bool bme280p = dev.id == BME280_CHIP_ID;
    printf("BMP280: found %s\n", bme280p ? "BME280" : "BMP280");

    while (1)
    {
        tries = 0;
        do
        {
            res = ESP_OK;

            if (xSemaphoreTake(xSemaphore_bmp, portMAX_DELAY) == pdTRUE)
            {
                // retrieve the values
                res = bmp280_read_float(&dev, &bmp_temperature, &bmp_pressure, &bmp_humidity);
                xSemaphoreGive(xSemaphore_bmp);
            }

            if (res != ESP_OK)
            {
                if (tries++ > 3)
                {
                    ESP_LOGE("BMP280", "failed to read values");
                    ESP_ERROR_CHECK_WITHOUT_ABORT(res);
                    bmp_pressure = -12700;
                    bmp_temperature = -12.7;
                    bmp_humidity = -127;
                    vTaskDelete(NULL);
                }

                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        } while (res != ESP_OK);

        // printf("Pressure: %.2f Pa, Temperature: %.2f C\n", bmp_pressure, bmp_temperature);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
