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

#ifdef HTTPS_SERVER_ENABLE // definido em app_https_server.h
#include <esp_https_server.h>
#else
#include <esp_http_server.h>
#endif

static const char *TAG = "HTTPS Server";

extern esp_err_t basic_auth_handler(httpd_req_t *req);

extern esp_err_t httpd_register_uri_handler_with_auth(httpd_handle_t handle,
                                                      httpd_uri_t *uri_handler);

extern void http_send_error(httpd_req_t *req, const char *http_status,
                            char *errCode, char *errMessage, char *errReason,
                            bool debug, char *TBD);

extern esp_err_t patch_router(httpd_req_t *req);
extern esp_err_t get_router(httpd_req_t *req);

// Única URI registrada no servidor para o verbo GET
// Desta forma todas as requisições devem passar pela função de roteamento,
// responsável pela autenticação e despacho das requisições
httpd_uri_t uri_get_models = {
    .uri = "/?*", // “?*” to make the previous character optional, and if present, allow anything to follow.
    .method = HTTP_GET,
    .handler = get_router};

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
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &disconnect_handler, &server));

    // ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &connect_handler, &server));
    // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &disconnect_handler, &server));


    /* Start the server for the first time */
    server = start_webserver();

    app_wifi_init_sta();
}