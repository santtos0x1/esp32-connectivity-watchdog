#ifndef SYS_FSM_H
#define SYS_FSM_H

#include "freertos/FreeRTOS.h"

typedef enum {
    STATE_INIT,    
    STATE_WIFI_CONNECTING,    
    STATE_PROVISIONING,    
    STATE_MQTT_CONNECTING,    
    STATE_OPERATIONAL_ONLINE, 
    STATE_OPERATIONAL_OFFLINE,    
    STATE_SYNCING,    
    STATE_ERROR
} system_state_t;

/* 
    Implements a state transition table using an 8-byte bitmask for a minimal memory footprint.
    This enforces strict transition rules within the set_state function,
    ensuring system security and predictable behavior.
*/
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

esp_err_t fsm_set_state( system_state_t new_state );
system_state_t fsm_get_state( void );
void vTaskFSM( void * pvParameters );
void fsm_init( void );
void panic_dev_restart( TickType_t ms, esp_err_t error_ret );

#endif // !SYS_FSM_H