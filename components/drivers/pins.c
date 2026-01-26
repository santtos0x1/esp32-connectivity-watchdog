#include <stdlib.h>
#include "driver/gpio.h"

// Configuration set
gpio_config_t io_conf_led = {
    .pin_bit_mask = (1ULL << 18),
    .mode = GPIO_MODE_OUTPUT,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

void sys_conf_gpio( void )
{
    // Configs the pin with the configuration set
    gpio_config( &io_conf_led );
}