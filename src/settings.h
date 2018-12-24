#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdint.h>
#include <WString.h>
#include <vector>

#include <IPAddress.h> // shit
#include <ESP8266WiFiType.h>
#include <FS.h>
#include <ArduinoJson.h>


#define DHTTYPE DHT22
#define DHTPIN D4
#define _DEBUG

#ifndef _DEBUG
    #define _DEBUG_PRINT(...) ()
    #define _DEBUG_PRINTNL(...) ()
#else
    #define _DEBUG_PRINT(...) Serial.print(__VA_ARGS__); Serial.flush();
    #define _DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__); Serial.flush();
#endif



struct ap_t {
    String ssid;
    String password;
};

struct measure_t {
    uint32_t interval = 0;
    uint32_t packet_size = 5;
    float delta = 1;
    enum send_mode_t { OFF, CHANGED, ALWAYS } send_mode = OFF;
};


struct settings_t {
    String name;
    String serial;
    String mdns_name = "clock_1";
    bool mdns_enabled = true;

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
        int32_t offset = 5;
        String ntp_server = "ntp1.stratum2.ru";
        bool daylight = true;
    } time;

    struct remote_server_t {
        uint32_t port;
        String address;
        String password;
    } remote_server;

    struct measures_t {
        measure_t temperature;
        measure_t humiduty;
        measure_t pressure;
    } measures;

    struct api_t {
        uint32_t port = 80;
        enum auth_method_t { OFF, BASIC, DIGEST, CUSTOM } auth_method = OFF;
        bool enabled = true;
        String url = "/";
        String login;
        String password;
    } api;

    struct logging_t {
        uint32_t split_size = 5;
        uint32_t send_interval;
        enum mode_t { OFF, SHORT, FULL } mode = OFF;
        enum location_t { HSERIAL, SSERIAL, SYS_FLASH, EXT_FLASH, SD } location = SYS_FLASH;
        enum split_mode_t { DAY, LINE } spit_mode = DAY;
        String filename;
    } logging;
};

String settings_serialize();
bool settings_parse(String text, settings_t& dest);
void settings_read_measure(measure_t& measure, JsonObject& obj);

bool settings_read();
bool settings_save();

#endif // _SETTINGS_H_
