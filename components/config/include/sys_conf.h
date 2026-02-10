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
#endif // !SYS_CONF_H