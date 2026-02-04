#include "sdkconfig.h"

#include <stdlib.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "sys_conf.h"
#include "nv_params.h"
#include "param_validation.h"

static nvs_handle_t nvHandle;

const static size_t nvs_data_size[] = { MAX_SSID_LEN, MAX_PASS_LEN, MAX_BSSID_LEN };
const static char * nvs_params[] = { NVS_PARAM_SSID, NVS_PARAM_PASSWORD, NVS_PARAM_BSSID };

const uint8_t arr_size = sizeof( nvs_data_size ) / sizeof( nvs_data_size[ 0 ] );

static const char * integrity_tag = "integrity";

// Initialize the result value
static uint8_t op_result = 0b00000000;
nvs_stats_t nvs_stats;

esp_err_t err = ESP_OK;

/*
    * Validate NVS parameters to ensure all required configuration fields are populated
*/
esp_err_t nvs_check_params( wifi_config_data_t * config )
{
    char * nvs_data[ arr_size ] = {};
    char * nvs_config_data[] = { config->ssid, config->pass, config->bssid };

    nvs_open( NVS_PARTITION_NAMESPACE, NVS_READONLY, &nvHandle );

    for( uint8_t i = 0 ; i < arr_size ; i++ )
    {
        // Retrieves stored values from NVS and validates them against expected value.
        err = nvs_get_str( nvHandle, nvs_params[ i ], nvs_data[ i ], &nvs_data_size[ i ] );
        if( err != ESP_OK )
        {
            return err;
        }

        if( strcmp( nvs_data[ i ], nvs_config_data[ i ] ) == 0 )
        {
            op_result |= (1 << i);
        }
    }

    if( op_result != 0b00000111 ) 
    {
        return ESP_FAIL;
    }

    return ESP_OK;
}