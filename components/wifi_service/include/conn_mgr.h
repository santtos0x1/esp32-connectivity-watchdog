#ifndef CONN_MGR_H
#define CONN_MGR_H

bool init_network_abstraction_layer(void);
esp_err_t init_wifi_connection(void);

#endif // !CONN_MGR_H