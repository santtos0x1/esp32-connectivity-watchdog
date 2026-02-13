#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "ping/ping_sock.h"
#include "lwip/inet.h"
#include "esp_log.h"

#include "net_ping.h"

//TODO: Implement led blinking if receive packets
void cmd_ping_on_ping_success(esp_ping_handle_t hdl, void *args)
{
    uint8_t ttl;
    uint16_t seqno;
    uint32_t elapsed_time, recv_len;
    
    ip_addr_t target_addr;

    esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
    esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
}

void cmd_ping_on_ping_timeout(esp_ping_handle_t hdl, void *args)
{
    uint16_t seqno;
    ip_addr_t target_addr;

    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
}

void cmd_ping_end(esp_ping_handle_t hdl, void *args)
{
    QueueHandle_t q = (QueueHandle_t)args;

    ping_result_t report;

    esp_ping_get_profile(
        hdl, 
        ESP_PING_PROF_REQUEST, 
        &report.transmitted, 
        sizeof(report.transmitted)
    );
    
    esp_ping_get_profile(
        hdl, 
        ESP_PING_PROF_REPLY, 
        &report.received, 
        sizeof(report.received)
    );

    esp_ping_get_profile(
        hdl, 
        ESP_PING_PROF_DURATION, 
        &report.total_time_ms, 
        sizeof(report.total_time_ms)
    );

    if(q != NULL)
    {
        xQueueSend(q, &report, 0);
    }
}

void initialize_ping(QueueHandle_t result_q)
{
    esp_ping_callbacks_t cbs;
    
    cbs.on_ping_success = cmd_ping_on_ping_success;
    cbs.on_ping_timeout = cmd_ping_on_ping_timeout;
    cbs.on_ping_end = cmd_ping_end;
    cbs.cb_args = result_q;

    ip_addr_t target_addr;
    struct in_addr addr4;
    inet_aton("8.8.8.8", &addr4);
    ip_2_ip4(&target_addr)->addr = addr4.s_addr; 
    target_addr.type = IPADDR_TYPE_V4;

    esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
    ping_config.target_addr = target_addr;
    ping_config.count = 3;
}