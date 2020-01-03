#ifndef _API_H_
#define _API_H_

#include <ESP8266WebServer.h>
#include <NtpClientLib.h>
#include "settings.h"



extern ESP8266WebServer server;
extern settings_t settings;
struct measure_stat_t {
    uint64_t last_measure;
    float prevois_value;
    float current_value;
    bool initialized;
    bool error;
};
extern measure_stat_t temp_m, hum_m, press_m;

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

void api_measures_get();

#endif
