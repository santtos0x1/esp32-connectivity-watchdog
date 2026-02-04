#include <stdlib.h>
#include <stdbool.h>

#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"

#include "nv_params.h"
#include "conn_mgr.h"

// Debug tags
static const char* wifi_tag = "wifi";
static const char* nal_tag  = "nal";

// Initializes the ESP-NETIF config (TCP/IP stack abstraction layer)
static wifi_config_t wifi_config = {
    .sta = {
        .bssid_set = 1,
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        .scan_method = WIFI_FAST_SCAN
    }
};

/*
    * Initializes the TCP/IP stack instance and sets WiFi to Station (STA) mode
*/
bool init_network_abstraction_layer( void )
{
    esp_err_t ret = esp_netif_init();
    if( ret != ESP_OK ) 
    {
        ESP_LOGE( nal_tag, "Netif init failed: %s", esp_err_to_name( ret ) );
        return false;
    }

    esp_netif_create_default_wifi_ap();

    esp_netif_create_default_wifi_sta();

    return true;
}

/*
    * Initializes the WiFi stack and defines station mode configurations
*/
esp_err_t init_wifi_connection( void )
{
    // Uses default initial configuration
    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT(); 

    // Creates error handlers 
    esp_err_t config_error_handler, connect_error_handler, get_params_error_handler;
    config_error_handler = ESP_OK;
    connect_error_handler = ESP_OK;
    get_params_error_handler = ESP_OK;

    // Gets the data from caller-allocated struct
    static wifi_config_data_t wifi_data;
    
    get_params_error_handler = get_wf_params_nvs( &wifi_data );
    if( get_params_error_handler != ESP_OK )
    { 
        ESP_LOGE( wifi_tag, "Cannot get WiFi data from NVS" );
    }

    // Storing data on struct from NVS
    memcpy( wifi_config.sta.ssid, wifi_data.ssid, sizeof( wifi_data.ssid ) );
    memcpy( wifi_config.sta.password, wifi_data.pass, sizeof( wifi_data.pass ) );
    memcpy( wifi_config.sta.bssid, wifi_data.bssid, sizeof( wifi_data.bssid ) );

    // Minimal radio setup with error handling
    config_error_handler |= esp_wifi_init( &init_cfg );
    config_error_handler |= esp_wifi_set_mode( WIFI_MODE_STA );
    config_error_handler |= esp_wifi_set_config( WIFI_IF_STA, &wifi_config );
    config_error_handler |= esp_wifi_set_ps( WIFI_PS_MIN_MODEM );
    config_error_handler |= esp_wifi_start();

    if( config_error_handler != ESP_OK )
    {
        ESP_LOGE( wifi_tag, "Fail on WiFi radio configuration: %s", esp_err_to_name( config_error_handler ) );
        return config_error_handler;
    }

    connect_error_handler |= esp_wifi_connect();
    if( connect_error_handler != ESP_OK )
    {
        ESP_LOGE( wifi_tag, "WiFi Connection failed: %s", esp_err_to_name( connect_error_handler ) );
        return connect_error_handler;
    }

    // If no error, return ESP_OK
    return ESP_OK;
}