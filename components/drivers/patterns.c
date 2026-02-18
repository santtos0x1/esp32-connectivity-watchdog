#include <stdlib.h>
#include <stdint.h>

#include "esp_rom_sys.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "sys_conf.h"
#include "patterns.h"

static const char *patterns_tag = "NS-PATTERNS";

// Triggers a specific LED pattern to indicate hardware initialization status
esp_err_t boot_fb(uint8_t led_pinout)
{
    esp_err_t err;

    for(uint8_t i = 0; i < 3; i++)
    {
        err = gpio_set_level(led_pinout, 1);
        if(err != ESP_OK)
        {
            ESP_LOGE(patterns_tag, "Failed to set gpio level: %s", esp_err_to_name(err));

            return err;
        }
        
        esp_rom_delay_us(DELAY_BOOT_BANNER_US);

        err = gpio_set_level(led_pinout, 0);
        if(err != ESP_OK)
        {
            ESP_LOGE(patterns_tag, "Failed to set gpio level: %s", esp_err_to_name(err));

            return err;
        }
        
        esp_rom_delay_us(DELAY_UI_REFRESH_US);
    }    

    return ESP_OK;
}