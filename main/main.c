#include <stdio.h>
#include "nv_params.h"
#include "sys_fsm.h"
#include "esp_event.h"

void app_main(void)
{
    // Starts the NVS
    init_nvs_storage();
    
    // Creates the event loop for notifications and wifi initialization
    esp_event_loop_create_default();

    // Starts the FSM logic
    fsm_init();
    
}
