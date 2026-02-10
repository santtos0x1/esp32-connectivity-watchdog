#ifndef UTILS_H
#define UTILS_H

/*
    This macro simplifies the chip get and reduces byte waste in sys_boot
*/
#define GET_MCU_CHIP_NAME(chip_model) ( \
    (CHIP_ESP32) ? "Chip model: ESP32" : \
    (CHIP_ESP32S2) ? "Chip model: ESP32-S2" : \
    (CHIP_ESP32S3) ? "Chip model: ESP32-S3" : \
    (CHIP_ESP32C3) ? "Chip model: ESP32-C3" : \
    (CHIP_ESP32C2) ? "Chip model: ESP32-C2" : \
    (CHIP_ESP32C6) ? "Chip model: ESP32-C6" : \
    (CHIP_ESP32H2) ? "Chip model: ESP32-H2" : \
    (CHIP_ESP32P4) ? "Chip model: ESP32-P4" : \
    (CHIP_ESP32C61) ? "Chip model: ESP32-C61" : \
    (CHIP_ESP32C5) ? "Chip model: ESP32-C5" : \
    (CHIP_ESP32H21) ? "Chip model: ESP32-H21" : \
    (CHIP_ESP32H4) ? "Chip model: ESP32-H4"  : \
    (CHIP_POSIX_LINUX) ? "Chip model: ESP32-POSIX/Linux" : "Chip model: Undefined" \
)

#endif // !UTILS_H