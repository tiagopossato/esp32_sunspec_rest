#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include "esp_log.h"
#include "cJSON.h"

#include "app_sunspec_models.h"
#include "app_https_server.h"

static const char *TAG = "main";

SunSpec *suns;
model *model_1;
model *model_65500;

void app_main(void)
{

    suns = init_sunspec();
    // inicializa os modelos
    model_1 = init_model_1();
    model_65500 = init_model_65500();
    // adiciona os modelos a lista de modelos
    add_model(suns, model_1);
    add_model(suns, model_65500);

    // inicializa os sensores


    app_https_server_start();



    // teste de escrita

    // cJSON *root;
    // char *my_json_string;
    // char *point_name_list;

    // point *test_point = model_65500->group->points->next->next; //compressor
    // root = cJSON_CreateObject();
    // if (false == test_point->set_value("1", root))
    // {
    //     my_json_string = cJSON_Print(root);
    //     cJSON_Minify(my_json_string);
    //     printf("%s\n", my_json_string);
    // }
    // else
    // {
    //     printf("valor alterado com sucesso\n");
    //     asprintf(&point_name_list, "Comp,SetPt,Hist,OpMode");
    //     get_model_cjson_points_by_name(root, suns, 65500, point_name_list);
    //     my_json_string = cJSON_Print(root);
    //     cJSON_Minify(my_json_string);
    //     printf("%s\n", my_json_string);
    //     free(point_name_list);
    // }

    // test_point = model_65500->group->points->next->next->next->next->next; // modo de operação
    // root = cJSON_CreateObject();
    // if (false == test_point->set_value("1", root))
    // {
    //     my_json_string = cJSON_Print(root);
    //     cJSON_Minify(my_json_string);
    //     printf("%s\n", my_json_string);
    // }
    // else
    // {
    //     printf("valor alterado com sucesso\n");
    //     asprintf(&point_name_list, "Comp,SetPt,Hist,OpMode");
    //     get_model_cjson_points_by_name(root, suns, 65500, point_name_list);
    //     my_json_string = cJSON_Print(root);
    //     cJSON_Minify(my_json_string);
    //     printf("%s\n", my_json_string);
    //     free(point_name_list);
    // }

    // test_point = model_65500->group->points->next->next; //compressor
    // root = cJSON_CreateObject();
    // if (false == test_point->set_value("0", root))
    // {
    //     my_json_string = cJSON_Print(root);
    //     cJSON_Minify(my_json_string);
    //     printf("%s\n", my_json_string);
    // }
    // else
    // {
    //     printf("valor alterado com sucesso\n");
    //     asprintf(&point_name_list, "Comp,SetPt,Hist,OpMode");
    //     get_model_cjson_points_by_name(root, suns, 65500, point_name_list);
    //     my_json_string = cJSON_Print(root);
    //     cJSON_Minify(my_json_string);
    //     printf("%s\n", my_json_string);
    //     free(point_name_list);
    // }

    // test_point = model_65500->group->points->next->next->next->next->next; // modo de operação
    // root = cJSON_CreateObject();
    // if (false == test_point->set_value("0", root))
    // {
    //     my_json_string = cJSON_Print(root);
    //     cJSON_Minify(my_json_string);
    //     printf("%s\n", my_json_string);
    // }
    // else
    // {
    //     printf("valor alterado com sucesso\n");
    //     asprintf(&point_name_list, "Comp,SetPt,Hist,OpMode");
    //     get_model_cjson_points_by_name(root, suns, 65500, point_name_list);
    //     my_json_string = cJSON_Print(root);
    //     cJSON_Minify(my_json_string);
    //     printf("%s\n", my_json_string);
    //     free(point_name_list);
    // }

    // test_point = model_65500->group->points->next->next; //compressor
    // root = cJSON_CreateObject();
    // if (false == test_point->set_value("0", root))
    // {
    //     my_json_string = cJSON_Print(root);
    //     cJSON_Minify(my_json_string);
    //     printf("%s\n", my_json_string);
    // }
    // else
    // {
    //     printf("valor alterado com sucesso\n");
    //     asprintf(&point_name_list, "Comp,SetPt,Hist,OpMode");
    //     get_model_cjson_points_by_name(root, suns, 65500, point_name_list);
    //     my_json_string = cJSON_Print(root);
    //     cJSON_Minify(my_json_string);
    //     printf("%s\n", my_json_string);
    //     free(point_name_list);
    // }

    // cJSON_Delete(root);
    // free(my_json_string);

    // while (1)
    // {
    //     vTaskDelay(2000 / portTICK_PERIOD_MS);
    // }

    while (1)
    {
        ESP_LOGI(TAG, "Free heap: %ld", esp_get_free_heap_size());
        ESP_LOGI(TAG, "Minimum free heap: %ld", esp_get_minimum_free_heap_size());
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
