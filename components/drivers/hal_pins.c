#include <stdlib.h>

#include "driver/gpio.h"
#include "esp_log.h"

#include "hal_map.h"

// Configuration set
gpio_config_t io_conf_led = {
    .pin_bit_mask = ( 1ULL << FEEDBACK_LED_PIN ),
    .mode = GPIO_MODE_OUTPUT,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

/*
    * Initializes and configures the GPIO pins used by the application.
*/
esp_err_t sys_conf_gpio( void )
{
    esp_err_t err;
    
    // Configs the pin with the configuration set
    err = gpio_config( &io_conf_led );
    if( err != ESP_OK )
    {
        ESP_LOGE("", "Failed to start GPIO configuration: %s", esp_err_to_name( err ) );
            
        return err;
    }
    
    return ESP_OK;
}