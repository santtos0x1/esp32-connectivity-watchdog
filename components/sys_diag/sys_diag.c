#include "sdkconfig.h"

#include <stdio.h>
#include <string.h>

#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_trace.h"

#include "sys_diag.h"
#include "sys_conf.h"

// Defines the stack buffer for diag task
#ifdef CONFIG_DIAG_STACK_SIZE
    #define V_DIAG_STACK_BUFFER  (uint16_t)atoi(CONFIG_DIAG_STACK_SIZE)
#else
    #define V_DIAG_STACK_BUFFER  2048
#endif

static const char *diag_tag = "NS-DIAG";

void vTaskDiag(void *pvParameters)
{
    UBaseType_t stack_free;
    uint32_t free_heap_s;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(HEALTH_CHECK_DELAY_MS);

    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        // Gets the amount of free stack memory
        stack_free = uxTaskGetStackHighWaterMark(NULL);

        // Gets the amount of free heap memory
        free_heap_s = esp_get_free_heap_size();

        ESP_LOGI(diag_tag, "Total free heap memory: %d bytes", free_heap_s);

        ESP_LOGI(
            diag_tag, 
            "Total stack memory left: %d bytes", 
            stack_free * sizeof(StackType_t)
        );    
    }
}

void init_diag(void)
{
    BaseType_t ret_task;

    ret_task = xTaskCreate(
        vTaskDiag,
        V_DIAG_TASK_NAME, 
        V_DIAG_STACK_BUFFER, 
        NULL, 
        tskIDLE_PRIORITY, 
        NULL
    );
    if(ret_task != pdPASS)
    {
        ESP_LOGE(diag_tag, "Critical failure: failed to create diagnostic task!");
        
        return;
    }
}