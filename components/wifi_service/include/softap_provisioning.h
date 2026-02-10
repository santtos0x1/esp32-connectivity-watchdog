#ifndef SOFTAP_PROVISIONING_H
#define SOFTAP_PROVISIONING_H

#include "esp_event.h"

esp_err_t init_mdns(void);

esp_err_t init_provisioning(void);

esp_err_t root_callback(uint32_t session_id, const uint8_t *in_data, ssize_t in_len,
                        uint8_t **out_data, ssize_t *out_len, void *priv_data);

void provisioning_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                                void *event_data);

#endif // !SOFTAP_PROVISIONING_H