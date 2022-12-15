#include <esp_log.h>
#include <esp_system.h>
#include "cJSON.h"
#include "sdkconfig.h"

#include <esp_https_server.h>
#include <esp_http_server.h>
#include "esp_tls_crypto.h"
#include "app_https_server.h"
#include "sunspec_models.h"

#define BASIC_AUTH_USERNAME "admin"
#define BASIC_AUTH_PASSWORD "admin"
#define HTTPD_401 "401 Unauthorized" /*!< HTTP Response 401 */

static const char *TAG = "HTTP AUTH";

extern void http_send_error(httpd_req_t *req, const char *http_status,
                            char *errCode, char *errMessage, char *errReason,
                            bool debug, char *TBD);
// Structure for basic authentication
typedef struct
{
    char *username;
    char *password;
} basic_auth_info_t;

/**
 * @brief Basic authentication
 * @param username Username
 * @param password Password
 * @return Pointer to the digest
 *       NULL on failure
 * @note The caller is responsible for freeing the returned digest
 */
static char *http_auth_basic(const char *username, const char *password)
{
    int out;
    char *user_info = NULL;
    char *digest = NULL;
    size_t n = 0;

    // junta o username e o password em uma string separada por ":"
    asprintf(&user_info, "%s:%s", username, password);
    if (!user_info)
    {
        ESP_LOGE(TAG, "No enough memory for user information");
        return NULL;
    }

    // calcula o tamanho da string em base64
    esp_crypto_base64_encode(NULL, 0, &n, (const unsigned char *)user_info, strlen(user_info));

    /* 6: The length of the "Basic " string
     * n: Number of bytes for a base64 encode format
     * 1: Number of bytes for a reserved which be used to fill zero
     */
    digest = calloc(1, 6 + n + 1);
    // se conseguir alocar memória
    if (digest)
    {
        // adiciona o cabeçalho "Basic " na string
        strcpy(digest, "Basic ");
        // adiciona a string em base64 na string
        esp_crypto_base64_encode((unsigned char *)digest + 6, n, (size_t *)&out, (const unsigned char *)user_info, strlen(user_info));
    }
    // libera a memória alocada para a string user_info
    free(user_info);
    // retorna a string com o cabeçalho e a string em base64
    return digest;
}

/**
 * @brief Basic authentication handler
 * @param req HTTP request
 * @return ESP_OK on success
 *        ESP_FAIL on failure
 */
esp_err_t basic_auth_handler(httpd_req_t *req)
{
#ifndef AUTHENTICATION_ENABLE
    return ESP_OK;
#endif

    char *buf = NULL;
    size_t buf_len = 0;

    // pega a informação de autenticação do usuário no contexto da requisição
    basic_auth_info_t *basic_auth_info = req->user_ctx;

    // pega o tamanho do cabeçalho de autorização
    buf_len = httpd_req_get_hdr_value_len(req, "Authorization") + 1;

    // se o tamanho do cabeçalho for maior que 1, significa que o cabeçalho existe
    if (buf_len > 1)
    {
        // aloca memória para o cabeçalho
        buf = calloc(1, buf_len);
        // se não conseguir alocar memória, retorna erro
        if (!buf)
        {
            ESP_LOGE(TAG, "No enough memory for basic authorization");

            httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic");

            http_send_error(req, HTTPD_500, "AUTH-ERR01",
                            "UNAUTHORIZED", "No enough memory for basic authorization",
                            false, NULL);

            return ESP_ERR_NO_MEM;
        }

        // se conseguir alocar memória, pega o valor do cabeçalho de autorização
        // se não conseguir pegar o valor do cabeçalho, retorna erro
        if (httpd_req_get_hdr_value_str(req, "Authorization", buf, buf_len) != ESP_OK)
        {
            ESP_LOGE(TAG, "No auth value received");
            free(buf);
            httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic");
            http_send_error(req, HTTPD_401, "AUTH-ERR02",
                            "UNAUTHORIZED", "No auth value received",
                            false, NULL);

            return ESP_FAIL;
        }
        else
        {
            ESP_LOGI(TAG, "Found header => Authorization: %s", buf);
        }
        // cria a string de autenticação básica composta por "Basic " e a string username:password em base64
        char *auth_credentials = http_auth_basic(basic_auth_info->username, basic_auth_info->password);

        // se não conseguir alocar memória para a string de autenticação básica, retorna erro
        if (!auth_credentials)
        {
            ESP_LOGE(TAG, "No enough memory for basic authorization credentials");
            // libera a memória alocada para o cabeçalho
            free(buf);
            httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic");
            http_send_error(req, HTTPD_500, "AUTH-ERR01",
                            "UNAUTHORIZED", "No enough memory for basic authorization credentials",
                            false, NULL);

            return ESP_ERR_NO_MEM;
        }

        // FINALMENTE, compara a string de autenticação básica com a string recebida no cabeçalho
        // se a string de autenticação básica for diferente da string do cabeçalho, retorna erro
        if (strncmp(auth_credentials, buf, buf_len) != 0)
        {
            ESP_LOGE(TAG, "Not authenticated");

            httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic");
            http_send_error(req, HTTPD_401, "AUTH-ERR03",
                            "UNAUTHORIZED", "Not authenticated",
                            false, NULL);

            free(auth_credentials);
            free(buf);
            return ESP_FAIL;
        }
        // se a string de autenticação básica for igual da string do cabeçalho
        // o usuário está autenticado
        // libera a memória alocada para a string de autenticação básica e o cabeçalho
        // não toca na requisição
        // retorna sucesso
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
    // se não receber o cabeçalho de autorização, retorna erro
    else
    {
        ESP_LOGE(TAG, "No auth header received");
        httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic");
        http_send_error(req, HTTPD_401, "AUTH-ERR03",
                        "UNAUTHORIZED", "No auth header received",
                        false, NULL);

        return ESP_FAIL;
    }

    return ESP_FAIL;
}

/**
 * @brief Register URI handlers with basic authentication in user context data
 * @param handle HTTPD server handle
 * @param uri_handler URI handler
 * @return ESP_OK on success
 *        ESP_FAIL on failure
 */
esp_err_t httpd_register_uri_handler_with_auth(httpd_handle_t handle,
                                               httpd_uri_t *uri_handler)
{
#ifdef AUTHENTICATION_ENABLE
    basic_auth_info_t *basic_auth_info = calloc(1, sizeof(basic_auth_info_t));
    if (basic_auth_info)
    {
        basic_auth_info->username = BASIC_AUTH_USERNAME;
        basic_auth_info->password = BASIC_AUTH_PASSWORD;
        uri_handler->user_ctx = basic_auth_info;
        return httpd_register_uri_handler(handle, uri_handler);
    }
    return ESP_FAIL;

#endif

    return httpd_register_uri_handler(handle, uri_handler);
}