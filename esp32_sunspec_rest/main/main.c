#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include "esp_log.h"

#include "app_sunspec_models.h"
#include "bmp280_reader.h"
#include "sht31_reader.h"

#include "sdkconfig.h"

static const char *TAG = "main";

model *model_1;
model *model_307;
void app_main(void)
{
    model_1 = init_model_1();
    if (model_1 != NULL)
    {
        print_model(model_1);
    }
    else
    {
        printf("Modelo 1 iniciado com sucesso\n");
    }

    model_307 = init_model_307();
    if (model_307 != NULL)
    {
        print_model(model_307);
    }
    else
    {
        printf("Modelo 307 iniciado com sucesso\n");
    }

    bmp280_begin();
    sht31_begin();
    while (1)
    {
        print_model(model_307);
        // ESP_LOGI(TAG, "Temperature SHT: %.2f C", get_sht_temperature());
        // ESP_LOGI(TAG, "Temperature BMP: %.2f C", get_bmp_temperature());
        // ESP_LOGI(TAG, "Humidity SHT: %.2f %%", get_sht_humidity());
        // ESP_LOGI(TAG, "Pressure BMP: %.2f hPa\n", get_bmp_pressure());
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
