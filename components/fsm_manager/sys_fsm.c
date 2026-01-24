#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "include/sys_fsm.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* fsm_tag = "FSM"; 

static system_state_t current_state;
static const uint8_t state_bitmask[ 8 ] = {
    0b10000110, // STATE_INIT
    0b10001101, // STATE_WIFI_CONNECTING
    0b10000001, // STATE_PROVISIONING
    0b10110011, // STATE_MQTT_CONNECTING
    0b11100011, // STATE_OPERATIONAL_ONLINE
    0b11000111, // STATE_OPERATIONAL_OFFLINE
    0b10110011, // STATE_SYNCING
    0b00000001  // STATE_ERROR
};

static void vTaskFSM( void * pvParameters )
{
    for( ;; )
    {
        switch( current_state )
        {
            case STATE_INIT:
            {
                
            }
        }
    }
}

bool fsm_set_state( system_state_t new_state )
{
    if( ( state_bitmask[ current_state ] >> new_state ) & 1 )
    {   
        current_state = new_state;
        ESP_LOGI(fsm_tag, "State changed to: %s\n", ( char * )current_state);
        return true;
    }
    else
    {
        ESP_LOGE(
            fsm_tag,
            "Cannot change the state: %s to %s\n", ( char * )current_state, ( char * )new_state
        );
        return false;
    }
}

system_state_t fsm_get_state( void )
{
    return current_state;
}