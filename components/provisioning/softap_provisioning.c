#include "sdkconfig.h"

#include "softap_provisioning.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_softap.h"
#include "esp_log.h"
#include "mdns.h"

static const char * prov_tag = "prov";

void provisioning_event_handler( void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data )
{
    if( event_base == WIFI_PROV_EVENT )
    {
        switch( event_id )
        {
            case WIFI_PROV_START:
            {
                ESP_LOGI( prov_tag, "Provisioning started!" );
                break;
            }
            case WIFI_PROV_CRED_RECV:
            {
                wifi_sta_config_t * wifi_sta_cfg = ( wifi_sta_config_t * ) event_data;
                ESP_LOGI( prov_tag, "Credential received successfully. SSID: %s", ( char * )wifi_sta_cfg->ssid );
                break;
            }
            case WIFI_PROV_CRED_SUCCESS:
            {
                ESP_LOGI( prov_tag, "Successfully connected!" );
                break;
            }
            case WIFI_PROV_END:
            {
                wifi_prov_mgr_deinit();
                mdns_free();
                break;
            }
            default:
            {
                break;
            }
        }
    }
}

void init_mdns( void )
{
    esp_err_t err = mdns_init();
    if( err != ESP_OK )
    {
        ESP_LOGE( prov_tag, "Failed to init the mDSN: %s", esp_err_to_name( err ) );
        return;
    }

    mdns_hostname_set("ns-monitor-devconf");

    mdns_instance_name_set("NS Monitor config");
}

void init_provisioning( void )
{
    wifi_prov_mgr_config_t mgr_conf = {
        .scheme = wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
    };

    ESP_ERROR_CHECK( wifi_prov_mgr_init( mgr_conf ) );

    ESP_ERROR_CHECK( wifi_prov_mgr_start_provisioning(
        WIFI_PROV_SECURITY_1, 
        CONFIG_WIFI_AP_PROV_POP, 
        CONFIG_WIFI_AP_PROV_SSID,
        NULL
    ));
}