#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "esp_log.h"

#include "sunspec_models.h"

static const char *TAG = "sunspec_patch";


/**
 * @brief Valida array de pontos para atualizar um modelo via PATCH
 * @param points cJSON array object com todos os pontos atualizáveis do modelo
 * @param suns SunSpec
 * @param model_id id do modelo
 * @return true se todos os pontos são válidos
 * @return false se algum ponto não é válido ou,
 *  se o modelo não existe ou,
 *  se tem algum ponto não atualizável ou,
 *  se o modelo não tem pontos atualizáveis ou,
 *  se algum valor a ser atualizado em algum ponto não é válido
 */
bool validate_patch_points(cJSON *array_patch_points, SunSpec *suns, uint16_t model_id, cJSON *error_response)
{
    if (array_patch_points == NULL)
    {
        ESP_LOGE(TAG, "cJSON *points is NULL");
        return false;
    }
    if (error_response == NULL)
    {
        ESP_LOGE(TAG, "cJSON *error_response is NULL");
        return false;
    }

    model *model_to_patch = suns->first;
    if (model_to_patch == NULL)
    {
        ESP_LOGE(TAG, "first suns is NULL");
        return false;
    }

    // busca o modelo pelo model_id
    while (model_to_patch != NULL)
    {
        if (model_to_patch->id == model_id)
        {
            break;
        }
        model_to_patch = model_to_patch->next;
    }
    // verifica se encontrou um modelo
    if (model_to_patch == NULL)
    {
        ESP_LOGE(TAG, "model_id %d not found", model_id);
        return false;
    }

    // pega o primeiro ponto do modelo
    point *point_to_patch = model_to_patch->group->points;
    if (point_to_patch == NULL)
    {
        ESP_LOGE(TAG, "model_id %d has no points", model_id);
        return false;
    }
    // monta uma lista com todos os pontos atualizáveis do modelo
    cJSON *patchable_points = cJSON_CreateArray();
    while (point_to_patch != NULL)
    {
        // verifica se o ponto possui função de atualização
        // isso implica no ponto ser do tipo pt_RW
        if (point_to_patch->set_value != NULL)
        {
            cJSON *i = cJSON_CreateObject();
            cJSON_AddObjectToObject(i, point_to_patch->name);
            cJSON_AddItemToArray(patchable_points, i);
        }
        point_to_patch = point_to_patch->next;
    }

    // imprime a lista de pontos atualizáveis do modelo
    char *patchable_points_string = cJSON_Print(patchable_points);
    cJSON_Minify(patchable_points_string);
    ESP_LOGI(TAG, "patchable_points: %s", patchable_points_string);

    int input_array_size = cJSON_GetArraySize(array_patch_points);
    int patchable_points_size = cJSON_GetArraySize(patchable_points);

    ESP_LOGI(TAG, "input_array_size: %d, patchable_points_size: %d",
             input_array_size, patchable_points_size);

    /****-----PRIMEIRA VERIFICAÇÃO: QUANTIDADE DE PONTOS-----****/

    // verifica se a quantidade de pontos recebida é igual a quantidade de pontos editáveis
    if (input_array_size != patchable_points_size)
    {
        char *errReason;
        asprintf(&errReason, "A quantidade de pontos informada não é compatível com o modelo. Os seguintes pontos podem ser atualizados: %s", patchable_points_string);
        new_error(error_response, "PATCH-VALIDATE-01",
                  "Invalid points", errReason,
                  false, NULL);
        free(errReason);
        free(patchable_points_string);
        cJSON_Delete(patchable_points);
        return false;
    }

    /****-----SEGUNDA VERIFICAÇÃO: COMPARA OS PONTOS EDITÁVEIS DO MODELO COM OS PONTOS RECEBIDOS-----****/

    // verifica se os pontos recebidos são os mesmos que os pontos editáveis
    // e valida os valores enviados
    for (uint16_t i = 0; i < patchable_points_size; i++)
    {
        cJSON *patchable_point = cJSON_GetArrayItem(patchable_points, i);

        ESP_LOGI(TAG, "buscando input_point: %s", patchable_point->child->string);
        cJSON *input_point = NULL;

        // itera sobre os pontos recebidos para verificar se o ponto atual é um dos pontos recebidos
        for (uint16_t j = 0; j < patchable_points_size; j++)
        {
            // pega o ponto j.
            // Vai retornar um objeto com o nome do ponto e o valor. Ex: {"DA": 1}
            cJSON *ip = cJSON_GetArrayItem(array_patch_points, j);
            // dentro de ip, pega o valor do ponto que está sendo verificado
            input_point = cJSON_GetObjectItemCaseSensitive(ip, patchable_point->child->string);
            // verifica se o ponto atual foi encontrado
            if (input_point != NULL)
            {
                break;
            }
        }

        /****-----VERIFICAÇÃO: -----****/
        // se o ponto não foi encontrado
        if (input_point == NULL)
        {
            char *errReason;
            asprintf(&errReason,
                     "O ponto %s não foi encontrado. Os seguintes pontos precisam ser enviados, mesmo que seja como um objeto vazio {}: %s",
                     patchable_point->child->string, patchable_points_string);
            new_error(error_response, "PATCH-VALIDATE-02",
                      "Invalid points", errReason,
                      false, NULL);
            free(errReason);
            free(patchable_points_string);
            cJSON_Delete(patchable_points);
            return false;
        }
        // o ponto foi encontrado, então verifica se o valor é válido

        /****-----TERCEIRA VERIFICAÇÃO: VALIDA O VALOR RECEBIDO-----****/
        // pega o primeiro ponto do modelo para verificar se o valor é válido
        point_to_patch = model_to_patch->group->points;
        // Percorre todos os pontos do modelo, até encontrar o ponto que será validado
        while (point_to_patch != NULL)
        {
            // compara o nome do ponto atual com o nome do ponto a ser validado
            if (strcmp(point_to_patch->name, patchable_point->child->string) == 0)
            {
                // Nesse momento, o ponto que se pretende validar foi encontrado no modelo
                // verifica se o valor recebido é um objeto vazio, o que significa que não precisa validar
                if (cJSON_IsObject(input_point))
                {
                    ESP_LOGI(TAG, "input_point: %s está vazio e não necessita validação",
                             patchable_point->child->string);
                    // para a busca pois já encontrou, apesar de não necessitar de validação
                    break;
                }
                else
                {
                    // o valor não é um objeto vazio, então precisa validar
                    // ponteiro para receber o valor do ponto
                    char *pt_value;
                    // verifica se o valor é uma string
                    if (cJSON_IsString(input_point))
                    {
                        pt_value = cJSON_GetStringValue(input_point);
                    }
                    else
                    {
                        // se não for uma string, então é um número
                        // por causa da biblioteca cJSON, é necessário converter o número para string
                        pt_value = cJSON_Print(input_point);
                    }
                    ESP_LOGI(TAG, "input_point pt_value: %s", pt_value);
                    // chama a função de validação do ponto
                    if (point_to_patch->validate_set_value(pt_value, error_response) == false)
                    {
                        // se a função retornar false, então o valor é inválido
                        // e a mensagem de erro já foi preenchida em error_response
                        // liberar a memória alocada para o valor do ponto
                        if (cJSON_IsString(input_point))
                        {
                            // se for string só apontar para NULL
                            // pois o ponteiro pt_value aponta para
                            // o mesmo endereço de memória do JSON.
                            // Se executar free, vai dar erro
                            pt_value = NULL;
                        }
                        else
                        {
                            free(pt_value);
                        }
                        free(patchable_points_string);
                        cJSON_Delete(patchable_points);
                        return false;
                    }
                    if (cJSON_IsString(input_point))
                    {
                        pt_value = NULL;
                    }
                    else
                    {
                        free(pt_value);
                    }
                    // para a busca pois já encontrou
                    break;
                }
            }
            point_to_patch = point_to_patch->next;
        }
        // se o ponto não foi encontrado no modelo
        if (point_to_patch == NULL)
        {
            char *errReason;
            asprintf(&errReason,
                     "O ponto %s não foi encontrado. Os seguintes pontos precisam ser enviados, mesmo que seja como um objeto vazio {}: %s",
                     patchable_point->child->string, patchable_points_string);
            new_error(error_response, "PATCH-VALIDATE-03",
                      "Invalid points", errReason,
                      false, NULL);
            free(errReason);
            free(patchable_points_string);
            cJSON_Delete(patchable_points);
            return false;
        }
    }
    // O PATCH foi validado com sucesso
    ESP_LOGI(TAG, "PATCH validado com sucesso");
    free(patchable_points_string);
    cJSON_Delete(patchable_points);

    return true;
}

