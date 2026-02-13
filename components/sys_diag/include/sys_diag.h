#ifndef SYS_DIAG_H
#define SYS_DIAG_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void vTaskDiag(void *pvParameters);
void init_diag(void);

#endif // !SYS_DIAG_H