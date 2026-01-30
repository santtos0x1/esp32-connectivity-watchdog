#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "esp_wifi.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sys_fsm.h"
#include "hal_pins.h"
#include "nv_params.h"
#include "conn_mgr.h"
#include "sys_conf.h"

// Tag for debugging 
static const char* fsm_tag = "fsm";

static system_state_t current_state = STATE_INIT;

static esp_err_t err;
static esp_err_t wifi_ret;
static bool init_nal;

// FSM task to run
void vTaskFSM( void * pvParameters )
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
                // Initializes GPIO configurations for the application.
                sys_conf_gpio();
                
                // Initializes NVS to store Wi-Fi credentials (SSID and password).
                set_wf_params_nv_storage();
                err |= fsm_set_state( STATE_WIFI_CONNECTING );
                if( err != ESP_OK )
                {
                    ESP_LOGE( fsm_tag, "Failed to set state" );
                    fsm_set_state( STATE_ERROR );
                    break;
                }

                ESP_LOGI( fsm_tag, "Transitioning to state: STATE_WIFI_CONNECTING" );
                break;
            }
            case STATE_WIFI_CONNECTING:
            {
                // Sets up network layer and wifi configuration 
                init_nal = init_network_abstraction_layer();
                if( init_nal != true )
                {
                    ESP_LOGE( fsm_tag, "Network Abstraction Layer failed to init." );
                    fsm_set_state( STATE_ERROR );
                    break;
                }
                wifi_ret = init_wifi_connection();

                if( wifi_ret == ESP_OK )
                {
                    ESP_LOGI( fsm_tag, "Successfully connected to the network!" );
                    err |= fsm_set_state( STATE_PROVISIONING );

                    if( err != ESP_OK )
                    {
                        ESP_LOGE( fsm_tag, "State transition failed." );
                        fsm_set_state( STATE_ERROR );
                        break;
                    }

                    ESP_LOGI( fsm_tag, "Transitioning to state: STATE_PROVISIONING" );
                    break;
                }
                else
                {
                    ESP_LOGE( fsm_tag, "Failed to start connection!" );
                    fsm_set_state( STATE_ERROR );
                    break;
                }
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
            // Used for treat all the errors in FSM
            case STATE_ERROR:
            {
                if( init_nal != true)
                {
                    // Working...
                }
                switch( err )
                {
                    case ESP_ERR_NO_MEM:
                    {
                        ESP_LOGE( fsm_tag, "Heap memory exhausted. Initiating system reset..." );

                        panic_dev_restart( LOW_DELAY_TICK_100 );
                    }
                    case ESP_ERR_NOT_SUPPORTED:
                    {
                        ESP_LOGE( fsm_tag, "Unsupported operation or configuration." );
                        fsm_set_state( STATE_INIT );
                        break;
                    }
                    case ESP_ERR_NOT_FOUND:
                    {
                        ESP_LOGE( fsm_tag, "Unknown operation." );
                        fsm_set_state( STATE_INIT );
                        break;
                    }
                }

                if( wifi_ret ==  ESP_ERR_TIMEOUT || wifi_ret == ESP_ERR_NOT_FOUND )
                {
                    ESP_LOGE( fsm_tag, "Timeout error or resource not found. Resetting device...");
                    
                    panic_dev_restart( LOW_DELAY_TICK_100 );
                }
            }
        }
    }
}

/*
    * Used for restart ESP32 completely 
*/
void panic_dev_restart( TickType_t ms )
{
    esp_wifi_stop();
    nvs_flash_deinit();
    
    vTaskDelay( pdMS_TO_TICKS( ms ) );
    esp_restart();
}

// Responsible for the task creation
void fsm_init( void )
{   
    xTaskCreate( vTaskFSM, "FSM", V_FSM_STACK_BUFFER, NULL, tskIDLE_PRIORITY, NULL );
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
