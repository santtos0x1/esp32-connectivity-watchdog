#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

void vTaskMQTT(void *pvParameters);
void mqtt_init(void);
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

#endif // !MQTT_SERVICE_H