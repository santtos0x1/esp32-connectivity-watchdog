#include <stdlib.h>
#include <stdint.h>

#include "esp_rom_sys.h"
#include "driver/gpio.h"

#include "sys_conf.h"
#include "patterns.h"

void success_fb(uint8_t led_pinout)
{
    for(uint8_t i = 0; i < 3; i++)
    {
        gpio_set_level(led_pinout, 1);
        esp_rom_delay_us(DELAY_UI_REFRESH_US);
        gpio_set_level(led_pinout, 0);
        esp_rom_delay_us(DELAY_UI_REFRESH_US);
    }    
}

void fail_fb(uint8_t led_pinout)
{
    for(uint8_t i = 0; i < 10; i++) 
    {
        gpio_set_level(led_pinout, 1);
        esp_rom_delay_us(DELAY_HW_STABILIZE_US);
        gpio_set_level(led_pinout, 0);
        esp_rom_delay_us(DELAY_HW_STABILIZE_US);
    }    
}

void boot_fb(uint8_t led_pinout)
{
    for(uint8_t i = 0; i < 2; i++)
    {
        gpio_set_level(led_pinout, 1);
        esp_rom_delay_us(DELAY_BOOT_BANNER_US);
        gpio_set_level(led_pinout, 0);
        esp_rom_delay_us(DELAY_UI_REFRESH_US);
    }    
}