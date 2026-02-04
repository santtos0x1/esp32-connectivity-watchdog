#ifndef SOFTAP_PROVISIONING_H
#define SOFTAP_PROVISIONING_H

#include "esp_event.h"

void init_mdns( void );
void init_provisioning( void );
void provisioning_event_handler( void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data );

#endif // !SOFTAP_PROVISIONING_H