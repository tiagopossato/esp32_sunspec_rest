idf_component_register(SRCS "app_get_router.c" "app_patch_router.c" "app_send_error.c" "app_https_server.c" "app_basic_auth.c"
                    INCLUDE_DIRS "include"
                    EMBED_TXTFILES "certs/servercert.pem"
                                   "certs/prvtkey.pem"
                    PRIV_REQUIRES esp_event 
                        nvs_flash 
                        esp_https_server
                        esp_http_server 
                        esp_netif 
                        esp_wifi
                        json
                        app_wifi
                        sunspec_models
                        app_sunspec_models)