#include "sdkconfig.h"

#include <stdio.h>

#include "esp_netif.h"
#include "esp_wifi.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_softap.h"
#include "esp_log.h"
#include "mdns.h"
#include "esp_http_server.h"

#include "softap_provisioning.h"

static const char *prov_tag = "prov";

// Handles background events triggered by the provisioning process
void provisioning_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if(event_base == WIFI_PROV_EVENT)
    {
        switch(event_id)
        {
            case WIFI_PROV_START:
            {
                ESP_LOGI(prov_tag, "Provisioning started!");
                
                break;
            }
            case WIFI_PROV_CRED_RECV:
            {
                // Extracts SSID from the received configuration data
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                ESP_LOGI(
                    prov_tag, 
                    "Credential received successfully. SSID: %s", 
                    (char *)wifi_sta_cfg->ssid 
                );
                
                break;
            }
            case WIFI_PROV_CRED_SUCCESS:
            {
                ESP_LOGI(prov_tag, "Successfully connected!");
                
                break;
            }
            case WIFI_PROV_END:
            {
                // Cleanup resources once provisioning is finished
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

esp_err_t root_callback(uint32_t session_id, const uint8_t *in_data, ssize_t in_len,
                        uint8_t **out_data, ssize_t *out_len, void *priv_data)
{
    ESP_LOGI(prov_tag, "/ endpoint accessed.");

    *out_data = NULL;
    *out_len = 0;

    return ESP_OK;
}


// Configures mDNS to allow the mobile app to find the device by name
esp_err_t init_mdns(void)
{   
    esp_err_t err;
    
    err = mdns_init();
    if(err != ESP_OK)
    {
        ESP_LOGE(prov_tag, "Failed to init the mDSN: %s", esp_err_to_name(err));
        
        return err;
    }

    err = mdns_hostname_set("ns-monitor-devconf");
    if(err != ESP_OK)
    {
        ESP_LOGE(prov_tag, "Failed to set hostname: %s", esp_err_to_name(err));
        
        return err;
    }

    err = mdns_instance_name_set("NS Monitor config");
    if(err != ESP_OK)
    {
        ESP_LOGE(prov_tag, "Failed to set instance name: %s", esp_err_to_name(err));
        
        return err;
    }

    return ESP_OK;
}

// Sets up and starts the SoftAP provisioning service
esp_err_t init_provisioning(void)
{
    bool provisioned = false;
    esp_err_t err;
    
    // Use SoftAP scheme (ESP32 acts as an Access Point)
    wifi_prov_mgr_config_t mgr_conf = {
        .scheme = wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
    };

    wifi_prov_mgr_is_provisioned(&provisioned);

    if(provisioned) {
        ESP_LOGI(prov_tag, "Device already provisioned. Skipping manager init.");
        return ESP_ERR_INVALID_STATE;
    }
    
    if(!provisioned)
    {
        err = wifi_prov_mgr_init(mgr_conf);
        if(err != ESP_OK)
        {
            ESP_LOGE(
                prov_tag, 
                "Failed to start provisioning manager configuration: %s", 
                esp_err_to_name(err) 
            );

            return err;
        }

        /*
        err = wifi_prov_mgr_endpoint_create("custom-data");
        if(err != ESP_OK)
        {
            ESP_LOGE(prov_tag, "Failed to create 'custom-data' endpoint: %s", esp_err_to_name(err));
            return err;
        }
        else 
        {
            err = wifi_prov_mgr_endpoint_register("custom-data", root_callback, NULL);
            if(err != ESP_OK)
            {
                ESP_LOGE(prov_tag, "Failed to register 'custom-data' endpoint");
                return err;
            }
        } 
        */

        // Start provisioning with Security 1 (requires PoP)
        err = wifi_prov_mgr_start_provisioning(
            WIFI_PROV_SECURITY_1, 
            CONFIG_WIFI_AP_PROV_POP, 
            CONFIG_WIFI_AP_PROV_SSID,
            NULL
        );
        if(err != ESP_OK)
        {
            ESP_LOGE( 
                prov_tag, 
                "Failed to start provisioning manager: %s", 
                esp_err_to_name(err) 
            );

            return err;
        }

        
    }

    return ESP_OK;
}