#include <stdio.h>

#include "nv_params.h"
#include "sys_fsm.h"
#include "esp_event.h"
#include "sys_boot.h"

void app_main(void)
{
    // Starts the NVS
    init_nvs();
    
    // Creates the event loop for notifications and wifi initialization
    esp_event_loop_create_default();

    // Shows ESP32 hardware information
    esp_show_info();

    // Starts the FSM logic
    fsm_init();
    
}
