#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include "esp_log.h"
#include "cJSON.h"

#include "app_sunspec_models.h"
#include "bmp280_reader.h"
#include "sht31_reader.h"
#include "app_wifi.h"

static const char *TAG = "main";

SunSpec sunspec;

model *model_1;
model *model_307;
void app_main(void)
{
    app_wifi_init_softap();

    cJSON *root;
    char *my_json_string;

    // inicializa os sensores
    bmp280_begin();
    sht31_begin();
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // inicializa os modelos
    model_1 = init_model_1();
    model_307 = init_model_307();

    model_1->next = model_307;
    sunspec.first = model_1;

    root = cJSON_CreateObject();

    sunspec_to_cjson(root, &sunspec, true);
    my_json_string = cJSON_Print(root);
    ESP_LOGI(TAG, "\n%s", my_json_string);
    ESP_LOGI(TAG, "Tamanho do JSON: %d", strlen(my_json_string));
    cJSON_Minify(my_json_string);
    ESP_LOGI(TAG, "Tamanho do JSON minimizado: %d", strlen(my_json_string));

    cJSON_Delete(root);
    free(my_json_string);
    // while (1)
    // {
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }

    // imprime o temperatura, umidade e pressao do modelo 307 a cada 5 segundos
    char *point_name_list[] = {"TmpAmb", "RH", "Pres", "JKJK", NULL};

    while (1)
    {

        root = cJSON_CreateObject();
        get_model_cjson_points_by_name(root, &sunspec, 307, point_name_list);
        my_json_string = cJSON_Print(root);
        cJSON_Minify(my_json_string);
        printf("%s\n", my_json_string);
        cJSON_Delete(root);
        free(my_json_string);
        // ESP_LOGI(TAG, "Temperature SHT: %.2f C", get_sht_temperature());
        // ESP_LOGI(TAG, "Temperature BMP: %.2f C", get_bmp_temperature());
        // ESP_LOGI(TAG, "Humidity SHT: %.2f %%", get_sht_humidity());
        // ESP_LOGI(TAG, "Pressure BMP: %.2f hPa\n", get_bmp_pressure());
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
