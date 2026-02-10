#ifndef SYS_BOOT_H
#define SYS_BOOT_H

#include "esp_chip_info.h"

void esp_heap_info_internal(void);

void esp_chip_info_internal(void);

void esp_show_info(void);

void boot_logo(void);

const char* get_mcu_chip_name(const esp_chip_model_t model);

#endif // !SYS_BOOT_H