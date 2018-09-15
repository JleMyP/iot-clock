#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#include <DHT.h>
#include <Adafruit_BMP085.h>

#include <TimeLib.h>
#include <NtpClientLib.h>

#include <ArduinoJson.h>


#define DATA_PIN D8
#define CP_PIN D0
#define DOTS_PIN D2

#define DHTTYPE   DHT22
#define DHTPIN    D4


DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

ESP8266WebServer server;


void setup() {
    dht.begin();

    if (!bmp.begin())
        Serial.println("BMP180 KO!");
    else
        Serial.println("BMP180 OK");

    NTP.onNTPSyncEvent([](NTPSyncEvent_t error) {
        if (error) {
            Serial.print("Time Sync error: ");

            if (error == noResponse)
                Serial.println("NTP server not reachable");
            else if (error == invalidAddress)
                Serial.println("Invalid NTP server address");
        } else {
            Serial.print("Got NTP time: ");
            Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
        }
    });

    NTP.begin("ntp1.stratum2.ru", -1, true); //192.168.222.17 RSVPU server
    NTP.setInterval(60000);


    Serial.begin(115200);

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

    if (!SPIFFS.begin()) {
        Serial.println("SPIFFS Mount failed");
    } else { 
        Serial.println("SPIFFS Mount succesfull");
        //loadHistory();
    }

    // server.on("/tabmesures.json", sendTabMesures);
    // server.on("/mesures.json", sendMesures);
    // server.on("/gpio", updateGpio);
    // server.on("/graph_temp.json", sendHistory);

    // server.serveStatic("/js", SPIFFS, "/js");
    // server.serveStatic("/css", SPIFFS, "/css");
    // server.serveStatic("/img", SPIFFS, "/img");
    // server.serveStatic("/", SPIFFS, "/index.html");

    server.begin();
    Serial.println("HTTP server started");

    Serial.print("Uptime :");
    Serial.println(NTP.getUptime());
    Serial.print("LastBootTime :");
    Serial.println(NTP.getLastBootTime());
}


void loop() {
    
}