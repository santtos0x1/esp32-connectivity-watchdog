#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

typedef struct {
    char topic[64];
    char payload[128];
    int qos;
    int retain;
} mqtt_message_t;

extern QueueHandle_t mqtt_queue;

void mqtt_init(void);

esp_err_t mqtt_start_app(void);

bool mqtt_is_connected(void);

#endif // !MQTT_SERVICE_H