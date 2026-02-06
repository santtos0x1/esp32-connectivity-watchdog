#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/wifi_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sys_fsm.h"
#include "hal_pins.h"
#include "nv_params.h"
#include "conn_mgr.h"
#include "sys_conf.h"
#include "softap_provisioning.h"

static bool fsm_status = false;

static system_state_t current_state = STATE_INIT;

static const char * fsm_tag = "fsm";

static esp_err_t err;
static esp_err_t ret_transition_err;

// FSM task to run
void vTaskFSM( void * pvParameters )
{
    // Uses default initial configuration
    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT(); 
    
    for( ;; )
    {
        // Prevent watchdog triggers and allow task switching
        vTaskDelay( pdMS_TO_TICKS( 10 ) );

        // FSM logic
        switch( current_state )
        {
            /*
                    * @brief Responsible for system-wide hardware and software initialization,
                    * including GPIO mapping, NVS, and wifi stack configuration.
            */
            case STATE_INIT:
            {
                // Verify if initialization was already performed
                if ( fsm_status != true )
                {
                    err = sys_conf_gpio();
                    if ( err != ESP_OK )
                    {
                        ret_transition_err = fsm_set_state( STATE_ERROR );
                        if( ret_transition_err != ESP_OK )
                        {
                            panic_dev_restart( 100, ret_transition_err );
                        }
                
                        break; 
                    }

                     // Initializes NVS to store Wi-Fi credentials (SSID and password). 
                    err = set_wf_params_nvs();
                    if ( err != ESP_OK ) {
                        ret_transition_err = fsm_set_state( STATE_ERROR );
                        if( ret_transition_err != ESP_OK )
                        {
                            panic_dev_restart( 100, ret_transition_err );
                        }
                
                        break;
                    }

                    // Sets up network layer and wifi configuration 
                    err = init_network_abstraction_layer();
                    if( err != ESP_OK )
                    {
                        ret_transition_err = fsm_set_state( STATE_ERROR );
                        if( ret_transition_err != ESP_OK )
                        {
                            panic_dev_restart( 100, ret_transition_err );
                        }
                
                        break;
                    }

                    // Initializes WiFi connection;
                    err = esp_wifi_init( &init_cfg );
                    if( err != ESP_OK )
                    {
                        ret_transition_err = fsm_set_state( STATE_ERROR );
                        if( ret_transition_err != ESP_OK )
                        {
                            panic_dev_restart( 100, ret_transition_err );
                        }
                
                        break;
                    }

                    // Register an event from event handler
                    ESP_ERROR_CHECK( esp_event_handler_register( 
                        WIFI_PROV_EVENT, 
                        ESP_EVENT_ANY_ID, 
                        provisioning_event_handler, 
                        NULL 
                    ) );
                }

                // Attempt to transition to the connection state
                ret_transition_err = fsm_set_state( STATE_WIFI_CONNECTING );
                if( ret_transition_err != ESP_OK )
                {
                    ret_transition_err = fsm_set_state( STATE_ERROR );
                    if( ret_transition_err != ESP_OK )
                    {
                        panic_dev_restart( 100, ret_transition_err );
                    }
                
                    break;
                }

                fsm_status = true;
                
                break;
            }
            case STATE_WIFI_CONNECTING:
            {                
                // Execute WiFi connection logic using stored parameters
                err = init_wifi_connection();
                if( err == ESP_OK )
                {
                    // Transition to provisioning state once connected
                    ret_transition_err = fsm_set_state( STATE_MQTT_CONNECTING );
                    if( ret_transition_err != ESP_OK )
                    {
                        ret_transition_err = fsm_set_state( STATE_ERROR );
                        if( ret_transition_err != ESP_OK )
                        {
                            panic_dev_restart( 100, ret_transition_err );
                        }
                    
                        break;
                    }

                    break;   
                } 
                else
                {
                    err = init_mdns();
                    if( err != ESP_OK )
                    {
                        ESP_LOGW( fsm_tag, "mDNS failed to start (%s). Discovery by name will be unavailable.", esp_err_to_name( err ) );
                    }
                    else
                    {
                        ESP_LOGI( fsm_tag, "mDNS initialized successfully." );
                    }

                    // Start the provisioning service via SoftAP
                    err = init_provisioning();
                    if( err != ESP_OK )
                    {
                        ret_transition_err = fsm_set_state( STATE_ERROR );
                        if( ret_transition_err != ESP_OK )
                        {
                            panic_dev_restart( 100, ret_transition_err );
                        }
                    
                        break;
                    }

                    ret_transition_err = fsm_set_state( STATE_PROVISIONING );
                    if( ret_transition_err != ESP_OK )
                    {
                        ret_transition_err = fsm_set_state( STATE_ERROR );
                        if( ret_transition_err != ESP_OK )
                        {
                            panic_dev_restart( 100, ret_transition_err );
                        }
                    
                        break;
                    }
                }

                break;
            }
            case STATE_PROVISIONING:
            {
                break;
            }
            case STATE_MQTT_CONNECTING:
            {
                break;
            }
            case STATE_OPERATIONAL_ONLINE:
            {
                break;
            }
            case STATE_OPERATIONAL_OFFLINE:
            {
                break;
            }
            case STATE_SYNCING:
            {
                break;
            }
            // Used for treat all the errors in FSM
            case STATE_ERROR:
            {
                if( ret_transition_err == ESP_FAIL)
                {
                    ESP_ERROR_CHECK_WITHOUT_ABORT( err );
                    ret_transition_err = fsm_set_state( STATE_INIT );
                    if( ret_transition_err != ESP_OK )
                    {
                        panic_dev_restart( 100, ret_transition_err );
                    }

                    break;
                }
                
                // Identify the specific cause of the system failure
                switch( err )
                {
                    case ESP_ERR_NO_MEM:
                    {
                        panic_dev_restart( LOW_DELAY_TICK_100, err );
                    }
                    case ESP_ERR_WIFI_NOT_CONNECT:
                    case ESP_ERR_TIMEOUT:
                    {
                        // Log the error and fall back to provisioning mode
                        ESP_ERROR_CHECK_WITHOUT_ABORT( err );
                        ret_transition_err = fsm_set_state( STATE_PROVISIONING );
                        if( ret_transition_err != ESP_OK )
                        {
                            panic_dev_restart( 100, ret_transition_err );
                        }
                        
                        break;
                    }
                    case ESP_ERR_WIFI_PASSWORD:
                    {
                        // Handle incorrect credentials by restarting provisioning
                        ESP_LOGE( fsm_tag, "WiFi password incorrect: %s", esp_err_to_name( err ) );
                        ret_transition_err = fsm_set_state( STATE_PROVISIONING );
                        if( ret_transition_err != ESP_OK )
                        {
                            panic_dev_restart( 100, ret_transition_err );
                        }
                        
                        break;
                    }
                    case ESP_ERR_NOT_FOUND:
                    default:
                    {
                        // Reset the machine to the initialization state for a fresh start
                        ESP_ERROR_CHECK_WITHOUT_ABORT( err );
                        ret_transition_err = fsm_set_state( STATE_INIT );
                        if( ret_transition_err != ESP_OK )
                        {
                            panic_dev_restart( 100, ret_transition_err );
                        }

                        break;
                    }
                }
            }

            break;
        }
        // Periodic delay to manage state machine execution frequency
        vTaskDelay( pdMS_TO_TICKS( 10 ) );
    }
}

