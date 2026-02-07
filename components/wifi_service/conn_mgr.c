#include <stdlib.h>
#include <stdbool.h>

#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"

#include "nv_params.h"
#include "conn_mgr.h"

// Debug tags
static const char * wifi_tag = "wifi";
static const char * nal_tag  = "nal";

// Creates error handlers 
esp_err_t err;
esp_err_t ret;

// Initializes the ESP-WIFI config
static wifi_config_t wifi_config = {
    .sta = {
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        .scan_method = WIFI_FAST_SCAN
    }
};

//Initializes the TCP/IP stack instance and sets WiFi to Station (STA) mode
bool init_network_abstraction_layer( void )
{
    ret = esp_netif_init();
    if( ret != ESP_OK ) 
    {
        ESP_LOGE( nal_tag, "Netif init failed: %s", esp_err_to_name( ret ) );
        return ret;
    }

    // Start AP for provisioning
    esp_netif_create_default_wifi_ap();

    esp_netif_create_default_wifi_sta();

    return ESP_OK;
}

/*
    * Initializes the WiFi stack and defines station mode configurations
*/
esp_err_t init_wifi_connection( void )
{
    // Gets the data from caller-allocated struct
    static wifi_config_data_t wifi_data;
    
    err = get_wf_params_nvs( &wifi_data );
    if( err != ESP_OK )
    { 
        ESP_LOGE( wifi_tag, "Failed to get WiFi data from NVS: %s", esp_err_to_name( err ) );
        return err;
    }

    // Cleaning the buffer before store data
    memset(wifi_config.sta.ssid, 0, sizeof(wifi_config.sta.ssid));
    memset(wifi_config.sta.password, 0, sizeof(wifi_config.sta.password));

    // Storing data on struct from NVS
    memcpy( wifi_config.sta.ssid, wifi_data.ssid, sizeof( wifi_data.ssid ) );
    memcpy( wifi_config.sta.password, wifi_data.pass, sizeof( wifi_data.pass ) );

    // Minimal radio setup with error handling
    err = esp_wifi_set_mode( WIFI_MODE_STA );
    if( err != ESP_OK )
    {
        ESP_LOGE( wifi_tag, "Failed to set mode STA: %s", esp_err_to_name( err ) );
        return err;
    }

    err = esp_wifi_set_config( WIFI_IF_STA, &wifi_config );
    if( err != ESP_OK )
    {
        ESP_LOGE( wifi_tag, "Failed to set configuration mode STA: %s", esp_err_to_name( err ) );
        return err;
    }

    err = esp_wifi_start();
    if( err != ESP_OK && err != ESP_ERR_WIFI_STATE)
    {
        ESP_LOGE( wifi_tag, "Failed to start WiFi: %s", esp_err_to_name( err ) );
        return err;
    }

    err = esp_wifi_set_ps( WIFI_PS_MIN_MODEM );
    if( err != ESP_OK )
    {
        ESP_LOGE( wifi_tag, "Failed to set ps mode: %s", esp_err_to_name( err ) );
        return err;
    }

    err = esp_wifi_connect();
    if( err != ESP_OK )
    {
        ESP_LOGE( wifi_tag, "WiFi Connection failed: %s", esp_err_to_name( err ) );
        return err;
    }

    ESP_LOGI( wifi_tag, "Successfully Connected in network!" );
    
    return ESP_OK;
}
