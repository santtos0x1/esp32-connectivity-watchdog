#ifndef CONN_MGR_H
#define CONN_MGR_H

esp_err_t init_network_abstraction_layer(void);
esp_err_t init_wifi_connection(void);
void wifi_status_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

#endif // !CONN_MGR_H