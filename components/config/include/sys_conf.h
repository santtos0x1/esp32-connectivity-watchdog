#ifndef SYS_CONF_H
#define SYS_CONF_H

// NVS Macros
#define NVS_PARTITION_NAME  "wifi_params"

#define NVS_PARAM_SSID      "ssid_param"
#define NVS_PARAM_PASSWORD  "pass_param"
#define NVS_PARAM_BSSID     "bssid_param"

// Defines the stack buffer for fsm task
#define V_FSM_STACK_BUFFER 4096

#endif // !SYS_CONF_H