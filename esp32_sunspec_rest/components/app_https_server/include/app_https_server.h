#pragma once
#include "cJSON.h"
#include <stdbool.h>

void app_https_server_start(void);

/**
 * @brief Create a new error message
 * @param root cJSON root object
 * @param errCode Error code that can be used programmatically
 * @param errMessage Concise error message suitable for display.
 * @param errReason Detailed error information describing the cause of the error in addition to the
HTTP status code cause.
 * @param debug Very detailed debug information.
 * @param TBD TBD If debug is true, this field is required. Possibly add request information such as target resource, target values, and
HTTP method.
 */
bool new_error(cJSON *root, char *errCode,
               char *errMessage, char *errReason,
               bool debug, char *TBD);