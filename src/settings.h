#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdint.h>
#include <WString.h>
#include <vector>

#include <IPAddress.h> // shit
#include <ESP8266WiFiType.h>
#include <FS.h>
#include <ArduinoJson.h>



struct ap_t {
    String ssid;
    String password;
};

struct measure_t {
    uint32_t interval = 0;
    enum send_mode_t { OFF, CHANGED, ALWAYS } send_mode = OFF;
    float delta = 1;
    uint32_t packet_size = 5;
};


struct settings_t {
    String name;
    bool mdns_enabled = true;
    String mdns_name = "clock_1";

    struct witi_t {
        WiFiMode mode = WIFI_AP_STA;
        ap_t ap = { "RSVPU_clock", "" };
        struct sta_t {
            uint8_t attempts = 5;
            uint8_t attempt_pause = 5;
            std::vector<ap_t> ap_list = {
                { "RSVPU", "" }
            };
        } sta;
    } wifi;

    struct _time_t {
        uint32_t update_interval = 600;
        String ntp_server = "ntp1.stratum2.ru";
        int32_t offset = 5;
        bool daylight = true;
    } time;

    struct remote_server_t {
        String address;
        uint32_t port;
        String password;
    } remote_server;

    struct measures_t {
        measure_t temperature;
        measure_t humiduty;
        measure_t pressure;
    } measures;

    struct api_t {
        bool enabled = true;
        uint32_t port = 80;
        String url = "/";
        enum auth_method_t { OFF, BASIC, DIGEST, CUSTOM } auth_method = OFF;
        String login;
        String password;
    } api;

    struct logging_t {
        enum mode_t { OFF, SHORT, FULL } mode = OFF;
        enum location_t { HSERIAL, SSERIAL, SYS_FLASH, EXT_FLASH, SD } location = SYS_FLASH;
        enum split_mode_t { DAY, LINE } spit_mode = DAY;
        uint32_t split_size = 5;
        uint32_t send_interval;
        String filename;
    } logging;
};

String settings_serialize();
bool settings_parse(String text, settings_t& dest);
void settings_read_measure(measure_t& measure, JsonObject& obj);

bool settings_read();
bool settings_save();

#endif // _SETTINGS_H_
