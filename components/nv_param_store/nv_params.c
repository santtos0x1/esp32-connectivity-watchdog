#include "sdkconfig.h"
#include <stdlib.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "nv_params.h"

// Sets information for set_wf function and debugging
static nvs_handle_t nvHandle;
static const char* nvs_tag = "NVS"; 

esp_err_t init_nvs_storage( void )
{
    // Starts the NVS storage in flash memory 
    esp_err_t ret = nvs_flash_init();
    
    if(ret == ESP_ERR_NO_MEM || ret == ESP_ERR_INVALID_VERSION)
    {
        // If memory is full or the version is invalid, the NVS partition will be erased.
        ESP_LOGE(nvs_tag, "Cannot init NVS, no memory or invalid version.");
        ESP_ERROR_CHECK(ret);

        ESP_LOGI(nvs_tag, "Erasing flash memory...");
        ret = nvs_flash_erase();
    }

    // Return the init status
    return ret;
}
/*
    * Stores WiFi configuration in the NVS partition for future boots.
*/
void set_wf_params_nv_storage( void )
{
    wifi_config_data_t wifi_data;

    esp_err_t init = init_nvs_storage();
    esp_err_t get_params = get_wf_params_nv_storage(&wifi_data);
    
    if(init == ESP_OK && get_params == ESP_OK)
    {
        ESP_LOGI( nvs_tag, "NVS Storage started sucessfully, setting WiFi params" );
        nvs_open( "wifi_params", NVS_READWRITE, &nvHandle );

        nvs_set_str( nvHandle, "ssid_param", wifi_data.ssid );
        nvs_set_str( nvHandle, "pass_param", wifi_data.pass );

        // Commits the information on NVS
        nvs_commit(nvHandle);
    }
}
/*
    * Fetches persisted WiFi configuration from non-volatile storage.
*/
 esp_err_t get_wf_params_nv_storage( wifi_config_data_t * config )
{
    esp_err_t err;

    size_t ssid_size = MAX_SSID_LEN;
    size_t pass_size = MAX_PASS_LEN;
    
    // Gets the SSID in NVS or get default value
    err = nvs_get_str( nvHandle, "ssid_param", config->ssid, &ssid_size );
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        strncpy(config->ssid, CONFIG_WIFI_SSID, MAX_SSID_LEN);
    }

    // Gets the pass in NVS or get default value
    err = nvs_get_str( nvHandle, "pass_param", config->ssid, &pass_size );
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        strncpy(config->pass, CONFIG_WIFI_SSID, MAX_SSID_LEN);
    }

    // Closes the NVS
    nvs_close(nvHandle);
    return ESP_OK;
}
