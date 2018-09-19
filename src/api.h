#ifndef _API_H_
#define _API_H_

#include <ESP8266WebServer.h>
#include <NtpClientLib.h>
#include "settings.h"


void init_api(void);

void api_system_get(void);
void api_system_restart_get(void);

void api_settings_get(void);
void api_settings_post(void);
void api_settings_reset_get(void);

void api_settings_wifi_get(void);
void api_settings_wifi_post(void);

void api_settings_wifi_sta_ap_list_post(void);

void api_time_get(void);
void api_time_post(void);

void api_post_echo(void);


#endif
