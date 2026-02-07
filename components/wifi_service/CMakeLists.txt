idf_component_register(SRCS "softap_provisioning.c" "conn_mgr.c"
                    INCLUDE_DIRS "include"  
                    REQUIRES nv_param_store esp_netif esp_wifi esp_event mdns wifi_provisioning)
