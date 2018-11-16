#include "api.h"


extern ESP8266WebServer server;
extern settings_t settings;


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

    server.on(settings.api.url + "post_echo", HTTP_POST, api_post_echo);
}


void api_system() {
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
}

void api_system_restart() {
    server.send(200);
    ESP.restart();
}


#pragma region settings

void api_settings_get() {
    String serialized = settings_serialize();
    server.send(200, "application/json", serialized);
}

void api_settings_post() {
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
}

void api_settings_reset() {
    settings_t new_settings;
    settings = new_settings;
    settings_save();
}

void api_settings_wifi_get() {
    // TODO: замутить отдачу блока
    server.send(200);
}

void api_settings_wifi_post() {
    WiFiMode mode = settings.wifi.mode;

    if (server.hasArg("mode")) {
        String s_mode = server.arg("mode");

        if (s_mode.length() == 1 && s_mode[0] >= '0' && s_mode[0] <= '3') {
            mode = (WiFiMode)s_mode[0];

            if (mode == WIFI_STA && settings.wifi.sta.ap_list.size() == 0) {
                server.send(400, "text/plain", "can't ser mode STA: sta.ap_list is empty");
            } else {
                settings.wifi.mode = mode;
                settings_save();
                server.send(200);
            }
        } else {
            server.send(400, "text/plain", "incorrect mode");
        }
    }
}

void api_settings_wifi_sta_ap_list_post() {
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
}

#pragma endregion


void api_time_get() {
    String response = "{ \"time\": \"" + NTP.getTimeDateString() + "\" }";
    server.send(200, "application/json", response);
}


void api_time_post() {
    // TODO: реализовать настройку времени
    server.send(200, "text/plain", "not realized");
}

void api_post_echo() {
    Serial.println("request");
    String response = "argc count: " + (String)server.args() + "\n";

    for (int i = 0; i < server.args(); i++) {
        response += server.argName(i) + " = " + server.arg(i) + "\n";
    }

    server.send(200, "text/plain", response);
}
