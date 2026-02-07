#include "sdkconfig.h"

#include <stdlib.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "sys_conf.h"
#include "nv_params.h"
#include "param_validation.h"

static nvs_handle_t nvHandle;

char * nvs_params[] = { NVS_PARAM_SSID, NVS_PARAM_PASSWORD };

static const char * integrity_tag = "integrity";

nvs_stats_t nvs_stats;

/*
    * Validate NVS parameters to ensure all required configuration fields are populated
*/
esp_err_t nvs_check_params( wifi_config_data_t * config )
{
    esp_err_t err;
    
    // Initialize the result value
    static uint8_t op_result = 0x00;

    char nvs_buff[ 64 ];
    char * nvs_config_data[] = { config->ssid, config->pass };

    nvs_open( NVS_PARTITION_NAMESPACE, NVS_READONLY, &nvHandle );

    for( uint8_t i = 0 ; i < 2 ; i++ )
    {
        size_t read_length;
        read_length = sizeof( nvs_buff );
        
        // Retrieves stored values from NVS and validates them against expected value.
        err = nvs_get_str( nvHandle, nvs_params[ i ], nvs_buff, &read_length );
        if( err != ESP_OK )
        {
            ESP_LOGE( integrity_tag, "Failed to retrieve storage data from NVS: %s", esp_err_to_name( err ) );
            nvs_close( nvHandle );
    
            return err;
        }

        if( strcmp( nvs_buff, nvs_config_data[ i ] ) == 0 )
        {
            op_result |= ( 1 << i );
        }

        nvs_buff[ 0 ] = '\0';
    }
    
    if( op_result != 0x03 )
    {
        ESP_LOGE( integrity_tag, "Integrity check failed: values mismatch!" );
        nvs_close( nvHandle );

        return ESP_FAIL;
    }
    
    nvs_close( nvHandle );
    
    return ESP_OK;
}