bool patch_points(cJSON *array_patch_points, SunSpec *suns, uint16_t model_id, cJSON *error_response)
{
    // validação básica dos parâmetros
    if (array_patch_points == NULL)
    {
        ESP_LOGE(TAG, "cJSON *array_patch_points is NULL");
        return false;
    }
    if (error_response == NULL)
    {
        ESP_LOGE(TAG, "cJSON *error_response is NULL");
        return false;
    }

    if (suns == NULL)
    {
        ESP_LOGE(TAG, "suns is NULL");
        return false;
    }

    // valida o PATCH conforme as regras da especificação do SunSpec
    if (validate_patch_points(array_patch_points, suns, model_id, error_response) == false)
    {
        return false;
    }

    // pega o primeiro modelo
    model *model_to_patch = suns->first;
    if (model_to_patch == NULL)
    {
        ESP_LOGE(TAG, "first suns is NULL");
        return false;
    }

    // busca o modelo pelo model_id
    while (model_to_patch != NULL)
    {
        if (model_to_patch->id == model_id)
        {
            break;
        }
        model_to_patch = model_to_patch->next;
    }
    // verifica se encontrou um modelo
    if (model_to_patch == NULL)
    {
        ESP_LOGE(TAG, "model_id %d not found", model_id);
        return false;
    }

    // iterate over the points to patch and call the set_value function
    // remember that values are validated
    for (uint16_t i = 0; i < cJSON_GetArraySize(array_patch_points); i++)
    {
        // take first point from points
        cJSON *json_input_point = cJSON_GetArrayItem(array_patch_points, i);

        // take first point from model_to_patch
        point *point_to_patch = model_to_patch->group->points;
        // iterate over the points of the model
        while (point_to_patch != NULL)
        {
            // compare the name of the current point with the name of the point to be patched
            if (strcmp(json_input_point->child->string, point_to_patch->name) == 0)
            {
                cJSON *input_point = cJSON_GetObjectItemCaseSensitive(json_input_point, point_to_patch->name);
                // verify if the point is a object
                if (cJSON_IsObject(input_point))
                {
                    ESP_LOGI(TAG, "input_point pt_value: %s está vazio e não será alterado",
                             point_to_patch->name);
                    // para a busca pois já encontrou, apesar de não necessitar alterar o valor
                    break;
                }
                else
                {
                    char *pt_value;

                    if (cJSON_IsString(input_point))
                    {
                        pt_value = cJSON_GetStringValue(input_point);
                    }
                    else
                    {
                        pt_value = cJSON_Print(input_point);
                    }
                    ESP_LOGI(TAG, "input_point pt_value: %s", pt_value);

                    // call the set_value function
                    point_to_patch->set_value(pt_value, error_response);

                    // libera a memória utilizada para armazenar o valor.
                    // se for string, a biblioteca cJSON não aloca memória,
                    // só aponta para a localização dentro do JSON,
                    // se liberar aqui, vai quebrar a aplicação quando chamar cJSON_Delete
                    if (cJSON_IsString(input_point))
                    {
                        pt_value = NULL;
                    }
                    else
                    {
                        free(pt_value);
                    }
                    // stop the search because it already found
                    break;
                }
            }
            point_to_patch = point_to_patch->next;
        }
    }
    return true;
}