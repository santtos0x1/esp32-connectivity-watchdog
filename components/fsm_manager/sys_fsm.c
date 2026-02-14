#include "sdkconfig.h"

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
#include "freertos/queue.h"

#include "net_ping.h"
#include "sys_fsm.h"
#include "hal_pins.h"
#include "patterns.h"
#include "nv_params.h"
#include "conn_mgr.h"
#include "hal_map.h"
#include "sys_conf.h"
#include "softap_provisioning.h"

// Defines the stack buffer for fsm task
#ifdef CONFIG_FSM_STACK_SIZE
    #define V_FSM_STACK_BUFFER CONFIG_FSM_STACK_SIZE
#else
    #define V_FSM_STACK_BUFFER 4096
#endif

// Bitmask for system init: 0x1D (00011101 binary) representing GPIO, NAL, WiFi, and Event handlers
#define SYSTEM_READY_BITMASK  0x1D
// Expected bitmask value: 0xE8 (11101000 binary) when all initialization stages are successfully completed.
#define INIT_SUCCESS_BITMASK  0xE8

// Maximum number of items (slots) the queue can hold. 
#define PING_QUEUE_ITEM_SIZE  10

QueueHandle_t ping_queue;

static bool fsm_status = false;
static system_state_t current_state = STATE_INIT;
static const char *fsm_tag = "fsm";

