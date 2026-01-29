#include "sys_boot.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_log.h"

static uint32_t free_heap_size;
static uint32_t free_internal_heap_size;

static const char * boot_tag = "boot";

typedef struct {
    uint32_t mask;
    const char* name;
} feature_name_t;

const feature_name_t features_map[ 6 ] = {
    { CHIP_FEATURE_EMB_FLASH,  "Embedded flash memory" },
    { CHIP_FEATURE_WIFI_BGN,   "2.4Ghz WiFi"           },
    { CHIP_FEATURE_BLE,        "Bluetooth LE"          },
    { CHIP_FEATURE_BT,         "Bluetooth classic"     },
    { CHIP_FEATURE_IEEE802154, "IEEE 802.15.4"         },
    { CHIP_FEATURE_EMB_PSRAM,  "Embedded psram"        }
};

esp_chip_info_t chip_info;

void boot_logo( void )
{
    ESP_LOGI(boot_tag, "======================================");
    ESP_LOGI(boot_tag, "  NS MONITOR - Connectivity watchdog ");
    ESP_LOGI(boot_tag, "======================================");
}

void esp_heap_info_internal( void )
{
    free_heap_size = esp_get_free_heap_size();
    free_internal_heap_size = esp_get_free_internal_heap_size();
    
    ESP_LOGI( boot_tag, "Free heap size: %d", free_heap_size );
    ESP_LOGI( boot_tag, "Free internal heap size: %d", free_internal_heap_size );
}

void esp_chip_info_internal( void )
{
    esp_chip_info( &chip_info );
    
    unsigned long features_map_size = sizeof( features_map ) / sizeof( features_map[ 0 ] );

    esp_chip_model_t chip_model = chip_info.model;
    uint32_t         features   = chip_info.features;
    uint16_t         revision   = chip_info.revision;
    uint8_t          cores      = chip_info.cores;

    uint16_t MINOR_VERSION = revision / 100;
    uint16_t MAJOR_VERSION = revision % 100;

    // TODO: VERIFY TYPE OF CHIP MODEL 
    ESP_LOGI( boot_tag, "Chip model: %d", chip_model );

    ESP_LOGI( "", "Features: " );
    for( uint8_t i = 0; i < features_map_size; i++ )
    {
        if( features & features_map[ i ].mask )
        {
            ESP_LOGI( "boot-features", " > %s", features_map[ i ].name );
        }
    }
    
    ESP_LOGI( boot_tag, "Silicon revision: v%d.%d", MAJOR_VERSION, MINOR_VERSION );
    ESP_LOGI( boot_tag, "Core number: %d", cores );
}

void esp_show_info( void )
{
    boot_logo();
    esp_heap_info_internal();
    esp_chip_info_internal();
}