#include "main.h"


bool init_wifi(void) {
    if (settings.wifi.mode == WIFI_AP || settings.wifi.mode == WIFI_AP_STA) {
        WiFi.mode(settings.wifi.mode);
        
        if (!WiFi.softAP(settings.wifi.ap.ssid.c_str(), settings.wifi.ap.password.c_str(), 1, 0, 1)) {
            Serial.println(F("starting ap failed"));
        }
    } 
    
    if (settings.wifi.mode == WIFI_STA || settings.wifi.mode == WIFI_AP_STA) {
        if (WiFi.getMode() != settings.wifi.mode) {
            WiFi.mode(settings.wifi.mode);
        }

        for (ap_t ap : settings.wifi.sta.ap_list) {
            for (uint8_t i = 0; i < settings.wifi.sta.attempts; i++) {
                wl_status_t status = WiFi.begin(ap.ssid.c_str(), ap.password.c_str());

                if (WiFi.waitForConnectResult() == WL_CONNECTED) {
                    break;
                }

                delay(settings.wifi.sta.attempt_pause);
            }

            if (WiFi.status() == WL_CONNECTED) {
                break;
            }
        }

        if (WiFi.status() != WL_CONNECTED) {
            Serial.println(F("sta not connected"));
        }
    }

    return true;

    // WiFi.begin (ssid, password);
    // int tentativeWiFi = 0;

    // while (WiFi.status() != WL_CONNECTED) {
    //     delay (500);
    //     Serial.print ( "." );
    //     tentativeWiFi++;

    //     if ( tentativeWiFi > 20 ) {
    //         ESP.reset();
    //         while(true) delay(1);
    //     }
    // }

    //    Serial.println("");
    //    Serial.print("Connected to "); Serial.println ( ssid );
    //    Serial.print("IP address: "); Serial.println ( WiFi.localIP() );

    // server.on("/tabmesures.json", sendTabMesures);
    // server.on("/mesures.json", sendMesures);
    // server.on("/gpio", updateGpio);
    // server.on("/graph_temp.json", sendHistory);

    // server.serveStatic("/js", SPIFFS, "/js");
    // server.serveStatic("/css", SPIFFS, "/css");
    // server.serveStatic("/img", SPIFFS, "/img");
    // server.serveStatic("/", SPIFFS, "/index.html");

    // server.begin();
    // Serial.println(F("HTTP server started"));
    // return true;
}

bool init_server(void) {
    updateServer.setup(&server);

    server.on(settings.api.url + "settings/", []() {
        String serialized = settings_serialize();
        server.send(200, "application/json", serialized);
    });

    server.on(settings.api.url + "post_echo", HTTP_POST, []() {
        Serial.println("request");
        String response = "argc count: " + (String)server.args() + "\n";

        for (int i = 0; i < server.args(); i++) {
            response += server.argName(i) + " = " + server.arg(i) + "\n";
        }

        server.send(200, "text/plain", response);
    });

    server.begin(80);
    Serial.println(F("HTTP server started"));
    return true;
}

bool init_time(void) {
    if (settings.time.update_interval) {
        NTP.onNTPSyncEvent([](NTPSyncEvent_t error) {
            if (error) {
                Serial.print(F("Time Sync error: "));

                if (error == noResponse) {
                    Serial.println(F("NTP server not reachable"));
                } else if (error == invalidAddress) {
                    Serial.println(F("Invalid NTP server address"));
                }
            } else {
                Serial.print(F("Got NTP time: "));
                Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
            }
        });

        NTP.begin(settings.time.ntp_server, settings.time.offset, settings.time.daylight);
        NTP.setInterval(settings.time.update_interval);
    }

    Serial.printf("Uptime : \n");//, NTP.getUptime());
    Serial.printf("LastBootTime : \n");//, NTP.getLastBootTime());
    return true;
}

bool init_sensors(void) {
    if (settings.measures.humiduty.interval || settings.measures.temperature.interval) {
        dht.begin();
        temp_m.initialized = true;
        hum_m.initialized = true;
    }

    if (settings.measures.pressure.interval) {
        press_m.initialized = bmp.begin();

        // BMP180?
        if (!press_m.initialized) {
            Serial.println(F("BMP180 KO!"));
            return false;
        }
    }

    return true;
}


void getMeasure(unsigned int ms, measure_stat_t& stat, measure_t& conf, float get(void)) {
    if (conf.interval && stat.initialized && (conf.interval < ms - stat.last_measure || ms < stat.last_measure)) {
        float _value = get();

        if (!isnan(_value)) {
            if (abs(stat.current_value - _value) >= conf.delta) {

            }

            stat.prevois_value = stat.current_value;
            stat.current_value = _value;
        } else {
            stat.error = true;
        }

        stat.last_measure = ms;
    }
}


void setup(void) {
    Serial.begin(115200);

    Serial.println(F("spifs init..."));
    if (!SPIFFS.begin()) {
        Serial.println(F("SPIFFS Mount failed"));
    }

    // Serial.println(F("read..."));
    // if (!read_settings()) {
    //     Serial.println(F("Settings file not parsed. used default"));
    // }

    Serial.println(F("wifi init..."));
    if (!init_wifi()) {
        Serial.println(F("wifi init failed"));
    }

    Serial.println(F("time init..."));
    if (!init_time()) {
        Serial.println(F("time init failed"));
    }

    Serial.println(F("sensors init..."));
    if (!init_sensors()) {
        Serial.println(F("sensors failed"));
    }

    Serial.println(F("server init..."));
    if (!init_server()) {
        Serial.println(F("server not started"));
    }

    Serial.flush();
    //save_settings();
}


void loop() {
    server.handleClient();
    unsigned long ms = millis();

    if (ms % 10000 == 0) {
        getMeasure(ms, temp_m, settings.measures.temperature, [](void) { return dht.readTemperature(); });
        getMeasure(ms, hum_m, settings.measures.humiduty, [](void) { return dht.readHumidity(); });
        getMeasure(ms, press_m, settings.measures.pressure, [](void) { return bmp.readPressure() / 10000.0F * 75; });
        Serial.println(NTP.getTimeStr());
    }
}
