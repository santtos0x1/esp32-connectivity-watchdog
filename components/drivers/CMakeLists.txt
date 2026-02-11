idf_component_register(SRCS "patterns.c" "hal_pins.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_gpio config)