#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_log.h"
#include "utils.h"

#include "sys_boot.h"

static uint32_t free_heap_size;
static uint32_t free_internal_heap_size;

static uint16_t MAJOR_VERSION;
static uint16_t MINOR_VERSION;

static char *boot_tag = "boot";
static char *features_tag = "boot-features";

typedef struct {
    uint32_t mask;
    const char* name;
} feature_name_t;

/**
 * Hardware features lookup table.
 * Maps bitmask flags (CHIP_FEATURE_*) to human-readable strings for
 * system diagnostics and boot-up reporting.
 */
static const feature_name_t features_map[6] = {
    {CHIP_FEATURE_EMB_FLASH,  "Embedded flash memory"},
    {CHIP_FEATURE_WIFI_BGN,   "2.4Ghz WiFi"},
    {CHIP_FEATURE_BLE,        "Bluetooth Low Energy"},
    {CHIP_FEATURE_BT,         "Bluetooth classic"},
    {CHIP_FEATURE_IEEE802154, "IEEE 802.15.4"},
    {CHIP_FEATURE_EMB_PSRAM,  "Embedded psram"}
};

/*
// Displays the system splash screen and branding.
void boot_logo(void)
{
    ESP_LOGI(boot_tag, "======================================");
    ESP_LOGI(boot_tag, "  NS MONITOR - Connectivity watchdog ");
    ESP_LOGI(boot_tag, "======================================");
}
*/

// Monitors both total and internal memory regions.
void esp_heap_info_internal(void)
{
    free_heap_size = esp_get_free_heap_size();
    free_internal_heap_size = esp_get_free_internal_heap_size();
    
    ESP_LOGI(boot_tag, "Free heap size: %d", free_heap_size);
    ESP_LOGI(boot_tag, "Free internal heap size: %d", free_internal_heap_size);
}

// Retrieves and logs detailed SoC hardware information.
void esp_chip_info_internal(void)
{
    // Chip config struct
    static esp_chip_info_t chip_info;
    
    esp_chip_info(&chip_info);
    
    static const unsigned long features_map_size = sizeof(features_map) / sizeof(features_map[0]);

    const esp_chip_model_t chip_model = chip_info.model;
    const uint32_t features = chip_info.features;
    const uint16_t revision = chip_info.revision;
    const uint8_t cores = chip_info.cores;

    // Silicon revision
    MINOR_VERSION = revision / 100;
    MAJOR_VERSION = revision % 100;

    static const char *c_model = GET_MCU_CHIP_NAME(chip_model);

    ESP_LOGI(boot_tag, "%s", c_model);
    
    ESP_LOGI("", "Features: ");
    for(uint8_t i = 0; i < features_map_size; i++)
    {
        if(features & features_map[i].mask)
        {
            ESP_LOGI(features_tag, " > %s", features_map[i].name);
        }
    }
    
    ESP_LOGI(boot_tag, "Silicon revision: v%d.%d", MAJOR_VERSION, MINOR_VERSION);
    ESP_LOGI(boot_tag, "Core number: %d", cores);
}

// Executes the full system diagnostic and identification sequence.
void esp_show_info(void)
{
    // boot_logo();
    esp_heap_info_internal();
    esp_chip_info_internal();
}