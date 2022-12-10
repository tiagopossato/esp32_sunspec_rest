
#include <esp_log.h>
#include <esp_system.h>
#include "cJSON.h"
#include "sdkconfig.h"

#include <esp_https_server.h>
#include <esp_http_server.h>
#include "esp_tls_crypto.h"
#include "app_https_server.h"

#define BASIC_AUTH_USERNAME "admin"
#define BASIC_AUTH_PASSWORD "admin"
#define HTTPD_401 "401 UNAUTHORIZED" /*!< HTTP Response 401 */

static const char *TAG = "HTTP AUTH";

typedef struct
{
    char *username;
    char *password;
} basic_auth_info_t;

static char *http_auth_basic(const char *username, const char *password)
{
    int out;
    char *user_info = NULL;
    char *digest = NULL;
    size_t n = 0;
    asprintf(&user_info, "%s:%s", username, password);
    if (!user_info)
    {
        ESP_LOGE(TAG, "No enough memory for user information");
        return NULL;
    }
    esp_crypto_base64_encode(NULL, 0, &n, (const unsigned char *)user_info, strlen(user_info));

    /* 6: The length of the "Basic " string
     * n: Number of bytes for a base64 encode format
     * 1: Number of bytes for a reserved which be used to fill zero
     */
    digest = calloc(1, 6 + n + 1);
    if (digest)
    {
        strcpy(digest, "Basic ");
        esp_crypto_base64_encode((unsigned char *)digest + 6, n, (size_t *)&out, (const unsigned char *)user_info, strlen(user_info));
    }
    free(user_info);
    return digest;
}

/* An HTTP GET handler */
esp_err_t basic_auth_handler(httpd_req_t *req)
{
    char *buf = NULL;
    size_t buf_len = 0;
    basic_auth_info_t *basic_auth_info = req->user_ctx;

    buf_len = httpd_req_get_hdr_value_len(req, "Authorization") + 1;
    if (buf_len > 1)
    {
        buf = calloc(1, buf_len);
        if (!buf)
        {
            ESP_LOGE(TAG, "No enough memory for basic authorization");
            httpd_resp_send_500(req);
            return ESP_ERR_NO_MEM;
        }

        if (httpd_req_get_hdr_value_str(req, "Authorization", buf, buf_len) != ESP_OK)
        {
            ESP_LOGE(TAG, "No auth value received");
            httpd_resp_set_status(req, HTTPD_401);
            httpd_resp_send(req, NULL, 0);
            free(buf);
            return ESP_FAIL;
        }
        // else
        // {
        //     ESP_LOGI(TAG, "Found header => Authorization: %s", buf);
        // }

        char *auth_credentials = http_auth_basic(basic_auth_info->username, basic_auth_info->password);
        if (!auth_credentials)
        {
            ESP_LOGE(TAG, "No enough memory for basic authorization credentials");
            free(buf);
            httpd_resp_send_500(req);
            return ESP_ERR_NO_MEM;
        }

        if (strncmp(auth_credentials, buf, buf_len))
        {
            ESP_LOGE(TAG, "Not authenticated");
            httpd_resp_set_status(req, HTTPD_401);
            httpd_resp_send(req, NULL, 0);
            free(auth_credentials);
            free(buf);
            return ESP_FAIL;
        }
        else
        {
            // ESP_LOGI(TAG, "Authenticated!");
            free(auth_credentials);
            free(buf);
            return ESP_OK;
        }
        free(auth_credentials);
        free(buf);
    }
    else
    {
        ESP_LOGE(TAG, "No auth header received");
        httpd_resp_set_status(req, HTTPD_401);
        httpd_resp_send(req, NULL, 0);
        return ESP_FAIL;
    }

    return ESP_FAIL;
}

esp_err_t httpd_register_uri_handler_with_auth(httpd_handle_t handle,
                                               httpd_uri_t *uri_handler)
{
    basic_auth_info_t *basic_auth_info = calloc(1, sizeof(basic_auth_info_t));
    if (basic_auth_info)
    {
        basic_auth_info->username = BASIC_AUTH_USERNAME;
        basic_auth_info->password = BASIC_AUTH_PASSWORD;

        uri_handler->user_ctx = basic_auth_info;
        httpd_register_uri_handler(handle, uri_handler);
        return ESP_OK;
    }
    return ESP_FAIL;
}