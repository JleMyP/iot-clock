#include "main.h"


bool init_wifi(void) {
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

    server.begin();
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

    Serial.print(F("Uptime :"));
    Serial.println(NTP.getUptime());
    Serial.print(F("LastBootTime :"));
    Serial.println(NTP.getLastBootTime());
    return true;
}

bool init_sensors(void) {
    if (settings.measures.humiduty.interval || settings.measures.temperature.interval) {
        dht.begin();
    }

    if (settings.measures.pressure.interval) {
        // BMP180?
        if (!bmp.begin()) {
            Serial.println(F("BMP180 KO!"));
            return false;
        }
    }

    return true;
}


void setup(void) {
    Serial.begin(115200);

    if (!SPIFFS.begin()) {
        Serial.println(F("SPIFFS Mount failed"));
    }

    if (!parse_settings(&settings)) {
        Serial.println(F("Settings file not parsed. used default"));
    }
    
    if (!init_wifi()) {
        Serial.println(F("wifi init failed"));
    }

    if (!init_time()) {
        Serial.println(F("time init failed"));
    }

    if (!init_sensors()) {
        Serial.println(F("sensors failed"));
    }
}


void loop(void) {
    
}
