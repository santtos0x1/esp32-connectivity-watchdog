#include "sdkconfig.h"

#include <stdio.h>

#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "mqtt_service.h"
#include "sys_conf.h"
#include "net_ping.h"

QueueHandle_t mqtt_queue;

// Defines the stack buffer for mqtt task
#ifdef CONFIG_MQTT_STACK_SIZE
    #define V_MQTT_STACK_BUFFER (uint16_t)atoi(CONFIG_MQTT_STACK_SIZE)
#else
    #define V_MQTT_STACK_BUFFER 2048
#endif

#define V_MQTT_TASK_PRIORITY 3

static const char *mqtt_tag = "NS-MQTT";

/**
 * @brief MQTT Consumer Task: Handles incoming traffic.
 * Responsible for managing MQTT subscriptions and processing 
 * received commands from the broker.
 */

/**
 * @brief FSM Producer Task: Handles outgoing traffic.
 * Manages the system state machine and triggers data publishing
 * to the broker based on state transitions.
 */

volatile bool is_mqtt_connected = false; 

void mqtt_event_handler(void *handler_args, 
                        esp_event_base_t base, 
                        int32_t event_id, 
                        void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int cmd_msg_id;

    switch ((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_CONNECTED:
        {
            ESP_LOGI(mqtt_tag, "MQTT Connected! Session established.");
            
            is_mqtt_connected = true;

            cmd_msg_id = esp_mqtt_client_subscribe(client, "v1/device/commands", 1);
            ESP_LOGI(mqtt_tag, "Subscribed to cmd topic, msg_id=%d", cmd_msg_id);

            esp_mqtt_client_publish(client, "v1/device/ping", "", 0, 1, 0);
            esp_mqtt_client_publish(client, "v1/device/status", "ONLINE", 0, 1, 0);
            
            break;
        }

        case MQTT_EVENT_DISCONNECTED:
        {
            ESP_LOGW(mqtt_tag, "MQTT Disconnected. Reconnecting logic is handled by SDK automatically.");
            
            is_mqtt_connected = false;
            
            break;
        }

        case MQTT_EVENT_DATA:
        {
            ESP_LOGI(mqtt_tag, "Received Data on topic: %.*s", event->topic_len, event->topic);
            ESP_LOGI(mqtt_tag, "Payload: %.*s\r\n", event->data_len, event->data);
            
            break;
        }

        case MQTT_EVENT_ERROR:
        {
            ESP_LOGE(mqtt_tag, "MQTT Error detected");
            
            if(event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
            {
                ESP_LOGE(
                    mqtt_tag, 
                    "Network Error: %s", 
                    strerror(event->error_handle->esp_transport_sock_errno)
                );
            }
            else if(event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED)
            {
                ESP_LOGE(
                    mqtt_tag, 
                    "Connection Refused code: 0x%x", 
                    event->error_handle->connect_return_code
                );
            }
            else
            {
                ESP_LOGE(mqtt_tag, "Unknown error type: %d", event->error_handle->error_type);
            }
            
            break;
        }

        default:
            ESP_LOGD(mqtt_tag, "Other event id:%d", event->event_id);

            break;
    }
}


void vTaskMQTT(void *pvParameters)
{   
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_MQTT_BROKER_URL,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    mqtt_message_t rx_msg;

    for(;;)
    {
        if(xQueueReceive(mqtt_queue, &rx_msg, portMAX_DELAY) == pdPASS)
        {
            int msg_id = esp_mqtt_client_publish(
                client, 
                rx_msg.topic, 
                rx_msg.payload, 
                0,
                rx_msg.qos, 
                rx_msg.retain
            );

            ESP_LOGI(mqtt_tag, "Message send to broker, ID: %d", msg_id);
        }    
    }
}

void mqtt_init(void)
{
    mqtt_queue = xQueueCreate(10, sizeof(mqtt_message_t));   

    xTaskCreate(
        vTaskMQTT, 
        V_MQTT_TASK_NAME, 
        V_MQTT_STACK_BUFFER, 
        NULL, 
        V_MQTT_TASK_PRIORITY, 
        NULL
    );
}