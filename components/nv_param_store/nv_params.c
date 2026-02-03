#include "sdkconfig.h"
#include <stdlib.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "nv_params.h"
#include "sys_conf.h"

// Sets information for set_wf function and debugging
static nvs_handle_t nvHandle;
static const char* nvs_tag = "nvs"; 

esp_err_t init_nvs( void )
{
    esp_err_t err = ESP_OK;

    // Starts the NVS storage in flash memory 
    err |= nvs_flash_init();
    
    if( err != ESP_OK )
    {
        ESP_ERROR_CHECK( err );
        err = nvs_flash_erase();
    }

    // Return the init status
    return err;
}

/*
    * Fetches persisted WiFi configuration from non-volatile storage.
*/
 esp_err_t get_wf_params_nvs( wifi_config_data_t * config )
{
    esp_err_t err;

    size_t ssid_size = MAX_SSID_LEN;
    size_t pass_size = MAX_PASS_LEN;
    size_t bssid_size = MAX_BSSID_LEN;
    
    // Gets the SSID in NVS or get default value
    err = nvs_get_str( nvHandle, NVS_PARAM_SSID, config->ssid, &ssid_size );
    if ( err == ESP_ERR_NVS_NOT_FOUND )
    {
        strncpy( config->ssid, CONFIG_WIFI_SSID, MAX_SSID_LEN );
    }

    // Gets the pass in NVS or get default value
    err = nvs_get_str( nvHandle, NVS_PARAM_PASSWORD, config->pass, &pass_size );
    if ( err == ESP_ERR_NVS_NOT_FOUND )
    {
        strncpy( config->pass, CONFIG_WIFI_SSID, MAX_SSID_LEN );
    }

    // Gets the bssid in NVS or get default value
    err = nvs_get_str( nvHandle, NVS_PARAM_BSSID, config->bssid, &bssid_size );
    if ( err == ESP_ERR_NVS_NOT_FOUND )
    {
        strncpy( config->bssid, CONFIG_WIFI_BSSID, MAX_BSSID_LEN );
    }

    // Closes the NVS
    nvs_close( nvHandle );
    
    return ESP_OK;
}

/*
    * Stores WiFi configuration in the NVS partition for future boots.
*/
esp_err_t set_wf_params_nvs( void )
{
    wifi_config_data_t wifi_data;
    esp_err_t err = ESP_OK;

    err |= get_wf_params_nvs( &wifi_data );
    nvs_open( NVS_PARTITION_NAMESPACE, NVS_READWRITE, &nvHandle );    
    if( err != ESP_OK )
    {
        ESP_LOGI( nvs_tag, "Failed to initialize NVS or failed to get params!" );
        return err;
    }

    ESP_LOGI( nvs_tag, "NVS initialized successfully; setting WiFi parameters." );
    
    nvs_set_str( nvHandle, NVS_PARAM_SSID, wifi_data.ssid );
    nvs_set_str( nvHandle, NVS_PARAM_PASSWORD, wifi_data.pass );
    nvs_set_str( nvHandle, NVS_PARAM_BSSID, wifi_data.bssid );
    
    // Commits the information on NVS
    nvs_commit( nvHandle );
    
    // Closes the NVS
    nvs_close( nvHandle );

    return ESP_OK;
}