// Retrieves the name associated with a specific state type
static const char * state_to_name( system_state_t state )
{
    switch( state )
    {
        case 0:
        {
            return "STATE_INIT";
            break;
        }
        case 1:
        {
            return "STATE_WIFI_CONNECTING";
            break;
        }
        case 2:
        {
            return "STATE_PROVISIONING";
            break;
        }
        case 3:
        {
            return "STATE_MQTT_CONNECTING";
            break;
        }
        case 4:
        { 
            return "STATE_OPERATIONAL_ONLINE";
            break;
        }
        case 5:
        {
            return "STATE_OPERATIONAL_OFFLINE";
            break;
        }
        case 6:
        {
            return "STATE_SYNCING";
            break;
        }
        case 7:
        {
            return "STATE_ERROR";
            break;
        }
        default:
        {
            return "State not found";
            break;
        }
    }
}

// Used for restart ESP32 completely and log error
void panic_dev_restart( TickType_t ms, esp_err_t error_ret )
{   
    vTaskDelay( pdMS_TO_TICKS( ms ) );
    ESP_ERROR_CHECK( error_ret );
}

// Responsible for the task creation
void fsm_init( void )
{   
    xTaskCreate( vTaskFSM, "FSM", V_FSM_STACK_BUFFER, NULL, tskIDLE_PRIORITY, NULL );
}

// Sets the state of the FSM
esp_err_t fsm_set_state( system_state_t new_state )
{   
    /*
        Performs a bitwise AND operation with a mask of 1 to isolate the shifted bit,
        returning its boolean state.
    */
    if( ( state_bitmask[ current_state ] >> new_state ) & 1 )
    {   
        current_state = new_state;
        ESP_LOGI( fsm_tag, "State changed to: %s", state_to_name( current_state ) );
        
        return ESP_OK;
    }
    else
    {
        // Log illegal transition attempt based on the bitmask
        ESP_LOGE(
            fsm_tag,
            "Cannot change the state: %s to %s", state_to_name( current_state ), state_to_name( new_state )
        );

        return ESP_FAIL;
    }
}

// Gets the state of the FSM
system_state_t fsm_get_state( void )
{
    return current_state;
}
