#ifndef _API_H_
#define _API_H_

#include <ESP8266WebServer.h>
#include <NtpClientLib.h>
#include "settings.h"


void init_api();

void api_system();
void api_system_restart();

void api_settings_get();
void api_settings_post();
void api_settings_reset();

void api_settings_wifi_get();
void api_settings_wifi_post();

void api_settings_wifi_sta_ap_list_post();

void api_time_get();
void api_time_post();

//void api_measures_get();

void api_post_echo();


void api_type_get();
void api_serial_get();
void api_serial_post();

#endif
