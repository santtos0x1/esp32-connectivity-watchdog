#ifndef SYS_FSM_H
#define SYS_FSM_H

#include "freertos/FreeRTOS.h"

typedef enum {
    STATE_INIT,    
    STATE_WIFI_CONNECTING,    
    STATE_PROVISIONING,    
    STATE_MQTT_CONNECTING,    
    STATE_OPERATIONAL_ONLINE, 
    STATE_OPERATIONAL_OFFLINE,    
    STATE_SYNCING,    
    STATE_ERROR
} system_state_t;

typedef struct {
    
} bitwise_navigation_conf;

/* Implements a state transition table using an 8-byte bitmask for a minimal memory footprint.
    This enforces strict transition rules within the set_state function,
    ensuring system security and predictable behavior.
*/
static const uint8_t state_bitmask[8] = {
    // Bits: 7(ERR) 6(SYN) 5(OFF) 4(ONL) 3(MQT) 2(PRV) 1(WIF) 0(INI)
    0b10000110, // STATE_INIT            -> Allow: WIF(1), PRV(2), ERR(7)
    0b10001101, // STATE_WIFI_CONNECTING -> Allow: INI(0), PRV(2), MQT(3), ERR(7)
    0b10001001, // STATE_PROVISIONING    -> Allow: INI(0), MQT(3), ERR(7)
    0b11010011, // STATE_MQTT_CONNECTING -> Allow: INI(0), WIF(1), ONL(4), SYN(6), ERR(7)
    0b11100011, // STATE_OPERATIONAL_ON  -> Allow: INI(0), WIF(1), OFF(5), SYN(6), ERR(7)
    0b11010011, // STATE_OPERATIONAL_OFF -> Allow: INI(0), WIF(1), ONL(4), SYN(6), ERR(7)
    0b10001011, // STATE_SYNCING         -> Allow: INI(0), WIF(1), MQT(3), ERR(7)
    0b01111111  // STATE_ERROR           -> Allow: All except bit 7 (it self)
};

esp_err_t fsm_set_state(system_state_t new_state);

system_state_t fsm_get_state(void);

void vTaskFSM(void *pvParameters);

void fsm_init(void);

void panic_dev_restart(TickType_t ms, esp_err_t error_ret) __attribute__((noreturn));

const char *state_to_name(system_state_t state);

uint8_t bitwise_nav(const uint8_t *bitmask_state, system_state_t current_state, system_state_t next_state);

#endif // !SYS_FSM_H