#include <stdio.h>

#include "esp_netif.h"

#include "nv_params.h"
#include "sys_fsm.h"
#include "esp_event.h"
#include "sys_boot.h"
#include "sys_diag.h"
#include "mqtt_service.h"

void app_main(void)
{
    // Starts the NVS
    init_nvs();

    ESP_ERROR_CHECK(esp_netif_init());
    
    // Creates the event loop for notifications and wifi initialization
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Shows ESP32 hardware information
    esp_show_info();

    // Tasks setup
    init_diag();
    mqtt_init();

    fsm_init();
}
