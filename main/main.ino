
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <FS.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <ArduinoJson.h>

#define DATA_PIN D8
#define CP_PIN D0
#define DOTS_PIN D2


#define ssid "kamikadze"        // WiFi SSID
#define password "18041996"    // WiFi password
#define DHTTYPE   DHT22             // DHT type (DHT11, DHT22)
#define DHTPIN    D4                // DHT Pin
#define HISTORY_FILE "/history.json"
const uint8_t GPIOPIN[4] = {D5,D6,D7,D8};  // Led
float   t = 0 ;
float   h = 0 ;
float   pa = 0;
int     sizeHist = 84 ;        //History size 

const long intervalHist = 1000 * 60 * 5;  //5 measures / hours
unsigned long previousMillis = intervalHist;  //time of last point added

// Création des objets / create Objects
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
ESP8266WebServer server ( 80 );

StaticJsonBuffer<10000> jsonBuffer;                 // Current JSON static buffer
JsonObject& root = jsonBuffer.createObject();
JsonArray& timestamp = root.createNestedArray("timestamp");
JsonArray& hist_t = root.createNestedArray("t");
JsonArray& hist_h = root.createNestedArray("h");
JsonArray& hist_pa = root.createNestedArray("pa");
JsonArray& bart = root.createNestedArray("bart");   //Key histogramm (temp/humidity)
JsonArray& barh = root.createNestedArray("barh");   //Key histogramm (temp/humidity)

char json[10000];                                   //JSON export buffer

void updateGpio(){
  String gpio = server.arg("id");
  String etat = server.arg("etat");
  String success = "1";
  int pin = D5;
 if ( gpio == "D5" ) {
      pin = D5;
 } else if ( gpio == "D7" ) {
     pin = D7;
 } else if ( gpio == "D8" ) {
     pin = D8;  
 } else {   
      pin = D5;
  }
  Serial.println(pin);
  if ( etat == "1" ) {
    digitalWrite(pin, HIGH);
  } else if ( etat == "0" ) {
    digitalWrite(pin, LOW);
  } else {
    success = "1";
    Serial.println("Err Led Value");
  }
  
  String json = "{\"gpio\":\"" + String(gpio) + "\",";
  json += "\"etat\":\"" + String(etat) + "\",";
  json += "\"success\":\"" + String(success) + "\"}";
    
  server.send(200, "application/json", json);
  Serial.println("GPIO updated");
}

void sendMesures() {
  String json = "{\"t\":\"" + String(t) + "\",";
  json += "\"h\":\"" + String(h) + "\",";
  json += "\"pa\":\"" + String(pa) + "\"}";

  server.send(200, "application/json", json);
  Serial.println("Send measures");
}

void calcStat(){
  float statTemp[7] = {-999,-999,-999,-999,-999,-999,-999};
  float statHumi[7] = {-999,-999,-999,-999,-999,-999,-999};
  int nbClass = 7;  // Nombre de classes - Number of classes                         
  int currentClass = 0;
  int sizeClass = hist_t.size() / nbClass;  // 2
  double temp;
  //
  if ( hist_t.size() >= sizeHist ) {
    //Serial.print("taille classe ");Serial.println(sizeClass);
    //Serial.print("taille historique ");Serial.println(hist_t.size());
    for ( int k = 0 ; k < hist_t.size() ; k++ ) {
      temp = root["t"][k];
      if ( statTemp[currentClass] == -999 ) {
        statTemp[ currentClass ] = temp;
      } else {
        statTemp[ currentClass ] = ( statTemp[ currentClass ] + temp ) / 2;
      }
      temp = root["h"][k];
      if ( statHumi[currentClass] == -999 ) {
        statHumi[ currentClass ] = temp;
      } else {
        statHumi[ currentClass ] = ( statHumi[ currentClass ] + temp ) / 2;
      }
         
      if ( ( k + 1 ) > sizeClass * ( currentClass + 1 ) ) {
        //Serial.print("k ");Serial.print(k + 1);Serial.print(" Cellule statTemp = ");Serial.println(statTemp[ currentClass ]);
        currentClass++;
      } else {
        //Serial.print("k ");Serial.print(k + 1);Serial.print(" < ");Serial.println(sizeClass * currentClass);
      }
    }
    
    Serial.println("Histogram - Temperature"); 
    for ( int i = 0 ; i < nbClass ; i++ ) {
      Serial.print(statTemp[i]);Serial.print('|');
    }
    Serial.println("Histogram - Humidity "); 
    for ( int i = 0 ; i < nbClass ; i++ ) {
      Serial.print(statHumi[i]);Serial.print('|');
    }
    Serial.print("");
    if ( bart.size() == 0 ) {
      for ( int k = 0 ; k < nbClass ; k++ ) { 
        bart.add(statTemp[k]);
        barh.add(statHumi[k]);
      }  
    } else {
      for ( int k = 0 ; k < nbClass ; k++ ) { 
        bart.set(k, statTemp[k]);
        barh.set(k, statHumi[k]);
      }  
    }
  }
}

