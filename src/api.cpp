#include "api.h"


extern ESP8266WebServer server;
extern settings_t settings;
struct measure_stat_t;


void init_api() {
    server.on(settings.api.url + "system", HTTP_GET, api_system);
    server.on(settings.api.url + "system/restart", HTTP_GET, api_system_restart);

    server.on(settings.api.url + "settings", HTTP_GET, api_settings_get);
    server.on(settings.api.url + "settings", HTTP_POST, api_settings_post);

    server.on(settings.api.url + "settings/reset", HTTP_GET, api_settings_reset);

    server.on(settings.api.url + "settings/wifi", HTTP_GET, api_settings_wifi_get);
    server.on(settings.api.url + "settings/wifi", HTTP_POST, api_settings_wifi_post);

    server.on(settings.api.url + "settings/wifi/sta/ap_list", HTTP_POST, api_settings_wifi_sta_ap_list_post);

    server.on(settings.api.url + "time", HTTP_GET, api_time_get);
    server.on(settings.api.url + "time", HTTP_POST, api_time_post);

    //server.on(settings.api.url + "measures", HTTP_GET, api_time_post);

    server.on(settings.api.url + "post_echo", HTTP_POST, api_post_echo);

    server.on(settings.api.url + "type", HTTP_GET, api_type_get);
    server.on(settings.api.url + "serial", HTTP_GET, api_serial_get);
    server.on(settings.api.url + "serial", HTTP_POST, api_serial_post);
}


void api_system() {
    _DEBUG_PRINT(F("handling GET api/system..."));

    DynamicJsonBuffer buffer;
    JsonObject& root = buffer.createObject();
    root["free_heap"] = ESP.getFreeHeap();
    root["free_programm_size"] = ESP.getFreeSketchSpace();
    root["programm_md5"] = ESP.getSketchMD5();

    root["boot_mode"] = ESP.getBootMode();
    root["boot_version"] = ESP.getBootVersion();

    root["chip_id"] = ESP.getChipId();
    root["flash_chip_id"] = ESP.getFlashChipId();
    root["cpu_freq"] = ESP.getCpuFreqMHz();

    root["core_version"] = ESP.getCoreVersion();

    root["reset_info"] = ESP.getResetInfo();
    root["reset_reason"] = ESP.getResetReason();

    String response;
    root.printTo(response);
    server.send(200, "application/json", response);

    _DEBUG_PRINTLN(F("ok"));
}

void api_system_restart() {
    _DEBUG_PRINT(F("handling GET api/system/restart..."));

    server.send(200);
    ESP.restart();
}


void api_settings_get() {
    _DEBUG_PRINT(F("handling GET api/settings..."));

    String serialized = settings_serialize();
    server.send(200, "application/json", serialized);

    _DEBUG_PRINTLN(F("ok"));
}

void api_settings_post() {
    _DEBUG_PRINT(F("handling POST api/settings..."));

    settings_t new_settings;

    if (server.hasArg("plain")) {
        if (settings_parse(server.arg("plain"), new_settings)) {
            settings = new_settings;
            server.send(200);
        } else {
            server.send(400, "text/plain", "incorect data");
        }
    } else {
        server.send(400, "text/plain", "no valid keys");
    }

    _DEBUG_PRINTLN(F("ok"));
}

void api_settings_reset() {
    _DEBUG_PRINT(F("handling GET api/settings/reset..."));

    settings_t new_settings;
    settings = new_settings;
    settings_save();
    server.send(200);

    _DEBUG_PRINTLN(F("ok"));
}

void api_settings_wifi_get() {
    _DEBUG_PRINT(F("handling GET api/settings/wifi..."));

    // TODO: замутить отдачу блока
    server.send(500);

    _DEBUG_PRINT(F("ok"));
}

void api_settings_wifi_post() {
    _DEBUG_PRINT(F("handling POST api/settings/wifi..."));

    WiFiMode mode = settings.wifi.mode;

    if (server.hasArg("mode")) {
        String s_mode = server.arg("mode");

        if (s_mode.length() == 1 && s_mode[0] >= '0' && s_mode[0] <= '3') {
            mode = (WiFiMode)(s_mode[0] - '0');

            if (mode == WIFI_STA && settings.wifi.sta.ap_list.size() == 0) {
                server.send(400, "text/plain", "can't set mode STA: sta.ap_list is empty");
            } else {
                settings.wifi.mode = mode;
                settings_save();
                server.send(200);
            }
        } else {
            server.send(400, "text/plain", "incorrect mode");
        }
    }

    _DEBUG_PRINTLN(F("ok"));
}

void api_settings_wifi_sta_ap_list_post() {
    _DEBUG_PRINT(F("handling POST api/wifi/sta/ap_list..."));

    if (server.hasArg("ssid") && server.hasArg("password")) {
        ap_t new_ap { .ssid = server.arg("ssid"), .password = server.arg("password") };
        bool found = false;

        for (ap_t& _ap : settings.wifi.sta.ap_list) {
            if (_ap.ssid == new_ap.ssid) {
                _ap.password = new_ap.password;
                found = true;
                break;
            }
        }

        if (!found) {
            settings.wifi.sta.ap_list.push_back(new_ap);
        }

        server.send(200, "text/plain", settings_save() ? "ok" : "error");
    } else {
        server.send(400, "text/plain", "incorrect input data");
    }

    _DEBUG_PRINTLN(F("ok"));
}


void api_time_get() {
    _DEBUG_PRINT(F("handling GET api/time..."));

    String response = "{ \"time\": \"" + NTP.getTimeDateString() + "\" }";
    server.send(200, "application/json", response);

    _DEBUG_PRINTLN(F("ok"));
}

void api_time_post() {
    _DEBUG_PRINT(F("handling POST api/time..."));

    setTime(server.arg("hour").toInt(), server.arg("minute").toInt(), server.arg("second").toInt(),
        server.arg("day").toInt(), server.arg("month").toInt(), server.arg("year").toInt());
    server.send(200);

    _DEBUG_PRINTLN(F("ok"));
}


void api_post_echo() {
    _DEBUG_PRINT(F("handling POST api/post_echo..."));

    String response = "argc count: " + (String)server.args() + "\n";

    for (int i = 0; i < server.args(); i++) {
        response += server.argName(i) + " = " + server.arg(i) + "\n";
    }

    server.send(200, "text/plain", response);

    _DEBUG_PRINTLN(F("ok"));
}


void api_type_get_get() {
    _DEBUG_PRINT(F("handling GET api/type..."));

    server.send(200, "application/json", "{\"type\": \"clock\"}");

    _DEBUG_PRINTLN(F("ok"));
}


void api_serial_get() {
    _DEBUG_PRINT(F("handling GET api/serial..."));

    server.send(200, "application/json", "{\"serial\": \"" + settings.serial + "\"}");

    _DEBUG_PRINTLN(F("ok"));
}

void api_serial_post() {
    _DEBUG_PRINT(F("handling POST api/serial..."));

    if (server.hasArg("serial")) {
        settings.serial = server.arg("serial");
        server.send(200, "ok");
    } else {
        server.send(400);
    }

    _DEBUG_PRINTLN(F("ok"));
}
