#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_wifi.h"
#include "esp_event.h"
#include <stdbool.h>

void init_wifi(void);
bool connect_to_wifi(void);
bool scan_and_connect_open_network(void);
void create_ap(void);
bool is_wifi_connected(void);

#endif
