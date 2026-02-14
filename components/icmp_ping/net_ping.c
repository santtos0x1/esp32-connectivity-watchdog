#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "ping/ping_sock.h"
#include "lwip/inet.h"
#include "esp_log.h"

#include "net_ping.h"

/* Using a direct IP to bypass DNS resolution issues during network failure */
#define PING_GLOBAL_IP_SERVER "8.8.8.8"
#define MAX_SEND_ICMP_PACKETS 3

static const char *ping_tag = "ping";

esp_ping_handle_t hdl;

//TODO: Implement led blinking if receive packets
void cmd_ping_on_ping_success(esp_ping_handle_t hdl, void *args)
{
    esp_err_t err_ttl, err_seq, err_targ, err_gap, err_size;

    uint8_t ttl;
    uint16_t seqno;
    uint32_t elapsed_time, recv_len;
    ip_addr_t target_addr;

    /* Extracting specific packet information for real-time monitoring */
    
    // 1. Get Time To Live (TTL)
    err_ttl = esp_ping_get_profile(
        hdl, 
        ESP_PING_PROF_TTL, 
        &ttl, 
        sizeof(ttl)
    );
    if (err_ttl != ESP_OK) {
        ESP_LOGE(
            ping_tag, 
            "Failed to get TTL: %s", 
            esp_err_to_name(err_ttl)
        );
    }

    // 2. Get Sequence Number
    err_seq = esp_ping_get_profile(
        hdl, 
        ESP_PING_PROF_SEQNO, 
        &seqno, 
        sizeof(seqno)
    );
    if (err_seq != ESP_OK) {
        ESP_LOGE(
            ping_tag, 
            "Failed to get sequence number: %s", 
            esp_err_to_name(err_seq)
        );
    }

    // 3. Get Target IP Address
    err_targ = esp_ping_get_profile(
        hdl, 
        ESP_PING_PROF_IPADDR, 
        &target_addr, 
        sizeof(target_addr)
    );
    if (err_targ != ESP_OK) {
        ESP_LOGE(
            ping_tag, 
            "Failed to get target address: %s", 
            esp_err_to_name(err_targ)
        );
    }

    // 4. Get Elapsed Time (RTT) for this specific packet
    err_gap = esp_ping_get_profile(
        hdl, 
        ESP_PING_PROF_TIMEGAP, 
        &elapsed_time, 
        sizeof(elapsed_time)
    );
    if (err_gap != ESP_OK) {
        ESP_LOGE(
            ping_tag, 
            "Failed to get elapsed time: %s",
            esp_err_to_name(err_gap)
        );
    }

    // 5. Get Received Data Length
    err_size = esp_ping_get_profile(
        hdl, 
        ESP_PING_PROF_SIZE, 
        &recv_len, 
        sizeof(recv_len)
    );
    if (err_size != ESP_OK) {
        ESP_LOGE(
            ping_tag, 
            "Failed to get receive length: %s", 
            esp_err_to_name(err_size)
        );
    }
}

void cmd_ping_on_ping_timeout(esp_ping_handle_t hdl, void *args)
{
    esp_err_t err_seq, err_targ;

    uint16_t seqno;
    ip_addr_t target_addr;

    /* Fetch sequence number to identify which specific packet failed */
    err_seq = esp_ping_get_profile(
        hdl, 
        ESP_PING_PROF_SEQNO, 
        &seqno, 
        sizeof(seqno)
    );
    if(err_seq != ESP_OK)
    {
        ESP_LOGE(
            ping_tag, 
            "Failed to get sequence number(seqno): %s", 
            esp_err_to_name(err_seq)
        );
    }

    err_targ = esp_ping_get_profile(
        hdl, 
        ESP_PING_PROF_IPADDR,
        &target_addr, 
        sizeof(target_addr)
    );
    if(err_targ != ESP_OK)
    {
        ESP_LOGE(
            ping_tag, 
            "Failed to get target address: %s", 
            esp_err_to_name(err_targ)
        );
    }
}

void cmd_ping_end(esp_ping_handle_t hdl, void *args)
{
    esp_err_t err_req, err_rep, err_dur;

    /* Retrieve the Queue handle passed via 'cb_args' */
    QueueHandle_t p_queue = (QueueHandle_t)args;
    ping_result_t report;

    /* Collect session statistics: transmitted vs received packets */
    // Get total number of ICMP packets sent
    err_req = esp_ping_get_profile(
        hdl, 
        ESP_PING_PROF_REQUEST, 
        &report.transmitted, 
        sizeof(report.transmitted)
    );
    if (err_req != ESP_OK) {
        ESP_LOGE(ping_tag, "Failed to get transmitted count");
        report.transmitted = 0;
    }

    // Get total number of ICMP replies received
    err_rep = esp_ping_get_profile(
        hdl, 
        ESP_PING_PROF_REPLY, 
        &report.received, 
        sizeof(report.received)
    );
    if (err_rep != ESP_OK) {
        ESP_LOGE(ping_tag, "Failed to get received count");
        report.received = 0;
    }

    // Get total elapsed time for the entire ping session
    err_dur = esp_ping_get_profile(
        hdl, 
        ESP_PING_PROF_DURATION, 
        &report.total_time_ms, 
        sizeof(report.total_time_ms)
    );
    if (err_dur != ESP_OK) {
        ESP_LOGE(ping_tag, "Failed to get session duration");
        report.total_time_ms = 0;
    }

    /* Send the consolidated report to the FSM/Watchdog via Queue */
    if(p_queue != NULL)
    {
        xQueueSend(p_queue, &report, 0);
    }

    esp_ping_delete_session(hdl);
}

esp_err_t initialize_ping(QueueHandle_t result_q)
{
    esp_err_t err;

    esp_ping_callbacks_t cbs;
    
    /* Assigning the callback functions and passing the Queue handle as context */
    cbs.on_ping_success = cmd_ping_on_ping_success;
    cbs.on_ping_timeout = cmd_ping_on_ping_timeout;
    cbs.on_ping_end = cmd_ping_end;
    cbs.cb_args = result_q;

    ip_addr_t target_addr;
    struct in_addr addr4;
    
    /* Convert string IP to network format and cast into the LwIP structure */
    inet_aton(PING_GLOBAL_IP_SERVER, &addr4);
    ip_2_ip4(&target_addr)->addr = addr4.s_addr; 
    target_addr.type = IPADDR_TYPE_V4;

    /* Apply configurations to the ping session */
    esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
    ping_config.target_addr = target_addr;
    ping_config.count = MAX_SEND_ICMP_PACKETS;
    
    err = esp_ping_new_session(&ping_config, &cbs, &hdl);
    if(err != ESP_OK)
    {
        ESP_LOGE(ping_tag, "Failed to start a new ping session: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_ping_start(hdl);
    if(err != ESP_OK)
    {
        ESP_LOGE(ping_tag, "Failed to start ping: %s", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}