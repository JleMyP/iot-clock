#ifndef _MAIN_H_
#define _MAIN_H_


#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <FS.h>

#include <Adafruit_BME280.h>

#include <TimeLib.h>
#include <NtpClientLib.h>
#include <ArduinoJson.h>


#include "settings.h"
#include "api.h"



extern measure_stat_t temp_m, hum_m, press_m;

typedef float (*measure_getter_t)();


bool init_wifi();
bool init_time();
bool init_sensors();
void get_measure(uint32_t ms, measure_stat_t& stat, measure_t& conf, measure_getter_t get);

void setup();
void loop();

void handle_udp();

float get_temperature();
float get_humidity();
float get_pressure();

#endif // _MAIN_H_