void sendTabMesures() {
  double temp = root["t"][0];      //get oldest record (temperature)
  String json = "[";
  json += "{\"mesure\":\"Температура\",\"valeur\":\"" + String(t) + "\",\"unite\":\" °C\",\"glyph\":\"glyphicon-indent-left\",\"precedente\":\"" + String(temp) + "\"},";
  temp = root["h"][0];             // get oldest record (humidity)
  json += "{\"mesure\":\"Влажность\",\"valeur\":\"" + String(h) + "\",\"unite\":\" %\",\"glyph\":\"glyphicon-tint\",\"precedente\":\"" + String(temp) + "\"},";
  temp = root["pa"][0];             //get oldest record (Atmospheric Pressure)
  json += "{\"mesure\":\"Атмосферное давление\",\"valeur\":\"" + String(pa) + "\",\"unite\":\" мм рт.ст\",\"glyph\":\"glyphicon-dashboard\",\"precedente\":\"" + String(temp) + "\"}";
  json += "]";
  server.send(200, "application/json", json);
  Serial.println("Send data tab");
}

void sendHistory(){  
  root.printTo(json, sizeof(json));             // Export JSON object as a string
  server.send(200, "application/json", json);   // Send history data to the web client
  Serial.println("Send History");   
}

void loadHistory(){
  File file = SPIFFS.open(HISTORY_FILE, "r");
  if (!file){
    Serial.println("No History Exist");
  } else {
    size_t size = file.size();
    if ( size == 0 ) {
      Serial.println("History file empty !");
    } else {
      std::unique_ptr<char[]> buf (new char[size]);
      file.readBytes(buf.get(), size);
      JsonObject& root = jsonBuffer.parseObject(buf.get());
      if (!root.success()) {
        Serial.println("Impossible to read JSON file");
     } else {
        Serial.println("Historique charge - History loaded");
        root.prettyPrintTo(Serial);  
      }
    }
    file.close();
  }
}

void saveHistory(){
  Serial.println("Save History");            
  File historyFile = SPIFFS.open(HISTORY_FILE, "w");
  root.printTo(historyFile); //Export and save JSON object to SPIFFS area
  historyFile.close();  
}

void setup() {

initDisplayPins();
testDisplay();
  
  NTP.onNTPSyncEvent([](NTPSyncEvent_t error) {
    if (error) {
      Serial.print("Time Sync error: ");
      if (error == noResponse)
        Serial.println("NTP server not reachable");
      else if (error == invalidAddress)
        Serial.println("Invalid NTP server address");
      }
    else {
      Serial.print("Got NTP time: ");
      Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
    }
  });
  //NTP Server, time offset, daylight 
  NTP.begin("ntp1.stratum2.ru",-1, true); //192.168.222.17 RSVPU server
  NTP.setInterval(60000);
  delay(500);
     
  for ( int x = 0 ; x < 5 ; x++ ) {
    pinMode(GPIOPIN[x], OUTPUT);
  }
  
  Serial.begin ( 115200 );
 //Init BMP180
  if ( !bmp.begin() ) {
    Serial.println("BMP180 KO!");
    while (1);
  } else {
    Serial.println("BMP180 OK");
  }

  WiFi.begin ( ssid, password );
  int tentativeWiFi = 0;
  //Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 ); Serial.print ( "." );
    tentativeWiFi++;
    if ( tentativeWiFi > 20 ) {
      ESP.reset();
      while(true)
        delay(1);
    }
  }
  //WiFi connexion is OK
  Serial.println ( "" );
  Serial.print ( "Connected to " ); Serial.println ( ssid );
  Serial.print ( "IP address: " ); Serial.println ( WiFi.localIP() );
  
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS Mount failed");        //Serious problem with SPIFFS 
  } else { 
    Serial.println("SPIFFS Mount succesfull");
    loadHistory();
  }
  delay(50);
  
  server.on("/tabmesures.json", sendTabMesures);
  server.on("/mesures.json", sendMesures);
  server.on("/gpio", updateGpio);
  server.on("/graph_temp.json", sendHistory);

  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/css", SPIFFS, "/css");
  server.serveStatic("/img", SPIFFS, "/img");
  server.serveStatic("/", SPIFFS, "/index.html");

  server.begin();
  Serial.println ( "HTTP server started" );

  Serial.print("Uptime :");
  Serial.println(NTP.getUptime());
  Serial.print("LastBootTime :");
  Serial.println(NTP.getLastBootTime());
}

