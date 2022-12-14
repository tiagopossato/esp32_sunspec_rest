#include <esp_event.h>
#include <esp_log.h>
#include "esp_wifi.h"
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "cJSON.h"

#include "esp_tls.h"
#include "sdkconfig.h"

#include "app_wifi.h"
#include "app_https_server.h"
#include "app_sunspec_models.h"

#define HTTPS_SERVER_ENABLE

#ifdef HTTPS_SERVER_ENABLE
#include <esp_https_server.h>
#else
#include <esp_http_server.h>
#endif

static const char *TAG = "HTTPS Server";

static SunSpec *suns;

extern esp_err_t basic_auth_handler(httpd_req_t *req);

extern esp_err_t httpd_register_uri_handler_with_auth(httpd_handle_t handle,
                                                      httpd_uri_t *uri_handler);

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
static esp_err_t get_router(httpd_req_t *req)
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
              "URI no match", "URI no match",
              false, NULL);
    char *my_json_string = cJSON_Print(error);
    httpd_resp_send(req, my_json_string, HTTPD_RESP_USE_STRLEN);
    cJSON_Delete(error);
    free(my_json_string);

    return ESP_OK;
}

// Única URI registrada no servidor para o verbo GET
// Desta forma todas as requisições devem passar pela função de roteamento,
// responsável pela autenticação e despacho das requisições
httpd_uri_t uri_get_models = {
    .uri = "/?*", // “?*” to make the previous character optional, and if present, allow anything to follow.
    .method = HTTP_GET,
    .handler = get_router};

static bool httpd_uri_match_simple(const char *uri1, const char *uri2, size_t len2)
{
    return strlen(uri1) == len2 &&           // First match lengths
           (strncmp(uri1, uri2, len2) == 0); // Then match actual URIs
}

static esp_err_t patch_router(httpd_req_t *req)
{
    if (basic_auth_handler(req) != ESP_OK)
    {
        // Caso a autenticação falhar, a requisição será preenchida com o código de erro
        //  esta função deve retornar OK para que o servidor envie a resposta de erro
        return ESP_OK;
    }
    // Caso a autenticação seja bem sucedida, a requisição prossegue normalmente

    /**********TESTES***************/
    char *buf;
    size_t buf_len;
    buf_len = httpd_req_get_hdr_value_len(req, "Content-Type") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Content-Type", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found header => Content-Type: %s", buf);
        }
        free(buf);
    }
    /**********FIM DOS TESTES***************/
    // TODO: Tornar as linhas abaixo dinâmicas
    if (httpd_uri_match_simple("/v1/models/1/instances/0", req->uri, strlen(req->uri)))
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
            return ESP_OK;
        }

        cJSON *error_response = cJSON_CreateObject();
        suns = get_sunspec();
        if (false == patch_points(array_points, suns, 1, error_response))
        {
            // print error
            char *my_json_string = cJSON_Print(error_response);
            // printf("error_response: %s\n", my_json_string);

            httpd_resp_set_hdr(req, "Connection", "close");
            httpd_resp_set_type(req, "application/json; charset=utf-8");
            httpd_resp_set_status(req, HTTPD_400);

            httpd_resp_send(req, my_json_string, HTTPD_RESP_USE_STRLEN);

            cJSON_Delete(error_response);
            // cJSON_Delete(json_content);

            free(my_json_string);
            free(content);
            return ESP_OK;
        }

        cJSON_Delete(error_response);
        // cJSON_Delete(json_content);

        // set connection close
        httpd_resp_set_hdr(req, "Connection", "close");
        httpd_resp_set_type(req, "application/json; charset=utf-8");
        httpd_resp_send(req, content, HTTPD_RESP_USE_STRLEN);
        free(content);
        return ESP_OK;
    }

    http_send_error(req, HTTPD_404, "PATCH-ERR05",
                    "URI no match", "URI no match",
                    false, NULL);

    return ESP_OK;
}

httpd_uri_t uri_patch_models = {
    .uri = "/?*", // “?*” to make the previous character optional, and if present, allow anything to follow.
    .method = HTTP_PATCH,
    .handler = patch_router};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server");

#ifdef HTTPS_SERVER_ENABLE
    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();
    // config.httpd.lru_purge_enable = true; //Purge “Least Recently Used” connection

    extern const unsigned char servercert_start[] asm("_binary_servercert_pem_start");
    extern const unsigned char servercert_end[] asm("_binary_servercert_pem_end");
    config.servercert = servercert_start;
    config.servercert_len = servercert_end - servercert_start;

    extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
    extern const unsigned char prvtkey_pem_end[] asm("_binary_prvtkey_pem_end");
    config.prvtkey_pem = prvtkey_pem_start;
    config.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;

    config.httpd.uri_match_fn = httpd_uri_match_wildcard;

    esp_err_t ret = httpd_ssl_start(&server, &config);
#else
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    // config.lru_purge_enable = true;//Purge “Least Recently Used” connection
    config.uri_match_fn = httpd_uri_match_wildcard;

    // Start the httpd server
    esp_err_t ret = httpd_start(&server, &config);
#endif

    if (ESP_OK != ret)
    {
        ESP_LOGI(TAG, "Error starting server!");
        return NULL;
    }

    // Set URI handlers
    ESP_LOGI(TAG, "Registering URI handlers");

    if (httpd_register_uri_handler_with_auth(server, &uri_get_models) != ESP_OK)
    {
        ESP_LOGI(TAG, "Error Registering URI uri_get_models!");
        return NULL;
    }

    if (httpd_register_uri_handler_with_auth(server, &uri_patch_models) != ESP_OK)
    {
        ESP_LOGI(TAG, "Error Registering URI uri_patch_models!");
        return NULL;
    }

    return server;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
#ifdef HTTPS_SERVER_ENABLE
    // Stop the httpd server
    return httpd_ssl_stop(server);
#else
    // Stop the httpd server
    return httpd_stop(server);
#endif
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server)
    {
        if (stop_webserver(*server) == ESP_OK)
        {
            *server = NULL;
        }
        else
        {
            ESP_LOGE(TAG, "Failed to stop https server");
        }
    }
}

static void connect_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        *server = start_webserver();
    }
}

void app_https_server_start(void)
{
    static httpd_handle_t server = NULL;

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Register event handlers to start server when Wi-Fi is connected,
     * and stop server when disconnection happens.
     */
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &disconnect_handler, &server));

    /* Start the server for the first time */
    server = start_webserver();

    app_wifi_init_softap();
}