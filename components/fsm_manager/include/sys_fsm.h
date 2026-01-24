#ifndef SYS_FSM_H
#define SYS_FSM_H

#include <stdbool.h>

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

bool fsm_set_state( system_state_t new_state );
system_state_t fsm_get_state( void );
static void vTaskFSM( void * pvParameters );
void fsm_init( void );

#endif // !SYS_FSM_H