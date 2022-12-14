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
#include "app_https_server.h"

static const char *TAG = "main";

SunSpec *suns;
model *model_1;
model *model_307;

void app_main(void)
{

    suns = init_sunspec();
    // inicializa os modelos
    model_1 = init_model_1();
    model_307 = init_model_307();
    // adiciona os modelos a lista de modelos
    add_model(suns, model_1);
    add_model(suns, model_307);

    // inicializa os sensores
    // bmp280_begin();
    // sht31_begin();

    app_https_server_start();

    // cJSON *root;

    // root = cJSON_CreateObject();
    // cJSON_AddNumberToObject(root, "id", 1);

    // cJSON *points = cJSON_AddArrayToObject(root, "points");
    // cJSON *point = cJSON_CreateObject();
    // cJSON_AddNumberToObject(point, "DA", 1);
    // cJSON_AddItemToArray(points, point);

    // point = cJSON_CreateObject();
    // cJSON_AddStringToObject(point, "SN", "2002-sdf");
    // cJSON_AddItemToArray(points, point);

    // char *my_json_string = cJSON_Print(root);
    // printf("%s\n", my_json_string);

    // cJSON *array_points = cJSON_GetObjectItem(root, "points");
    // //my_json_string = cJSON_Print(array_points);
    // //printf("array_points:n%s\n", my_json_string);

    // cJSON *error_response = cJSON_CreateObject();
    // if (false == validate_and_patch_points(array_points, suns, 1, error_response))
    // {
    //     // print error
    //     my_json_string = cJSON_Print(error_response);
    //     printf("error_response: %s\n", my_json_string);
    // }

    // cJSON_Delete(root);
    
    // free(my_json_string);

    while (1)
    {
        ESP_LOGI(TAG, "Free heap: %ld", esp_get_free_heap_size());
        ESP_LOGI(TAG, "Minimum free heap: %ld", esp_get_minimum_free_heap_size());
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    // imprime o temperatura, umidade e pressao do modelo 307 a cada 5 segundos
    // char *point_name_list;
    // cJSON *root;
    // char *my_json_string;
    // while (1)
    // {
    //     asprintf(&point_name_list, "TmpAmb,RH,Pres,JKJK");
    //     root = cJSON_CreateObject();
    //     get_model_cjson_points_by_name(root, suns, 307, point_name_list);
    //     my_json_string = cJSON_Print(root);
    //     cJSON_Minify(my_json_string);
    //     printf("%s\n", my_json_string);
    //     cJSON_Delete(root);
    //     free(my_json_string);
    //     vTaskDelay(5000 / portTICK_PERIOD_MS);
    // }
}