void loop() {
  // put your main code here, to run repeatedly:
  //testDisplay();
  server.handleClient();
  t = dht.readTemperature();
 h = dht.readHumidity();
  pa = (bmp.readPressure() / 10000.0F)*75;
  if ( isnan(t) || isnan(h) ) {
    //Error, no valid value
  } else {
    addPtToHist();
  }
  delay(5);
}
////////////////////////----------------------------------------------------------------------------------------------------------------////////////////////////
void initDisplayPins()
{

  pinMode(DATA_PIN, OUTPUT);
  digitalWrite(DATA_PIN, LOW);
  pinMode(CP_PIN, OUTPUT);
  digitalWrite(CP_PIN, LOW);
  pinMode(DOTS_PIN,OUTPUT);
  digitalWrite(DOTS_PIN,LOW);

printf("init pins\n");
}

void enableDots()
{

  digitalWrite(DOTS_PIN,HIGH);

}

void disableDots()
{

  digitalWrite(DOTS_PIN,LOW);

}

void writeBit(uint8_t databit)
{
//printf("wr bit");

  digitalWrite(CP_PIN, LOW);

  digitalWrite(DATA_PIN, databit);
  delayMicroseconds(10);
  digitalWrite(CP_PIN, HIGH);
  delayMicroseconds(10);

}

void writeByte(uint8_t databyte) {

  for (uint8_t i = 0; i < 8; i++) {
    writeBit((databyte >> i) & 0x01);
  }

}
void writeData(uint8_t* data, uint8_t datalength) {

//printf("wr data");
  for (int i = 0; i < datalength; i++) {
    writeByte(data[i]);
  }

  char *c = (char *)calloc(5,sizeof(char));
  for(uint8_t i = 0; i < 4; i++)
    c[i] = ByteCodeToASCII(data[i]);
  printf("%s\n",c);

}

char ByteCodeToASCII(uint8_t code)
{
  switch (code)
    {
      case 0b11111100:
        return '0';
      case 0b00001100:
        return '1';
      case 0b11011010:
        return '2';
      case 0b10011110:
        return '3';
      case 0b00101110:
        return '4';
      case 0b10110110:
        return '5';
      case 0b11110110:
        return '6';
      case 0b00011100:
        return '7';
      case 0b11111110:
        return '8';
      case 0b10111110:
        return '9';
      case 0b00100000:
    return '-';
      default:
        return 0;
    
    }
}

uint8_t ASCIItoByteCode(char c)
{
  switch (c)
  {
    case '0':
      return 0b11111100;
    case '1':
      return 0b00001100;
    case '2':
      return 0b11011010;
    case '3':
      return 0b10011110;
    case '4':
      return 0b00101110;
    case '5':
      return 0b10110110;
    case '6':
      return 0b11110110;
    case '7':
      return 0b00011100;
    case '8':
      return 0b11111110;
    case '9':
      return 0b10111110;
    case '-':
  return 0b00100000;
    case 'C':
  return 0b11110000;
    case 'H':
  return 0b01101110;
    case 'P':
  return 0b01111010;
    default:
      return 0;
    
  }
}

uint8_t getByteCode(uint8_t number)
{
//printf("get byte code for %d",number);
  switch (number)
  {
    case 0:
      return 0b11111100;
    case 1:
      return 0b00001100;
    case 2:
      return 0b11011010;
    case 3:
      return 0b10011110;
    case 4:
      return 0b00101110;
    case 5:
      return 0b10110110;
    case 6:
      return 0b11110110;
    case 7:
      return 0b00011100;
    case 8:
      return 0b11111110;
    case 9:
      return 0b10111110;
    default:
      return 0;
  }
}
void testDisplay()
{
  uint8_t data[4] = {0, 0, 0, 0};
  for (uint8_t i = 0; i < 11; i++)
  {
    for (uint8_t j = 0; j < 4; j++)
      data[j] = getByteCode(i);
    writeData(data, 4);
    delay(500);

  }
}

///////////////////////----------------------------------------------------------------------------------------------------------------////////////////////////
void addPtToHist(){
  unsigned long currentMillis = millis();
  
  //Serial.println(currentMillis - previousMillis);
  if ( currentMillis - previousMillis > intervalHist ) {
    long int tps = NTP.getTime();
    previousMillis = currentMillis;
    Serial.println(NTP.getTime());
    Serial.println ("Время");
    Serial.print (tps); Serial.print (". ");
    if ( tps > 0 ) {
      timestamp.add(tps);
      hist_t.add(double_with_n_digits(t, 1));
      hist_h.add(double_with_n_digits(h, 1));
      hist_pa.add(double_with_n_digits(pa, 1));

      //root.printTo(Serial);
      if ( hist_t.size() > sizeHist ) {
        
        timestamp.removeAt(0);
        hist_t.removeAt(0);
        hist_h.removeAt(0);
        hist_pa.removeAt(0);
      }
      //Serial.print("size hist_t ");Serial.println(hist_t.size());
      calcStat();
      delay(100);
      saveHistory();
      //root.printTo(Serial);  
    }  
  }
}
