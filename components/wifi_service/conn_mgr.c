#include <stdlib.h>
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nv_params.h"
#include "conn_mgr.h"

wifi_config_t wifi_config = {
    .sta = {
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
    }
};

static const char* wifi_tag = "WIFI";
static const char* nal_tag = "WIFI NAL";

void init_network_abstraction_layer( void )
{
    esp_err_t ret = esp_netif_init();
    if( ret == ESP_FAIL ) ESP_LOGE( nal_tag, "Netif init failed." );

    esp_netif_create_default_wifi_sta();
}

esp_err_t init_wifi_connection( void )
{

    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT(); 
    
    esp_err_t config_error_handler, connect_error_handler, get_params_error_handler;
    config_error_handler = ESP_OK;
    connect_error_handler = ESP_OK;
    get_params_error_handler = ESP_OK;

    wifi_config_data_t wifi_data;
    
    get_params_error_handler = get_wf_params_nv_storage( &wifi_data );
    if( get_params_error_handler != ESP_OK )
    { 
        ESP_LOGE( wifi_tag, "Cannot get WiFi data from NVS" );
    }

    ESP_LOGI( wifi_tag, "Storing WiFi data on struct" );
    memcpy( wifi_config.sta.ssid, wifi_data.ssid, sizeof( wifi_data.ssid ) );
    memcpy( wifi_config.sta.password, wifi_data.pass, sizeof( wifi_data.pass ) );

    config_error_handler |= esp_wifi_init( &init_cfg );
    config_error_handler |= esp_wifi_set_mode( WIFI_MODE_STA );
    config_error_handler |= esp_wifi_set_config( WIFI_IF_STA, &wifi_config );
    config_error_handler |= esp_wifi_set_ps( WIFI_PS_MIN_MODEM );
    config_error_handler |= esp_wifi_start();

    if( config_error_handler != ESP_OK )
    {
        ESP_LOGE( wifi_tag, "Fail on WiFi radio configuration!" );
        return config_error_handler;
    }

    connect_error_handler |= esp_wifi_connect();
    if( connect_error_handler != ESP_OK )
    {
        ESP_LOGE( wifi_tag, "WiFi connection failed!" );
        return connect_error_handler;
    }

    return ESP_OK;
}