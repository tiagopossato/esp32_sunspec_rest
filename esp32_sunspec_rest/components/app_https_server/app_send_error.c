#include <esp_log.h>
#include <esp_system.h>
#include "cJSON.h"
#include "sdkconfig.h"

#include <esp_https_server.h>
#include <esp_http_server.h>

#include "app_https_server.h"
#include "sunspec_models.h"

void http_send_error(httpd_req_t *req, const char *http_status,
                     char *errCode, char *errMessage, char *errReason,
                     bool debug, char *TBD)
{
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "application/json; charset=utf-8");
    httpd_resp_set_status(req, http_status);
    cJSON *error = cJSON_CreateObject();
    new_error(error, errCode, errMessage,
              errReason, false, NULL);
    char *my_json_string = cJSON_Print(error);
    httpd_resp_send(req, my_json_string, HTTPD_RESP_USE_STRLEN);
    cJSON_Delete(error);
    free(my_json_string);
}