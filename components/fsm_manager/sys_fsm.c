#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "include/sys_fsm.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pins.h"

// Defines the stack buffer for fsm task
#define V_FSM_STACK_BUFFER 2048

// Tag for debugging 
static const char* fsm_tag = "FSM"; 

static system_state_t current_state;

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

// FSM task to run
static void vTaskFSM( void * pvParameters )
{
    for( ;; )
    {
        // FSM logic
        switch( current_state )
        {
            /*
                Responsible for system-wide hardware and software initialization,
                including GPIO mapping, NVS flash, and Wi-Fi stack configuration. 
            */
            case STATE_INIT:
            {
                // init GPIO pins configuration;
                sys_conf_gpio();
                
                // Try to set the state to 
                if( fsm_set_state( STATE_WIFI_CONNECTING ) )
                {
                    ESP_LOGI( fsm_tag, "Setting state to: STATE_WIFI_CONNECTING" );
                }
                else 
                {
                    ESP_LOGE( fsm_tag, "Error to set state to: STATE_WIFI_CONNECTING" );
                }
                break;
            }
            case STATE_WIFI_CONNECTING:
            {

            }
            case STATE_PROVISIONING:
            {

            }
            case STATE_MQTT_CONNECTING:
            {

            }
            case STATE_OPERATIONAL_ONLINE:
            {

            }
            case STATE_OPERATIONAL_OFFLINE:
            {

            }
            case STATE_SYNCING:
            {

            }
            case STATE_ERROR:
            {

            }
        }
    }
}

// Responsible to create the task
void fsm_init( void )
{   
    xTaskCreate(vTaskFSM, "FSM", V_FSM_STACK_BUFFER, NULL, tskIDLE_PRIORITY, NULL);
}

// Sets the state of the FSM
bool fsm_set_state( system_state_t new_state )
{   
    /*
        Performs a bitwise AND operation with a mask of 1 to isolate the shifted bit,
        returning its boolean state.
    */
    if( ( state_bitmask[ current_state ] >> new_state ) & 1 )
    {   
        current_state = new_state;
        ESP_LOGI( fsm_tag, "State changed to: %s\n", ( char * )current_state );
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

// Gets the state of the FSM
system_state_t fsm_get_state( void )
{
    return current_state;
}
