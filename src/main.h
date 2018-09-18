#ifndef _MAIN_H_
#define _MAIN_H_


#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <FS.h>

#include <DHT.h>
#include <Adafruit_BMP085.h>

#include <TimeLib.h>
#include <NtpClientLib.h>
#include <ArduinoJson.h>


#include "settings.h"
#include "display.h"

#define DHTTYPE   DHT22
#define DHTPIN    D4

#define PRINT_DEBUG (msg, ...) ()


struct measure_stat_t {
    bool initialized;
    bool error;
    uint64_t last_measure;
    float prevois_value;
    float current_value;
} temp_m, hum_m, press_m;


DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

settings_t settings;
String settings_filename = "/settings.json";

ESP8266WebServer server;
ESP8266HTTPUpdateServer updateServer;


bool init_wifi(void);
bool init_time(void);
bool init_sensors(void);

void setup(void);
void loop(void);

#endif // _MAIN_H_
