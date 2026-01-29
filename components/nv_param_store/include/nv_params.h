#ifndef NV_PARAMS_H
#define NV_PARAMS_H

#include "nvs_flash.h"

#define MAX_SSID_LEN  32
#define MAX_BSSID_LEN 48
#define MAX_PASS_LEN  64

typedef struct {
    char ssid[ MAX_SSID_LEN ];
    char pass[ MAX_PASS_LEN ];
    char bssid[ MAX_BSSID_LEN ];
} wifi_config_data_t;

esp_err_t get_wf_params_nv_storage( wifi_config_data_t * config );
esp_err_t init_nvs_storage( void );
void set_wf_params_nv_storage( void );

#endif // !NV_PARAMS_H