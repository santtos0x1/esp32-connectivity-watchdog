#ifndef SYS_CONF_H
#define SYS_CONF_H

// NVS Macros
#define NVS_PARTITION_NAMESPACE "wifi_params"
#define NVS_PARAM_SSID          "ssid_param"
#define NVS_PARAM_PASSWORD      "pass_param"

// Task names
#define V_FSM_TASK_NAME         "FSM_TASK"
#define V_MQTT_TASK_NAME        "MQTT_TASK"
#define V_DIAG_TASK_NAME        "SYS_DIAG_TASK"

// Conversion macros
#define SEC_TO_US(s)            ((s) * 1000000ULL)
#define MS_TO_US(ms)            ((ms) * 1000ULL)

// US Delays
#define DELAY_HW_TICK_US        MS_TO_US(1)
#define DELAY_HW_STABILIZE_US   MS_TO_US(10)
#define DELAY_HW_RESET_US       MS_TO_US(50)

#define DELAY_UI_REFRESH_US     MS_TO_US(200)
#define DELAY_BOOT_BANNER_US    MS_TO_US(500)

// MS delays
#define DELAY_HW_TICK_MS        1
#define DELAY_HW_STABILIZE_MS   10
#define DELAY_HW_RESET_MS       50
#define LOW_DELAY_TICK_MS       100

#define DELAY_UI_REFRESH_MS     200
#define DELAY_BOOT_BANNER_MS    500

#define HEALTH_CHECK_DELAY_MS   5000

#endif // !SYS_CONF_H