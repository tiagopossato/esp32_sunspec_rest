#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include "esp_log.h"
#include "cJSON.h"

#include "app_sunspec_models.h"
#include "bmp280_reader.h"
#include "sht31_reader.h"

#include "sdkconfig.h"

static const char *TAG = "main";

SunSpec sunspec;

model *model_1;
model *model_307;
void app_main(void)
{
    // inicializa os modelos
    model_1 = init_model_1();
    model_307 = init_model_307();
    model_1->next = model_307;

    sunspec.first = model_1;

    // imprime os modelos
    print_model(model_1);
    print_model(model_307);
    // cJSON *root;
	// root = cJSON_CreateObject();
    // point_to_cjson(root, model_1->group->points);

    // char *my_json_string = cJSON_Print(root);
	// ESP_LOGI(TAG, "my_json_string\n%s",my_json_string);

    // cJSON_Minify(my_json_string);
	// ESP_LOGI(TAG, "my_json_string\n%s",my_json_string);

	// cJSON_Delete(root);


    // inicializa os sensores
    bmp280_begin();
    sht31_begin();

    // imprime o modelo 307 a cada 2 segundos
    while (1)
    {
        print_model(model_307);
        // ESP_LOGI(TAG, "Temperature SHT: %.2f C", get_sht_temperature());
        // ESP_LOGI(TAG, "Temperature BMP: %.2f C", get_bmp_temperature());
        // ESP_LOGI(TAG, "Humidity SHT: %.2f %%", get_sht_humidity());
        // ESP_LOGI(TAG, "Pressure BMP: %.2f hPa\n", get_bmp_pressure());
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
