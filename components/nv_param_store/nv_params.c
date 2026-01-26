#include "sdkconfig.h"
#include <stdlib.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "nv_params.h"

static nvs_handle_t nvHandle;
static const char SSID[ 32 ] = CONFIG_WIFI_SSID;
static const char PASS[ 64 ] = CONFIG_WIFI_PASSWORD;

static const char* nvs_tag = "NVS"; 



static esp_err_t init_nvs_storage( void )
{ 
    esp_err_t ret = nvs_flash_init();
    
    if(ret == ESP_ERR_NO_MEM || ret == ESP_ERR_INVALID_VERSION)
    {
        ESP_LOGE(nvs_tag, "Cannot init NVS, no memory or invalid version.");
        ESP_ERROR_CHECK(ret);

        ESP_LOGI(nvs_tag, "Erasing flash memory...");
        ret = nvs_flash_erase();
    }

    return ret;
}

void set_wf_params_nv_storage( void )
{
    esp_err_t init = init_nvs_storage();
    
    if(init == ESP_OK)
    {
        ESP_LOGI(nvs_tag, "NVS Storage started sucessfully, setting WiFi params");
        nvs_open( "wifi_params", NVS_READWRITE, &nvHandle );

        nvs_set_str( nvHandle, "ssid_param", SSID );
        nvs_set_str( nvHandle, "pass_param", PASS );

        nvs_commit(nvHandle);
    }
}


esp_err_t get_wf_params_nv_storage( wifi_config_data_t * config )
{
    esp_err_t err;
    size_t ssid_size = MAX_SSID_LEN;
    size_t pass_size = MAX_PASS_LEN;
    
    err = nvs_get_str( nvHandle, "ssid_param", config->ssid, &ssid_size );
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        strncpy(config->ssid, CONFIG_WIFI_SSID, MAX_SSID_LEN);
    }

    err = nvs_get_str( nvHandle, "pass_param", config->ssid, &pass_size );
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        strncpy(config->pass, CONFIG_WIFI_SSID, MAX_SSID_LEN);
    }

    nvs_close(nvHandle);
    return ESP_OK;
}
