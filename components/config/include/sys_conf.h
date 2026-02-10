#ifndef SYS_CONF_H
#define SYS_CONF_H

// NVS Macros
#define NVS_PARTITION_NAMESPACE  "wifi_params"

#define NVS_PARAM_SSID           "ssid_param"
#define NVS_PARAM_PASSWORD       "pass_param"

// Defines the stack buffer for fsm task
#define V_FSM_STACK_BUFFER 4096

// Delays
#define LOW_DELAY_TICK_100 100

#define DELAY_HW_TICK_US        MS_TO_US(1)
#define DELAY_HW_STABILIZE_US   MS_TO_US(10)
#define DELAY_HW_RESET_US       MS_TO_US(50)

#define DELAY_UI_REFRESH_US     MS_TO_US(200)
#define DELAY_BOOT_BANNER_US    MS_TO_US(500)

#endif // !SYS_CONF_H