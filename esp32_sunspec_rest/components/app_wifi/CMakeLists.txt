idf_component_register(SRCS "app_ap_wifi.c" "app_sta_wifi.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES nvs_flash esp_wifi)
                    