#include <esp_log.h>
#include "esp_wifi.h"
#include <esp_system.h>

#include "cJSON.h"

#include "esp_tls.h"
#include "sdkconfig.h"

#include "app_https_server.h"
#include "app_sunspec_models.h"

#ifdef HTTPS_SERVER_ENABLE // definido em app_https_server.h
#include <esp_https_server.h>
#else
#include <esp_http_server.h>
#endif

#define HTTPD_405 "405 Method Not Allowed" /*!< HTTP Response 405 */

static const char *TAG = "patch_router";

static SunSpec *suns;

// funções criadas em outros arquivos .c deste componente
extern esp_err_t basic_auth_handler(httpd_req_t *req);
extern void http_send_error(httpd_req_t *req, const char *http_status,
                            char *errCode, char *errMessage, char *errReason,
                            bool debug, char *TBD);

static bool httpd_uri_match_simple(const char *uri1, const char *uri2, size_t len2)
{
    return strlen(uri1) == len2 &&           // First match lengths
           (strncmp(uri1, uri2, len2) == 0); // Then match actual URIs
}

static esp_err_t patch_model(httpd_req_t *req, uint16_t model_id)
{
    // get http body
    // verifica o tamanho do conteúdo da requisição
    size_t recv_size = req->content_len;
    // cria um buffer para armazenar o conteúdo da requisição
    char *content = calloc(1, recv_size + 1);
    // verifica se o buffer foi criado com sucesso
    if (content == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for content");
        http_send_error(req, HTTPD_500, "PATCH-ERR01",
                        "No  enough memory", "No enough memory for PATCH content",
                        false, NULL);

        return ESP_OK;
    }
    // lê o conteúdo da requisição
    int ret = httpd_req_recv(req, content, recv_size);
    // verifica se a leitura foi bem sucedida
    if (ret <= 0)
    { /* 0 return value indicates connection closed */
        ESP_LOGE(TAG, "Failed to read content");
        http_send_error(req, HTTPD_500, "PATCH-ERR02",
                        "Failed to read content", "Failed to read content",
                        false, NULL);
        free(content);
        return ESP_OK;
    }

    // print the content
    ESP_LOGI(TAG, "Content: %s", content);

    // transforma o texto recebido em um objeto JSON
    cJSON *json_content = cJSON_Parse(content);
    // validar
    if (cJSON_IsObject(json_content) == false)
    {
        http_send_error(req, HTTPD_500, "PATCH-ERR03",
                        "Failed to read json_content content", "Failed to read json_content content",
                        false, NULL);
        free(content);
        return ESP_OK;
    }

    // pega o vetor de pontos
    cJSON *array_points = cJSON_GetObjectItem(json_content, "points");
    if (cJSON_IsArray(array_points) == false)
    {
        http_send_error(req, HTTPD_500, "PATCH-ERR04",
                        "Failed to read points content", "Failed to read points content",
                        false, NULL);
        free(content);
        cJSON_Delete(json_content);
        return ESP_OK;
    }

    cJSON *error_response = cJSON_CreateObject();
    suns = get_sunspec();
    if (false == patch_points(array_points, suns, model_id, error_response))
    {
        // print error
        char *my_json_string = cJSON_Print(error_response);
        // printf("error_response: %s\n", my_json_string);

        httpd_resp_set_hdr(req, "Connection", "close");
        httpd_resp_set_type(req, "application/json; charset=utf-8");
        httpd_resp_set_status(req, HTTPD_405);

        httpd_resp_send(req, my_json_string, HTTPD_RESP_USE_STRLEN);

        cJSON_Delete(array_points);
        cJSON_Delete(error_response);
        free(my_json_string);
        free(content);
        return ESP_OK;
    }

    cJSON_Delete(error_response);
    cJSON_Delete(json_content);

    // set connection close
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "application/json; charset=utf-8");
    httpd_resp_send(req, content, HTTPD_RESP_USE_STRLEN);
    free(content);
    return ESP_OK;
}

esp_err_t patch_router(httpd_req_t *req)
{
    if (basic_auth_handler(req) != ESP_OK)
    {
        // Caso a autenticação falhar, a requisição será preenchida com o código de erro
        //  esta função deve retornar OK para que o servidor envie a resposta de erro
        return ESP_OK;
    }
    // Caso a autenticação seja bem sucedida, a requisição prossegue normalmente

    // TODO: Tornar as linhas abaixo dinâmicas
    if (httpd_uri_match_simple("/v1/models/1/instances/0", req->uri, strlen(req->uri)))
    {
        return patch_model(req, 1);
    }

    http_send_error(req, HTTPD_404, "PATCH-ERR05",
                    "URI not match", "URI not match",
                    false, NULL);

    return ESP_OK;
}
