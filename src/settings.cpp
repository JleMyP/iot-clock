#include "settings.h"


extern settings_t settings;
extern String settings_filename;



String settings_serialize() {
    DynamicJsonDocument root(1024);

    root["name"] = settings.name;
    root["serial"] = settings.serial;
    root["mdns_enabled"] = settings.mdns_enabled;
    root["mdns_name"] = settings.mdns_name;

    JsonObject wifi = root.createNestedObject("wifi");
    wifi["mode"] = (int)settings.wifi.mode;

    JsonObject wifi_sta = wifi.createNestedObject("sta");
    wifi_sta["attempts"] = settings.wifi.sta.attempts;
    wifi_sta["attempt_pause"] = settings.wifi.sta.attempt_pause;

    JsonArray ap_list = wifi_sta.createNestedArray("ap_list");

    for (auto s_ap : settings.wifi.sta.ap_list) {
        JsonObject ap = ap_list.createNestedObject();
        ap["ssid"] = s_ap.ssid;
        ap["password"] = s_ap.password;
    }

    JsonObject ap = wifi.createNestedObject("ap");
    ap["ssid"] = settings.wifi.ap.ssid;
    ap["password"] = settings.wifi.ap.password;

    JsonObject time = root.createNestedObject("time");
    time["update_interval"] = settings.time.update_interval;
    time["ntp_server"] = settings.time.ntp_server;
    time["offset"] = settings.time.offset;
    time["daylight"] = settings.time.daylight;

    JsonObject remote_server = root.createNestedObject("remote_server");
    remote_server["address"] = settings.remote_server.address;
    remote_server["port"] = settings.remote_server.port;
    remote_server["password"] = settings.remote_server.password;

    JsonObject measures = root.createNestedObject("measures");
    JsonObject temp_m = measures.createNestedObject("temperature");
    temp_m["interval"] = settings.measures.temperature.interval;
    temp_m["send_mode"] = (int)settings.measures.temperature.send_mode;
    temp_m["delta"] = settings.measures.temperature.delta;
    temp_m["packet_size"] = settings.measures.temperature.packet_size;

    JsonObject humiduty_m = measures.createNestedObject("humiduty");
    humiduty_m["interval"] = settings.measures.humiduty.interval;
    humiduty_m["send_mode"] = (int)settings.measures.humiduty.send_mode;
    humiduty_m["delta"] = settings.measures.humiduty.delta;
    humiduty_m["packet_size"] = settings.measures.humiduty.packet_size;

    JsonObject pressure_m = measures.createNestedObject("pressure");
    pressure_m["interval"] = settings.measures.pressure.interval;
    pressure_m["send_mode"] = (int)settings.measures.pressure.send_mode;
    pressure_m["delta"] = settings.measures.pressure.delta;
    pressure_m["packet_size"] = settings.measures.pressure.packet_size;

    JsonObject api = root.createNestedObject("api");
    api["enabled"] = settings.api.enabled;
    api["url"] = settings.api.url;
    api["auth_method"] = (int)settings.api.auth_method;
    api["login"] = settings.api.login;
    api["password"] = settings.api.password;

    JsonObject logging = root.createNestedObject("logging");
    logging["mode"] = (int)settings.logging.mode;
    logging["location"] = (int)settings.logging.location;
    logging["filename"] = settings.logging.location;
    logging["split_mode"] = (int)settings.logging.spit_mode;
    logging["split_size"] = settings.logging.split_size;
    logging["send_interval"] = settings.logging.send_interval;

    String out;
    serializeJson(root, out);
    return out;
}

bool settings_parse(String text, settings_t& dest) {
    DynamicJsonDocument root(3024);
    DeserializationError error = deserializeJson(root, text);

    if (error) {
        _DEBUG_PRINT(F("file parse failed" ));
        _DEBUG_PRINTLN((int)error.code());
        return false;
    }

    dest.name = root["name"].as<char*>();
    dest.serial = root["serial"].as<char*>();
    dest.mdns_enabled = root["mdns_enabled"].as<bool>();
    dest.mdns_name = root["mdns_name"].as<char*>();

    dest.wifi.mode = (WiFiMode)root["wifi"]["mode"].as<uint8_t>();
    dest.wifi.ap.ssid = root["wifi"]["ap"]["ssid"].as<char*>();
    dest.wifi.ap.password = root["wifi"]["ap"]["password"].as<char*>();

    dest.wifi.sta.ap_list.clear();

    for (auto elem : root["wifi"]["sta"]["ap_list"].as<JsonArray>()) {
        ap_t ap { .ssid = elem["ssid"].as<char*>(), .password = elem["password"].as<char*>() };
        dest.wifi.sta.ap_list.push_back(ap);
    }

    dest.time.ntp_server = root["time"]["ntp_server"].as<char*>();
    dest.time.offset = root["time"]["offset"].as<uint8_t>();
    dest.time.update_interval = root["time"]["update_interval"].as<uint32_t>();
    dest.time.daylight = root["time"]["daylight"].as<bool>();

    settings_read_measure(dest.measures.temperature, root["measures"]["temperature"]);
    settings_read_measure(dest.measures.humiduty, root["measures"]["humiduty"]);
    settings_read_measure(dest.measures.pressure, root["measures"]["pressure"]);

    dest.remote_server.address = root["remote_server"]["address"].as<char*>();
    dest.remote_server.password = root["remote_server"]["password"].as<char*>();
    dest.remote_server.port = root["remote_server"]["port"].as<uint32_t>();

    dest.api.enabled = root["api"]["enabled"].as<bool>();
    dest.api.port = root["api"]["port"].as<uint32_t>();
    dest.api.auth_method = (settings_t::api_t::auth_method_t)root["api"]["auth_method"].as<uint8_t>();
    dest.api.url = root["api"]["url"].as<char*>();
    dest.api.login = root["api"]["login"].as<char*>();
    dest.api.password = root["api"]["password"].as<char*>();

    dest.logging.mode = (settings_t::logging_t::mode_t)root["logging"]["mode"].as<uint8_t>();
    dest.logging.split_size = root["logging"]["split_size"].as<uint32_t>();
    dest.logging.spit_mode = (settings_t::logging_t::split_mode_t)root["logging"]["spit_mode"].as<uint8_t>();
    dest.logging.send_interval = root["logging"]["send_interval"].as<uint32_t>();
    dest.logging.location = (settings_t::logging_t::location_t)root["logging"]["location"].as<uint8_t>();
    dest.logging.filename = root["logging"]["filename"].as<char*>();

    return true;
}


void settings_read_measure(measure_t& measure, JsonObject obj) {
    measure.interval = obj["interval"].as<uint32_t>();
    measure.send_mode = (measure_t::send_mode_t)obj["send_mode"].as<uint8_t>();
    measure.delta = obj["delta"].as<float>();
    measure.packet_size = obj["packet_size"].as<uint32_t>();
}


bool settings_read() {
    File f = SPIFFS.open(settings_filename, "r");

    if (!f) {
        f.close();
        return false;
    }

    String content = f.readString();
    f.close();

    if (content.length() == 0) {
        _DEBUG_PRINTLN(F("file is empty"));
        return false;
    }

    settings_t parsed;
    if (!settings_parse(content, parsed)) {
        _DEBUG_PRINTLN(F("parse error"));
        return false;
    }

    settings = parsed;
    return true;
}

bool settings_save() {
    String serialized = settings_serialize();

    File f = SPIFFS.open(settings_filename, "w");
    if (f) {
        f.print(serialized);
        f.close();
        return true;
    }

    return false;
}
