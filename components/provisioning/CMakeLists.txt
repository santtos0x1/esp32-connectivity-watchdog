idf_component_register(SRCS "softap_provisioning.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_netif esp_wifi wifi_provisioning mdns esp_event)