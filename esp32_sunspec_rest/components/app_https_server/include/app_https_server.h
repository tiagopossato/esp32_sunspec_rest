#pragma once
#include "cJSON.h"
#include <stdbool.h>


#define HTTPS_SERVER_ENABLE
#define AUTHENTICATION_ENABLE

void app_https_server_start(void);