// FSM task to run
void vTaskFSM(void *pvParameters)
{
    ping_result_t p_report;   

    // Initializes errors handlers
    static esp_err_t err;
    static esp_err_t ret_transition_err;

    // Uses default initial configuration
    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT(); 

    for(;;)
    {
        uint8_t ini_bit = SYSTEM_READY_BITMASK;

        // Prevent watchdog triggers and allow task switching
        vTaskDelay(pdMS_TO_TICKS(DELAY_HW_STABILIZE_MS));

        // FSM logic
        switch(current_state)
        {
            /*
                * @brief Responsible for system-wide hardware and software initialization,
                * including GPIO mapping, NVS, and wifi stack configuration.
            */
            case STATE_INIT:
            {
                ini_bit = SYSTEM_READY_BITMASK;
                
                // Verify if initialization was already performed
                if(fsm_status == false)
                {
                    err = sys_conf_gpio();
                    if(err != ESP_OK)
                    {
                        // Set state to STATE_ERROR
                        ret_transition_err = fsm_set_state(STATE_ERROR);
                        if(ret_transition_err != ESP_OK)
                        {
                            panic_dev_restart(LOW_DELAY_TICK_MS, ret_transition_err);
                        }

                        break; 
                    }
                    else
                    {
                        ESP_LOGI(fsm_tag, "GPIO config initialized successfully!");
                        ini_bit <<= 1;
                    }

                    boot_fb(BOOT_FEEDBACK_LED_PIN);

                    vTaskDelay(pdMS_TO_TICKS(DELAY_UI_REFRESH_MS));

                    // Sets up network layer and wifi configuration 
                    err = init_network_abstraction_layer();
                    if(err != ESP_OK)
                    {
                        // Set state to STATE_ERROR
                        ret_transition_err = fsm_set_state(STATE_ERROR);
                        if(ret_transition_err != ESP_OK)
                        {
                            panic_dev_restart(LOW_DELAY_TICK_MS, ret_transition_err);
                        }

                        break;
                    }
                    else
                    {
                        ESP_LOGI(fsm_tag, "NAL initialized successfully!");
                        ini_bit <<= 1;
                    }

                    // Initializes WiFi connection;
                    err = esp_wifi_init(&init_cfg);
                    if(err != ESP_OK)
                    {
                        // Set state to STATE_ERROR
                        ret_transition_err = fsm_set_state(STATE_ERROR);
                        if(ret_transition_err != ESP_OK)
                        {
                            panic_dev_restart(LOW_DELAY_TICK_MS, ret_transition_err);
                        }

                        break;
                    }
                    else
                    {
                        ESP_LOGI(fsm_tag, "WiFi initialized successfully!");
                        ini_bit <<= 1;
                    }

                    // Register an event from event handler
                    ESP_ERROR_CHECK(esp_event_handler_register(
                        WIFI_PROV_EVENT,
                        ESP_EVENT_ANY_ID,
                        provisioning_event_handler,
                        NULL
                    ));

                    if(ini_bit == INIT_SUCCESS_BITMASK)
                    {
                        fsm_status = true;
                        ESP_LOGI(
                            fsm_tag,
                            "Initialization complete. Mask: 0x%02X | Status: %s",
                            ini_bit, fsm_status ? "READY" : "FAILED"
                        );
                        
                        // Attempt to transition to the connection state
                        ret_transition_err = fsm_set_state(STATE_WIFI_CONNECTING);
                        if(ret_transition_err != ESP_OK)
                        {
                            // Sets state to STATE_ERROR
                            ret_transition_err = fsm_set_state(STATE_ERROR);
                            if(ret_transition_err != ESP_OK)
                            {
                                panic_dev_restart(LOW_DELAY_TICK_MS, ret_transition_err);
                            }
                        
                            break;
                        }
                    } 
                    else
                    {
                        fsm_status = false;
                    }
                }

                break;
            }
            case STATE_WIFI_CONNECTING:
            {                
                // Execute WiFi connection logic using stored parameters
                err = init_wifi_connection();
                if(err == ESP_OK)
                {
                    // Transition to provisioning state once connected
                    ret_transition_err = fsm_set_state(STATE_MQTT_CONNECTING);
                    if(ret_transition_err != ESP_OK)
                    {
                        // Set state to STATE_ERROR
                        ret_transition_err = fsm_set_state(STATE_ERROR);
                        if(ret_transition_err != ESP_OK)
                        {
                            panic_dev_restart(LOW_DELAY_TICK_MS, ret_transition_err);
                        }
                    
                        break;
                    }

                    break;   
                } 
                else
                {
                    // Starts mDNS configuration
                    err = init_mdns();
                    if(err != ESP_OK)
                    {
                        ESP_LOGW(
                            fsm_tag, 
                            "mDNS failed to start (%s). Discovery by name will be unavailable.", 
                            esp_err_to_name(err) 
                        );
                    }
                    else
                    {
                        ESP_LOGI(fsm_tag, "mDNS initialized successfully.");
                    }

                    // Start the provisioning service via SoftAP
                    err = init_provisioning();
                    if(err != ESP_OK)
                    {
                        // Sets state to STATE_ERROR
                        ret_transition_err = fsm_set_state(STATE_ERROR);
                        if(ret_transition_err != ESP_OK)
                        {
                            panic_dev_restart(LOW_DELAY_TICK_MS, ret_transition_err);
                        }
                    
                        break;
                    }

                    // Set state to STATE_PROVISIONING
                    ret_transition_err = fsm_set_state(STATE_PROVISIONING);
                    if(ret_transition_err != ESP_OK)
                    {
                        ret_transition_err = fsm_set_state(STATE_ERROR);
                        if(ret_transition_err != ESP_OK)
                        {
                            panic_dev_restart(LOW_DELAY_TICK_MS, ret_transition_err);
                        }
                    
                        break;
                    }
                }

                break;
            }
            case STATE_PROVISIONING:
            {
                vTaskDelay(pdMS_TO_TICKS(LOW_DELAY_TICK_MS));
                break;
            }
            case STATE_MQTT_CONNECTING:
            {
                vTaskDelay(pdMS_TO_TICKS(PING_COOLDOWN_MS));

                err = fsm_set_state(STATE_OPERATIONAL_ONLINE);
                break;
            }
            case STATE_OPERATIONAL_ONLINE:
            {   // Publish information on MQTT broker

                initialize_ping(ping_queue);

                if(xQueueReceive(
                    ping_queue, &p_report, pdMS_TO_TICKS(QUEUE_RECEIVE_DELAY)) == pdPASS
                )
                {
                    if(p_report.received > 0)
                    {
                        ESP_LOGI(fsm_tag, "Network ok, waiting interval...");

                        vTaskDelay(PING_SECURITY_DELAY);
                    }
                    else
                    {
                        ESP_LOGE(fsm_tag, "Ping failed! Trying to reconnect");
                        err = fsm_set_state(STATE_WIFI_CONNECTING);
                        if(err != ESP_OK)
                        {
                            ret_transition_err = fsm_set_state(STATE_ERROR);
                            if(ret_transition_err != ESP_OK)
                            {
                                panic_dev_restart(LOW_DELAY_TICK_MS, ret_transition_err);
                            } 
                        }
                    }
                }

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
                if(ret_transition_err == ESP_FAIL)
                {
                    ESP_ERROR_CHECK_WITHOUT_ABORT(err);
                    
                    // Sets state to STATE_INIT
                    ret_transition_err = fsm_set_state(STATE_INIT);
                    if(ret_transition_err != ESP_OK)
                    {
                        panic_dev_restart(LOW_DELAY_TICK_MS, ret_transition_err);
                    }

                    break;
                }
                
                // Identify the specific cause of the system failure
                switch(err)
                {
                    case ESP_ERR_NO_MEM:
                    {
                        panic_dev_restart(LOW_DELAY_TICK_MS, err);
                    }
                    case ESP_ERR_WIFI_NOT_CONNECT:
                    case ESP_ERR_TIMEOUT:
                    {
                        // Log the error and fall back to provisioning mode
                        ESP_ERROR_CHECK_WITHOUT_ABORT(err);

                        // Sets state to STATE_PROVISIONING
                        ret_transition_err = fsm_set_state(STATE_PROVISIONING);
                        if(ret_transition_err != ESP_OK)
                        {
                            panic_dev_restart(LOW_DELAY_TICK_MS, ret_transition_err);
                        }
                        
                        break;
                    }
                    case ESP_ERR_WIFI_PASSWORD:
                    {
                        // Handle incorrect credentials by restarting provisioning
                        ESP_LOGE(
                            fsm_tag, 
                            "WiFi password incorrect: %s", 
                            esp_err_to_name(err) 
                        );

                        // Sets state to STATE_PROVISIONING
                        ret_transition_err = fsm_set_state(STATE_PROVISIONING);
                        if(ret_transition_err != ESP_OK)
                        {
                            panic_dev_restart(LOW_DELAY_TICK_MS, ret_transition_err);
                        }
                        
                        break;
                    }
                    case ESP_ERR_NOT_FOUND:
                    default:
                    {
                        // Reset the machine to the initialization state for a fresh start
                        ESP_ERROR_CHECK_WITHOUT_ABORT(err);

                        // Sets state to STATE_INIT
                        ret_transition_err = fsm_set_state(STATE_INIT);
                        if(ret_transition_err != ESP_OK)
                        {
                            panic_dev_restart(LOW_DELAY_TICK_MS, ret_transition_err);
                        }

                        break;
                    }
                }
            }

            break;
        }
        // Periodic delay to manage state machine execution frequency
        vTaskDelay(pdMS_TO_TICKS(DELAY_HW_STABILIZE_MS));
    }
}

