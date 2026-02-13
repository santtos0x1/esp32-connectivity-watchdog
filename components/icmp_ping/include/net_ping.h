#ifndef NET_PING_H
#define NET_PING_H

#include "ping/ping_sock.h"
#include "lwip/inet.h"
#include "esp_log.h"

typedef struct {
    uint32_t transmitted;
    uint32_t received;
    uint32_t total_time_ms;
} ping_result_t;

extern ping_result_t report_global;

void cmd_ping_on_ping_success(esp_ping_handle_t hdl, void *args);

void cmd_ping_on_ping_timeout(esp_ping_handle_t hdl, void *args);

void cmd_ping_end(esp_ping_handle_t hdl, void *args);

void initialize_ping(QueueHandle_t result_q);

#endif // !NET_PING_H