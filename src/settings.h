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
    uint32_t interval = 300;
    enum { OFF, CHANGED, ALWAYS } send_mode;
    float delta = 1;
    uint32_t packet_size = 5;
};

struct settings_t {
    String name;
    bool mdns_enabled = true;
    String mdns_name;

    struct {
        WiFiMode mode = WIFI_AP;
        ap_t ap = { "RSVPU_clock", "" };
        struct {
            uint8_t attempts = 5;
            uint8_t attempt_pause = 5;
            std::vector<ap_t> ap_list = {
                { "RSVPU", "" }
            };
        } sta;
    } wifi;

    struct {
        uint32_t update_interval = 60000;
        String ntp_server = "ntp1.stratum2.ru";
        int32_t offset = -1;
        bool daylight = true;
    } time;

    struct {
        String address;
        uint32_t port;
        String password;
    } remote_server;

    struct {
        measure_t temperature;
        measure_t humiduty;
        measure_t pressure;
    } measures;

    struct {
        bool enabled = true;
        String url = "/";
        enum { OFF, BASIC, DIGEST, CUSTOM } auth_method;
        String login;
        String password;
    } api;

    struct {
        enum { OFF, SHORT, FULL } mode;
        enum { HSERIAL, SSERIAL, SYS_FLASH, EXT_FLASH, SD } location;
        enum { DAY, LINE } spit_mode;
        uint32_t split_size = 5;
        String filename;
        uint32_t send_interval;
    } logging;
};

String settings_serialize();
bool settings_parse(String text, settings_t& dest);

bool settings_read(void);
bool settings_save(void);


#endif // _SETTINGS_H_
