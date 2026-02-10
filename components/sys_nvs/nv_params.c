#include "sdkconfig.h"
#include <stdlib.h>

#include "nvs_flash.h"
#include "esp_log.h"
#include "nv_params.h"
#include "sys_conf.h"

static const char* nvs_tag = "nvs"; 

esp_err_t init_nvs(void)
{
    esp_err_t err;

    // Starts the NVS storage in flash memory 
    err = nvs_flash_init();
    if(err != ESP_OK)
    {
        ESP_LOGE(nvs_tag, "Failed to init NVS flash memory: %s", esp_err_to_name(err));
        ESP_ERROR_CHECK(err);
        err = nvs_flash_erase();
    }

    // Return the init status
    return ESP_OK;
}