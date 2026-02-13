#include <stdio.h>

#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sys_diag.h"

static const char *diag_tag = "diag";

void vTaskDiag(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    const TickType_t xPeriod = pdMS_TO_TICKS(5000);

    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        uint32_t free_heap_s = esp_get_free_heap_size();
        ESP_LOGI(diag_tag, "Total free heap memory: %d bytes", free_heap_s);
    }
}

void init_diag(void)
{
    xTaskCreate(vTaskDiag, "SYS_DIAG", 2048, NULL, tskIDLE_PRIORITY, NULL);
}