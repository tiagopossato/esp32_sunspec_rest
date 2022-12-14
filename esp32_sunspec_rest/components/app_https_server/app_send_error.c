#include <esp_log.h>
#include <esp_system.h>
#include "cJSON.h"
#include "sdkconfig.h"

#include <esp_https_server.h>
#include <esp_http_server.h>

#include "app_https_server.h"
#include "sunspec_models.h"

/**
 * @brief Send error message to client
 * @param[in] req HTTP request
 * @param[in] http_status HTTP code
 * @param errCode Error code that can be used programmatically
 * @param errMessage Concise error message suitable for display.
 * @param errReason Detailed error information describing the cause of the error in addition to the
HTTP status code cause.
 * @param debug Very detailed debug information.
 * @param TBD TBD If debug is true, this field is required. Possibly add request information such as target resource, target values, and
HTTP method.
 */
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