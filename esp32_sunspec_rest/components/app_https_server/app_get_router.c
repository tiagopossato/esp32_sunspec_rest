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

static const char *TAG = "get_router";

static SunSpec *suns;

// funções criadas em outros arquivos .c deste componente
extern esp_err_t basic_auth_handler(httpd_req_t *req);
extern void http_send_error(httpd_req_t *req, const char *http_status,
                            char *errCode, char *errMessage, char *errReason,
                            bool debug, char *TBD);

/**
 * @brief Get model by id
 * @param req HTTP request
 * @param model_id Model id
 * @return ESP_OK on success
 *       ESP_FAIL on failure
 */
static esp_err_t get_model(httpd_req_t *req, uint16_t model_id)
{
    char *buf;
    size_t buf_len;
    char *csv_points;
    cJSON *root;
    char *my_json_string;

    // set connection close
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "application/json; charset=utf-8");

    // set refresh  header
    // TODO: Remove this in production
    httpd_resp_set_hdr(req, "Refresh", "5");

    suns = get_sunspec();

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
        {
            // ESP_LOGI(TAG, "Found URL query => %s", buf);
            csv_points = malloc(buf_len);
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "points", csv_points, buf_len) == ESP_OK)
            {
                root = cJSON_CreateObject();
                if (get_model_cjson_points_by_name(root, suns, model_id, csv_points))
                {
                    my_json_string = cJSON_Print(root);
                    // cJSON_Minify(my_json_string);
                    httpd_resp_send(req, my_json_string, HTTPD_RESP_USE_STRLEN);
                    cJSON_Delete(root);
                    free(my_json_string);
                    free(csv_points);
                    free(buf);
                    return ESP_OK;
                }
                else
                {
                    // Envia erro de modelo não encontrado
                    char *errReason;
                    asprintf(&errReason, "Model %u not found.", model_id);
                    http_send_error(req, HTTPD_404, "MODEL-ERR01",
                                    "Model not found.", errReason,
                                    false, NULL);
                    free(errReason);
                    cJSON_Delete(root);
                    free(csv_points);
                    free(buf);
                    return ESP_OK;
                }
            }
            free(csv_points);
        }
        free(buf);
    }

    // if no points are specified, return all points
    root = cJSON_CreateObject();
    if (get_model_cjson_by_id(root, suns, model_id))
    {
        my_json_string = cJSON_Print(root);
        // cJSON_Minify(my_json_string);
        httpd_resp_send(req, my_json_string, HTTPD_RESP_USE_STRLEN);
        free(my_json_string);
        cJSON_Delete(root);
        return ESP_OK;
    }

    // Envia erro de modelo não encontrado
    char *errReason;
    asprintf(&errReason, "Model %u not found.", model_id);
    http_send_error(req, HTTPD_404, "MODEL-ERR01",
                    "Model not found.", errReason,
                    false, NULL);
    free(errReason);
    cJSON_Delete(root);
    return ESP_OK;
}

/**
 * @brief Get models
 * @param req HTTP request
 * @return ESP_OK on success
 *      ESP_FAIL on failure
 */
static esp_err_t get_all_models(httpd_req_t *req)
{
    char *buf;
    size_t buf_len;
    bool summary = false;

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
        {
            // ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "summary", param, sizeof(param)) == ESP_OK)
            {
                summary = strcmp(param, "true") == 0 ? true : false;
            }
        }
        free(buf);
    }

    // set connection close
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "application/json; charset=utf-8");
    // pega o ponteiro para a estrutura com os dados SunSpec
    suns = get_sunspec();

    cJSON *root = cJSON_CreateObject();
    // pega os dados de todos os modelos e coloca no JSON root
    sunspec_to_cjson(root, suns, summary);
    char *my_json_string = cJSON_Print(root);
    // cJSON_Minify(my_json_string);
    // preenche os dados da resposta http
    httpd_resp_send(req, my_json_string, HTTPD_RESP_USE_STRLEN);

    cJSON_Delete(root);
    free(my_json_string);

    return ESP_OK;
}

/**
 * @brief Get router. This is a protected resource that handles every request
 * using HTTP GET method.
 * @param req HTTP request
 * @return ESP_OK on success
 *      ESP_FAIL on failure
 */
esp_err_t get_router(httpd_req_t *req)
{
    if (basic_auth_handler(req) != ESP_OK)
    {
        // Caso a autenticação falhar, a requisição será preenchida com o código de erro
        //  esta função deve retornar OK para que o servidor envie a resposta de erro
        return ESP_OK;
    }
    // Caso a autenticação seja bem sucedida, a requisição prossegue normalmente

    // ESP_LOGI(TAG, "URI: %s", req->uri);
    // verify model uri
    if (httpd_uri_match_wildcard("/v1/models??*", req->uri, strlen(req->uri)))
    {
        return get_all_models(req);
    }

    // TODO: Tornar as linhas abaixo dinâmicas
    if (httpd_uri_match_wildcard("/v1/models/1/instances/0??*", req->uri, strlen(req->uri)))
    {
        return get_model(req, 1);
    }
    if (httpd_uri_match_wildcard("/v1/models/307/instances/0??*", req->uri, strlen(req->uri)))
    {
        return get_model(req, 307);
    }

    httpd_resp_set_type(req, "application/json; charset=utf-8");
    httpd_resp_set_status(req, HTTPD_404);
    cJSON *error = cJSON_CreateObject();
    new_error(error, "ERR01",
              "URI not match", "URI not match",
              false, NULL);
    char *my_json_string = cJSON_Print(error);
    httpd_resp_send(req, my_json_string, HTTPD_RESP_USE_STRLEN);
    cJSON_Delete(error);
    free(my_json_string);

    return ESP_OK;
}