// Used for restart ESP32 completely and log error
void panic_dev_restart(TickType_t ms, esp_err_t error_ret)
{   
    if(ms > 0)
    {
        vTaskDelay(pdMS_TO_TICKS(ms));    
    }

    ESP_LOGE(fsm_tag, "PANIC Restart error: %s ", esp_err_to_name(error_ret));
    esp_restart();

    // Fallback: prevent returning if restart sequence fails
    for(;;){}; 
}

// Responsible for the task creation
void fsm_init(void)
{   
    ping_queue = xQueueCreate(PING_QUEUE_ITEM_SIZE, sizeof(ping_result_t));

    xTaskCreate(vTaskFSM, V_FSM_TASK_NAME, V_FSM_STACK_BUFFER, NULL, tskIDLE_PRIORITY, NULL);
}

/*
    Performs a bitwise AND operation with a mask of 1 to isolate the shifted bit,
    returning its boolean state.
*/
uint8_t bitwise_nav(const uint8_t *bitmask_state, system_state_t c_state, system_state_t n_state)
{
    return (bitmask_state[c_state] >> n_state) & 1;
}

// Sets the state of the FSM
esp_err_t fsm_set_state(system_state_t new_state)
{   
    uint8_t bitwise_op = bitwise_nav(state_bitmask, current_state, new_state);

    if(bitwise_op)
    {   
        current_state = new_state;
        ESP_LOGI(fsm_tag, "State changed to: %s", state_to_name(current_state));
        
        return ESP_OK;
    }
    else
    {
        // Log illegal transition attempt based on the bitmask
        ESP_LOGE(
            fsm_tag,
            "Cannot change the state: %s to %s", 
            state_to_name(current_state),
            state_to_name(new_state)
        );

        return ESP_FAIL;
    }
}

// Gets the state of the FSM
system_state_t fsm_get_state(void)
{
    return current_state;
}

// Retrieves the name associated with a specific state type
const char *state_to_name(system_state_t state)
{
    switch(state)
    {
        case STATE_INIT:                return "STATE_INIT";
        case STATE_WIFI_CONNECTING:     return "STATE_WIFI_CONNECTING";
        case STATE_PROVISIONING:        return "STATE_PROVISIONING";
        case STATE_MQTT_CONNECTING:     return "STATE_MQTT_CONNECTING";
        case STATE_OPERATIONAL_ONLINE:  return "STATE_OPERATIONAL_ONLINE";
        case STATE_OPERATIONAL_OFFLINE: return "STATE_OPERATIONAL_OFFLINE";
        case STATE_SYNCING:             return "STATE_SYNCING";
        case STATE_ERROR:               return "STATE_ERROR";
        default:                        return "STATE_UNKNOWN";
    }
}