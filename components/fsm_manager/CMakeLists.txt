idf_component_register(SRCS "sys_fsm.c"
                    INCLUDE_DIRS "include"
                    REQUIRES drivers sys_nvs wifi_service esp_wifi config esp_event)