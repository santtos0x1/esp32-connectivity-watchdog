#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_log.h"

#include "sys_boot.h"

static uint32_t free_heap_size;
static uint32_t free_internal_heap_size;

static uint16_t MINOR_VERSION;
static uint16_t MAJOR_VERSION;

static const char * boot_tag = "boot";
static const char * features_tag = "boot-features";

typedef struct {
    uint32_t mask;
    const char* name;
} feature_name_t;

/**
 * Hardware features lookup table.
 * * Maps bitmask flags (CHIP_FEATURE_*) to human-readable strings for
 * system diagnostics and boot-up reporting.
 */
static const feature_name_t features_map[ 6 ] = {
    { CHIP_FEATURE_EMB_FLASH,  "Embedded flash memory" },
    { CHIP_FEATURE_WIFI_BGN,   "2.4Ghz WiFi" },
    { CHIP_FEATURE_BLE,        "Bluetooth Low Energy" },
    { CHIP_FEATURE_BT,         "Bluetooth classic" },
    { CHIP_FEATURE_IEEE802154, "IEEE 802.15.4" },
    { CHIP_FEATURE_EMB_PSRAM,  "Embedded psram" }
};

esp_chip_info_t chip_info;

// Displays the system splash screen and branding.
static void boot_logo( void )
{
    ESP_LOGI(boot_tag, "======================================");
    ESP_LOGI(boot_tag, "  NS MONITOR - Connectivity watchdog ");
    ESP_LOGI(boot_tag, "======================================");
}

// Monitors both total and internal memory regions.
void esp_heap_info_internal( void )
{
    free_heap_size = esp_get_free_heap_size();
    free_internal_heap_size = esp_get_free_internal_heap_size();
    
    ESP_LOGI( boot_tag, "Free heap size: %d", free_heap_size );
    ESP_LOGI( boot_tag, "Free internal heap size: %d", free_internal_heap_size );
}

// Retrieves and logs detailed SoC hardware information.
void esp_chip_info_internal( void )
{
    esp_chip_info( &chip_info );
    
    unsigned long features_map_size = sizeof( features_map ) / sizeof( features_map[ 0 ] );

    esp_chip_model_t chip_model = chip_info.model;
    uint32_t         features   = chip_info.features;
    uint16_t         revision   = chip_info.revision;
    uint8_t          cores      = chip_info.cores;

    MINOR_VERSION = revision / 100;
    MAJOR_VERSION = revision % 100;
    
    switch(chip_model)
    {
        case CHIP_ESP32:
        {
            ESP_LOGI( boot_tag, "Chip model: ESP32" );
            break;
        }
        case CHIP_ESP32S2:
        {
            ESP_LOGI( boot_tag, "Chip model: ESP32-S2" );
            break;
        }
        case CHIP_ESP32S3:
        {
            ESP_LOGI( boot_tag, "Chip model: ESP32-S3" );
            break;
        }
        case CHIP_ESP32C3:
        {
            ESP_LOGI( boot_tag, "Chip model: ESP32-C3" );
            break;
        }
        case CHIP_ESP32C2:
        {
            ESP_LOGI( boot_tag, "Chip model: ESP32-C2" );
            break;
        }
        case CHIP_ESP32C6:
        {
            ESP_LOGI( boot_tag, "Chip model: ESP32-C6" );
            break;
        }
        case CHIP_ESP32H2:
        {
            ESP_LOGI( boot_tag, "Chip model: ESP32-H2" );
            break;
        }
        case CHIP_ESP32P4:
        {
            ESP_LOGI( boot_tag, "Chip model: ESP32-P4" );
            break;
        }
        case CHIP_ESP32C61:
        {
            ESP_LOGI( boot_tag, "Chip model: ESP32-C61" );
            break;  
        }
        case CHIP_ESP32C5:
        {
            ESP_LOGI( boot_tag, "Chip model: ESP32-C5" );
            break;  
        }
        case CHIP_ESP32H21:
        {
            ESP_LOGI( boot_tag, "Chip model: ESP32-H21" );
            break;
        }
        case CHIP_ESP32H4:
        {
            ESP_LOGI( boot_tag, "Chip model: ESP32-H4" );
            break;
        }
        case CHIP_POSIX_LINUX:
        {
            ESP_LOGI( boot_tag, "Chip model: ESP32-POSIX/Linux" );
            break;
        }
        default:
        {
            ESP_LOGI( boot_tag, "Chip model: Undefined" );
            break;
        }
    }
    

    ESP_LOGI( "", "Features: " );
    for( uint8_t i = 0; i < features_map_size; i++ )
    {
        if( features & features_map[ i ].mask )
        {
            ESP_LOGI( features_tag, " > %s", features_map[ i ].name );
        }
    }
    
    ESP_LOGI( boot_tag, "Silicon revision: v%d.%d", MAJOR_VERSION, MINOR_VERSION );
    ESP_LOGI( boot_tag, "Core number: %d", cores );
}

// Executes the full system diagnostic and identification sequence.
void esp_show_info( void )
{
    boot_logo();
    esp_heap_info_internal();
    esp_chip_info_internal();
}
