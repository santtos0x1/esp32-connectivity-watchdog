#include <stdlib.h>
#include "esp_log.h"
#include "nv_params.h"
#include "conn_mgr.h"

void init_wifi_connection( void )
{
    wifi_config_data_t wifi_data;
    
    esp_err_t ret = get_wf_params_nv_storage( &wifi_data );
    if( ret == ESP_OK ) ESP_LOGI("WIFI", "WiFi credentials retrieved successfully.");
}