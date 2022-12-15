#pragma once

#define HTTPS_SERVER_ENABLE
#define AUTHENTICATION_ENABLE
// #define WIFI_SOFTAP_MODE

#ifdef __cplusplus
extern "C"
{
#endif

    void app_https_server_start(void);

#ifdef __cplusplus
}
#endif
