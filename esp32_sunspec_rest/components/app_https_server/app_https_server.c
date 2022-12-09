#include <esp_event.h>
#include <esp_log.h>
#include "esp_wifi.h"
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "cJSON.h"

#include <esp_https_server.h>
#include "esp_tls.h"
#include "sdkconfig.h"

#include "app_wifi.h"
#include "app_https_server.h"
#include "app_sunspec_models.h"

static const char *TAG = "HTTPS Server";

static SunSpec *suns;

static esp_err_t get_model(httpd_req_t *req, uint16_t model_id)
{
    char *buf;
    size_t buf_len;
    char *csv_points;
    cJSON *root;
    char *my_json_string;

    // set connection close
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "application/json");

    // APAGAR DEPOIS
    //  SET refresh header
    httpd_resp_set_hdr(req, "Refresh", "5");

    suns = get_sunspec();
    root = cJSON_CreateObject();

    // csv_points = malloc(1); // TODO: isso é para não dar erro na compilação pois o compilador não enxerga que é feito malloc. ver isso
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
                get_model_cjson_points_by_name(root, suns, model_id, csv_points);
                my_json_string = cJSON_Print(root);
                cJSON_Minify(my_json_string);
                httpd_resp_send(req, my_json_string, HTTPD_RESP_USE_STRLEN);
                cJSON_Delete(root);
                free(my_json_string);
                free(csv_points);
                free(buf);
                return ESP_OK;
            }
            free(csv_points);
        }
        free(buf);
    }

    // if no points are specified, return all points
    get_model_cjson_by_id(root, suns, model_id);
    my_json_string = cJSON_Print(root);
    cJSON_Minify(my_json_string);
    httpd_resp_send(req, my_json_string, HTTPD_RESP_USE_STRLEN);
    cJSON_Delete(root);
    free(my_json_string);

    return ESP_OK;
}

/* An HTTP GET handler for models*/
static esp_err_t get_models(httpd_req_t *req)
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

    httpd_resp_set_type(req, "application/json");
    suns = get_sunspec();
    cJSON *root = cJSON_CreateObject();
    sunspec_to_cjson(root, suns, summary);
    char *my_json_string = cJSON_Print(root);
    cJSON_Minify(my_json_string);

    httpd_resp_send(req, my_json_string, HTTPD_RESP_USE_STRLEN);

    cJSON_Delete(root);
    free(my_json_string);

    return ESP_OK;
}

/* An HTTP GET handler for models*/
static esp_err_t get_models_router(httpd_req_t *req)
{
    // print uri
    // ESP_LOGI(TAG, "URI: %s", req->uri);
    // verify model uri
    if (httpd_uri_match_wildcard("/v1/models??*", req->uri, strlen(req->uri)))
    {
        return get_models(req);
    }

    if (httpd_uri_match_wildcard("/v1/models/1/instances/0??*", req->uri, strlen(req->uri)))
    {
        return get_model(req, 1);
    }
    // verify model uri
    if (httpd_uri_match_wildcard("/v1/models/307/instances/0??*", req->uri, strlen(req->uri)))
    {
        return get_model(req, 307);
    }

    httpd_resp_send_404(req);

    return ESP_OK;
}

static const httpd_uri_t uri_get_models = {
    .uri = "/v1/models/?*", // “?*” to make the previous character optional, and if present, allow anything to follow.
    .method = HTTP_GET,
    .handler = get_models_router};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server");

    httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();

    extern const unsigned char servercert_start[] asm("_binary_servercert_pem_start");
    extern const unsigned char servercert_end[] asm("_binary_servercert_pem_end");
    conf.servercert = servercert_start;
    conf.servercert_len = servercert_end - servercert_start;

    extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
    extern const unsigned char prvtkey_pem_end[] asm("_binary_prvtkey_pem_end");
    conf.prvtkey_pem = prvtkey_pem_start;
    conf.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;

    conf.httpd.uri_match_fn = httpd_uri_match_wildcard;

    esp_err_t ret = httpd_ssl_start(&server, &conf);
    if (ESP_OK != ret)
    {
        ESP_LOGI(TAG, "Error starting server!");
        return NULL;
    }

    // Set URI handlers
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &uri_get_models);

    return server;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_ssl_stop(server);
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

    app_wifi_init_softap();
}