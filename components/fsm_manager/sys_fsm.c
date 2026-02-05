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

static const char* fsm_tag = "fsm";

static esp_err_t err = ESP_OK;
static esp_err_t wifi_ret = ESP_OK;
static esp_err_t nvs_ret = ESP_OK;
static bool init_nal;

// FSM task to run
void vTaskFSM( void * pvParameters )
{
    // Uses default initial configuration
    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT(); 
    
    for( ;; )
    {
        vTaskDelay(pdMS_TO_TICKS(10));

        // FSM logic
        switch( current_state )
        {
            /*
                    * @brief Responsible for system-wide hardware and software initialization,
                    * including GPIO mapping, NVS, and wifi stack configuration.
            */
            case STATE_INIT:
            {
                if ( fsm_status != true )
                {
                    err = sys_conf_gpio();
                    if (err != ESP_OK)
                    {
                        current_state = STATE_ERROR;
                        break; 
                    }

                     // Initializes NVS to store Wi-Fi credentials (SSID and password). 
                    err = set_wf_params_nvs();
                    if (err != ESP_OK) {
                        current_state = STATE_ERROR;
                        break;
                    }

                    // Sets up network layer and wifi configuration 
                    err = init_network_abstraction_layer();
                    if( err != ESP_OK )
                    {
                        current_state = STATE_ERROR;
                        break;
                    }

                    // Initializes WiFi connection;
                    err = esp_wifi_init( &init_cfg );
                    if( err != ESP_OK )
                    {
                        current_state = STATE_ERROR;
                        break;
                    }

                    err = init_mdns();
                    if( err != ESP_OK )
                    {
                        ESP_LOGW(fsm_tag, "mDNS failed to start (%s). Discovery by name will be unavailable.", esp_err_to_name(err));
                    }
                    else
                    {
                        ESP_LOGI(fsm_tag, "mDNS initialized successfully.");
                    }

                    err = init_provisioning();
                    if( err != ESP_OK )
                    {
                        current_state = STATE_ERROR;
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

                err = fsm_set_state( STATE_WIFI_CONNECTING );
                if( err != ESP_OK )
                {
                    ESP_LOGE( fsm_tag, "Failed to set state" );
                    current_state = STATE_ERROR;
                    break;
                }

                break;
            }

            //TODO: ======================== STOP POINT ========================
            case STATE_WIFI_CONNECTING:
            {                
                wifi_ret = init_wifi_connection();
                if( wifi_ret != ESP_OK )
                {
                    ESP_LOGE( fsm_tag, "Failed to start connection!" );
                    current_state = STATE_ERROR;
                    break;
                }

                ESP_LOGI( fsm_tag, "Successfully connected to the network!" );
                
                err |= fsm_set_state( STATE_PROVISIONING );
                if( err != ESP_OK )
                {
                    ESP_LOGE( fsm_tag, "State transition failed." );
                    current_state = STATE_ERROR;
                    break;
                }

                break;
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
                if( nvs_ret != ESP_OK)
                {
                    panic_dev_restart( LOW_DELAY_TICK_100, nvs_ret);
                }

                if( init_nal != true)
                {
                    panic_dev_restart( LOW_DELAY_TICK_100, init_nal);
                }

                if( wifi_ret ==  ESP_ERR_TIMEOUT || wifi_ret == ESP_ERR_NOT_FOUND )
                {
                    ESP_LOGE( fsm_tag, "Timeout error or resource not found: %s", esp_err_to_name( wifi_ret ) );
                    current_state = STATE_PROVISIONING;
                    break;
                }

                switch( err )
                {
                    case ESP_ERR_NO_MEM:
                    {
                        panic_dev_restart( LOW_DELAY_TICK_100, err );
                        break;
                    }
                    default:
                    {
                        ESP_ERROR_CHECK_WITHOUT_ABORT( err );
                        current_state = STATE_INIT;
                        break;
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/*
    * Used for restart ESP32 completely and log error
*/
void panic_dev_restart( TickType_t ms, esp_err_t error_ret )
{
    esp_wifi_stop();
    nvs_flash_deinit();
    
    vTaskDelay( pdMS_TO_TICKS( ms ) );
    ESP_ERROR_CHECK( error_ret );
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
