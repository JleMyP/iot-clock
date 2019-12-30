#include "main.h"

measure_stat_t temp_m, hum_m, press_m;


Adafruit_BME280 bme;
Adafruit_Sensor* bme_temp = bme.getTemperatureSensor();
Adafruit_Sensor* bme_pressure = bme.getPressureSensor();
Adafruit_Sensor* bme_humidity = bme.getHumiditySensor();

settings_t settings;
String settings_filename = "/settings.json";

WiFiUDP udp;
ESP8266WebServer server;
ESP8266HTTPUpdateServer updateServer;


bool init_wifi() {
    if (settings.wifi.mode == WIFI_AP || settings.wifi.mode == WIFI_AP_STA) {
        WiFi.mode(settings.wifi.mode);

        if (!WiFi.softAP(settings.wifi.ap.ssid.c_str(), settings.wifi.ap.password.c_str(), 1, 0, 1)) {
            _DEBUG_PRINTLN(F("starting ap failed"));
        }
    }

    if (settings.wifi.mode == WIFI_STA || settings.wifi.mode == WIFI_AP_STA) {
        if (WiFi.getMode() != settings.wifi.mode) {
            WiFi.mode(settings.wifi.mode);
        }

        for (ap_t ap : settings.wifi.sta.ap_list) {
            for (uint8_t i = 0; i < settings.wifi.sta.attempts; i++) {
                WiFi.begin(ap.ssid.c_str(), ap.password.c_str());

                if (WiFi.waitForConnectResult() == WL_CONNECTED) {
                    _DEBUG_PRINTLN(ap.ssid);
                    _DEBUG_PRINTLN(WiFi.localIP());
                    break;
                }

                delay(settings.wifi.sta.attempt_pause);
            }

            if (WiFi.status() == WL_CONNECTED) {
                break;
            }
        }

        if (WiFi.status() != WL_CONNECTED) {
            _DEBUG_PRINTLN(F("sta not connected"));
        }
    }

    return true;
}

bool init_server() {
    updateServer.setup(&server);

    init_api();

    server.begin(settings.api.port);
    _DEBUG_PRINTLN(F("HTTP server started"));
    udp.begin(settings.udp_port);
    return true;
}

bool init_time() {
    if (settings.time.update_interval) {
        NTP.onNTPSyncEvent([](NTPSyncEvent_t error) {
            if (error) {
                _DEBUG_PRINTLN(F("Time Sync error: "));

                if (error == noResponse) {
                    _DEBUG_PRINTLN(F("NTP server not reachable"));
                } else if (error == invalidAddress) {
                    _DEBUG_PRINTLN(F("Invalid NTP server address"));
                }
            } else {
                _DEBUG_PRINT(F("Got NTP time: "));
                _DEBUG_PRINTLN(NTP.getTimeDateString(NTP.getLastNTPSync()));
            }
        });

        NTP.begin(settings.time.ntp_server, settings.time.offset, settings.time.daylight);
        NTP.setInterval(settings.time.update_interval);
    }

    _DEBUG_PRINTLN("Uptime : \n");//, NTP.getUptime());
    _DEBUG_PRINTLN("LastBootTime : \n");//, NTP.getLastBootTime());
    return true;
}

bool init_sensors() {
    if (settings.measures.humiduty.interval
        || settings.measures.temperature.interval
        || settings.measures.pressure.interval) {
        press_m.initialized = bme.begin(BME280_ADDRESS_ALTERNATE);
        temp_m.initialized = press_m.initialized;
        hum_m.initialized = press_m.initialized;
        return press_m.initialized;
    }

    return true;
}


void setup() {
    Serial.begin(115200);

    _DEBUG_PRINTLN(F("spifs init..."));
    if (!SPIFFS.begin()) {
        _DEBUG_PRINTLN(F("SPIFFS Mount failed"));
    }

    _DEBUG_PRINTLN(F("read..."));
    if (!settings_read()) {
        _DEBUG_PRINTLN(F("Settings file not parsed. used default"));
    }

    _DEBUG_PRINTLN(F("wifi init..."));
    if (!init_wifi()) {
        _DEBUG_PRINTLN(F("wifi init failed"));
    }

    _DEBUG_PRINTLN(F("time init..."));
    if (!init_time()) {
        _DEBUG_PRINTLN(F("time init failed"));
    }

    _DEBUG_PRINTLN(F("sensors init..."));
    if (!init_sensors()) {
        _DEBUG_PRINTLN(F("sensors failed"));
    }

    _DEBUG_PRINTLN(F("server init..."));
    if (!init_server()) {
        _DEBUG_PRINTLN(F("server not started"));
    }

    server.on(settings.api.url + "measures", HTTP_GET, api_measures_get);

    if (settings.mdns_enabled) {
        _DEBUG_PRINTLN(F("mdns init..."));

        if (!MDNS.begin(settings.mdns_name.c_str())) {
            _DEBUG_PRINTLN(F("HTTP server started"));
        } else {
            _DEBUG_PRINTLN(settings.mdns_name.c_str());
            MDNS.addService("http", "tcp", settings.api.port);
        }
    }

    Serial.flush();
    //save_settings();
}

void loop() {
    server.handleClient();
    unsigned long ms = millis();

    if (ms % 10000 == 0) {
        get_measure(ms, temp_m, settings.measures.temperature, get_temperature);
        get_measure(ms, hum_m, settings.measures.humiduty, get_humidity);
        get_measure(ms, press_m, settings.measures.pressure, get_pressure);
        // _DEBUG_PRINTLN(NTP.getTimeStr());
        // Serial.flush();
    }

    // if (ms % 100) {
    //     handle_udp();
    // }

    delay(10);
}


void handle_udp() {
    int packetSize = udp.parsePacket();
    if (packetSize) {
        char packet_buffer[255];
        Serial.print("Received packet of size ");
        Serial.println(packetSize);
        Serial.print("From ");
        IPAddress remoteIp = udp.remoteIP();
        Serial.print(remoteIp);
        int len = udp.read(packet_buffer, 255);
        if (len > 0) {
            packet_buffer[len] = 0;
        }
        Serial.println("Contents:");
        Serial.println(packet_buffer);
    }
}


void api_measures_get() {
    _DEBUG_PRINT(F("handling GET api/measures..."));

    DynamicJsonDocument root(1024);

    root["temperature"] = temp_m.current_value;
    root["humiduty"] = hum_m.current_value;
    root["pressure"] = press_m.current_value;

    String response;
    serializeJson(root, response);
    server.send(200, "application/json", response);

    _DEBUG_PRINTLN(F("ok"));
}


void get_measure(uint32_t ms, measure_stat_t& stat, measure_t& conf, measire_getter_t get) {
    if (conf.interval && stat.initialized && (conf.interval < (ms - stat.last_measure)
                                              || ms < stat.last_measure)) {
        float _value = get();

        if (!isnan(_value)) {
            if (abs(stat.current_value - _value) >= conf.delta) {
                // TODO: эт шо
            }

            stat.prevois_value = stat.current_value;
            stat.current_value = _value;
        } else {
            stat.error = true;
        }

        stat.last_measure = ms;
    }
}

float get_temperature() {
    return bme.readTemperature();
}

float get_humidity() {
    return bme.readHumidity();
}

float get_pressure() {
    return bme.readPressure();
}
