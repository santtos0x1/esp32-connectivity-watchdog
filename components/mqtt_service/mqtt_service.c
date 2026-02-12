#include <stdio.h>

#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "mqtt_service.h"
#include "sys_conf.h"

//TODO: THIS TASK WILL HANDLE THE SUBSCRIPTIONS ON MQTT, THEREFORE, THE RECEIVED COMMANDS
//TODO: AND THE FSM TASK WILL HANDLE THE PUBLISHING, THEREFORE, SENDING DATA TO THE BROKER

static const char *mqtt_tag = "mqtt";

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_CONNECTED:
        {
            ESP_LOGI(mqtt_tag, "Connected to broker, creating ns/monitor/new topic.");
            
            esp_mqtt_client_publish(client, "ns/monitor/now", "Test", 0, 1, 0);
            break;
        }
        case MQTT_EVENT_DISCONNECTED:
        {
            ESP_LOGI(mqtt_tag, "Disconnecting from broker!");
            break;
        }
        case MQTT_EVENT_DATA:
        {
            ESP_LOGI(mqtt_tag, "Message received.");
            
            ESP_LOGI(mqtt_tag, "Topic: %.*s\r\n", event->topic_len, event->topic);
            ESP_LOGI(mqtt_tag, "Data: %.*s\r\n", event->data_len, event->data);
            
            break;
        }
        case MQTT_EVENT_ERROR:
        {
            ESP_LOGI(mqtt_tag, "Occurred an error on MQTT");
            break;
        }
        default:
        {
            break;
        }
    }
}

void vTaskMQTT(void *pvParameters)
{
    for(;;)
    {

    }
}

void mqtt_init(void)
{
    xTaskCreate(vTaskMQTT, "MQTT_TASK", V_MQTT_STACK_BUFFER, NULL, tskIDLE_PRIORITY, NULL);
}