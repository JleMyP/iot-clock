#ifndef _MAIN_H_
#define _MAIN_H_


#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiUdp.h>
#include <FS.h>

#include <Adafruit_BME280.h>

#include <TimeLib.h>
#include <NtpClientLib.h>
#include <ArduinoJson.h>


#include "settings.h"
#include "api.h"


struct measure_stat_t {
    uint64_t last_measure;
    float prevois_value;
    float current_value;
    bool initialized;
    bool error;
};

measure_stat_t temp_m, hum_m, press_m;

typedef float (*measire_getter_t)();


bool init_wifi();
bool init_time();
bool init_sensors();
void get_measure(uint32_t ms, measure_stat_t& stat, measure_t& conf, measire_getter_t get);

void setup();
void loop();

void handle_udp();

float get_temperature();
float get_humidity();
float get_pressure();

void api_measures_get();

#endif // _MAIN_H_
