#ifndef UTILS_H
#define UTILS_H

#include "concat.h"

#define GET_MCU_CHIP_NAME( chip_model, boot_tag ) ( \
    ( CHIP_ESP32 ) ? concat( boot_tag, "Chip model: ESP32" ) : \
    ( CHIP_ESP32S2 ) ? concat( boot_tag, " Chip model: ESP32-S2" ) : \
    ( CHIP_ESP32S3 ) ? concat( boot_tag, " Chip model: ESP32-S3" ) : \
    ( CHIP_ESP32C3 ) ? concat( boot_tag, " Chip model: ESP32-C3" ) : \
    ( CHIP_ESP32C2 ) ? concat( boot_tag, " Chip model: ESP32-C2" ) : \
    ( CHIP_ESP32C6 ) ? concat( boot_tag, " Chip model: ESP32-C6" ) : \
    ( CHIP_ESP32H2 ) ? concat( boot_tag, " Chip model: ESP32-H2" ) : \
    ( CHIP_ESP32P4 ) ? concat( boot_tag, " Chip model: ESP32-P4" ) : \
    ( CHIP_ESP32C61 ) ? concat( boot_tag, " Chip model: ESP32-C61" ) : \
    ( CHIP_ESP32C5 ) ? concat( boot_tag, " Chip model: ESP32-C5" ) : \
    ( CHIP_ESP32H21 ) ? concat( boot_tag, " Chip model: ESP32-H21" ) : \
    ( CHIP_ESP32H4 ) ? concat( boot_tag, " Chip model: ESP32-H4" ) : \
    ( CHIP_POSIX_LINUX ) ? concat( boot_tag, " Chip model: ESP32-POSIX/Linux" ) : concat( boot_tag, "Chip model: Undefined" ) \
)

#endif // !UTILS_H

