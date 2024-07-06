//#define Test
/* *******************************************************
 * This code is compiled with the board Generic ESP8266 Module
 * Note ESP8266. WebSocket Services library used is from 
 * Markus Sattler. For WebSockets URL for libraries:
 *     https://github.com/Links2004/arduinoWebSockets
 *
 * OTA - Over The Air is used for compiling and pushing the bin file to ESP 
 * For OTA ElegantOTA is to be considered
 * Details of ElegantOTA use the library by Ayush Sharma URL:
 *     https://github.com/ayushsharma82/ElegantOTA - 
 * ElegantOTA is always at localhost:/update. 
 *
 * Since Serial Port will not be available for debugging, Telnet is used Ref:
 * ESP Telnet by Lennart Hennigs https://github.com/LennartHennigs/ESPTelnet 
 *
 * ezTime by RopGonggrijp https://github.com/ropg/ezTime is used for Time keeping
 *
 * DallasTemperature by Miles Burton for Temeprature sensor is used
 * URL: https://github.com/milesburton/Arduino-Temperature-Control-Library
 * OneWire by Jim Studt. Tom Pollard... https://www.pjrc.com/teensy/td_libs_OneWire.html 
 *
 * For JSON library Arduino_JSON by Arduino ver0.2.0
 * URL https://github.com/arduino-libraries/Arduino_JSON
 *
 * For SD Mem Interface Arduino SD by SparkFun is not used. Instead using already installed
 * Arduino15\packages\esp8266\hardware\esp8266\3.1.2\libraries\SD part of ESP8266

 * In Captive mode, the device will reboot after 10 mins of
 * inactivity. This is in case if local network was busy and 
 * could not be connected.
 *
 * The program by default will start in Captive mode if local 
 * connectivity not available. 
 *
 * Note: Avoid using Strings in 8266. This corrupts memory. 
 * Where possible use char[] or PROGMEM  
 *
 * In case if the program is corrupt change PROGID.
 * This will erase the EEPROM and new program can be easily 
 * loaded.
 *
 * Last uodated:02/06/2024 - Vasu
 * Update: Added persistent Storage for Temperature
 * Ver updated 0x0102
 * Temperature is split into 512 chunks to reduce memory
 * Last updated: 06/07/2024 - Vasu
 ***********************************************************/

#include <WebSocketsServer.h> //Socket Service if required
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Arduino.h>
#include <EEPROM.h>             //WiFi info are stored. Up to 6 networks can be stored
#include "ESPTelnet.h"
#include <ElegantOTA.h>       // may Corrupts EEPROM memory in ESP8266. Need further testing. 
#include <Arduino_JSON.h>
#include <ezTime.h> //Manage Date / Time
#include "Style.h"
#include <ESP8266HTTPClient.h>

//Following for SD card adapter
#include "FS.h"
#include "SD.h"
#include "SPI.h"
//Pin Configuration and declaration for SD MEM Interface
#define MOSI 13
#define MISO 12
#define SCK 14
#define CS 15
bool bCard = true;

#define WHILE_LOOP_DELAY  500   //Connection Wait tim
#define WIFI_TIMEOUT 8         //Time out in secs for Wifi connection.
#define MAXNETWORK 6  
#define SECONDS 1000

//Used for testing only. Remove it in other programs 

//Following are required for Temperature Sensor
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 05          // Temperature Sensor GPIO 05

//Header For EEPROM storage and retrieval
#define version 0x0102   //First cut
#define progId 0x1606 //First started on 16th June 2024
#define NetworkOffset 100   //We will leave first 100 bytes for Headers

#ifdef Test
  const char HOSTNAME[] = "TemperatureMonitor1";   //
  IPAddress ESPIP(172, 25, 4, 1);  //Defalt server IP. Safe on class B Network 
  String ESPSSID = "WTM240619";
#else
  const char HOSTNAME[] = "TemperatureMonitor";   //
  IPAddress ESPIP(172, 25, 3, 1);  //Defalt server IP. Safe on class B Network 
  String ESPSSID = "WTM240616";
#endif

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

//Provides GatewayIP, location and Time zone. 
String LocationUri = "http://ipinfo.io/?token=";
String LocationToken = "xxxxxxxxx"; //use the token from your registered profile
String TZDefault = "Australia/Sydney";
struct NETWORK
{
  char SSID[33];
  char Password[33];
};

NETWORK Networks[MAXNETWORK];

struct HEADER
{
  uint16_t id;
  uint16_t ver; 
} header;

uint8_t storedNetworks = 0;
int storageIndex;
File upLoadFile;
//Network Variables. Minimum Requirement
const char WIFIPwd[] = "pass1234";  //Host password - None

IPAddress netMask(255, 255, 255, 0); //by using 0 in 4th octet, (254 - 5) network connectivity possible
ESP8266WebServer server(80);
WebSocketsServer ws = WebSocketsServer(81); //Socket Service if required
bool bConnect = false;
String logFile = "";
bool bCaptive = false;
//bool bCentigrade = true;
bool bDayChange = false;

unsigned long elapsedTime = 0;  
unsigned long timeDelay = 30000; //30 secs will be used in multiples for next fetch
unsigned long resetDelay = timeDelay * 20;//two timedDelay is 1 minute
unsigned long rd; //Restart Captive Server in 10 mins if idle
unsigned long tempSense = 0; //Time to check Temperature

//For Telnet - if needed
ESPTelnet telnet;
#define TELNETPORT 23
bool bTelnet = false;

//For Temperature Senso
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
char tempCString[10] = "\0";
char tempFString[10] = "\0";


//For Time
Timezone ClkTZ;

String curDir;
String delDir;
void setup() 
{
  delay(1000);
  Serial.begin(115200);
  DS18B20.begin();

  WiFi.softAPConfig(ESPIP, ESPIP, netMask);
  WiFi.softAP(ESPSSID.c_str(), WIFIPwd);
  delay(500);
  SDInit(true);
  logFile+= "Trying to read from EEPROM.\r\n";
  if (loadCredentials()) // Load WLAN credentials from EPROM if not valid or no network stored then we need to start webserver
    connectWifi();
  else
  {
    Serial.println("Unable to continue");
    delay(30000);
    ESP.restart();
  }

  if(bConnect)
  {
    GetLocation();
    waitForSync();
    updateNTP();
    logFile += ClkTZ.dateTime("H:i:s~ ")  + "Current Time Now: " + ClkTZ.dateTime() + "\r\n";
  }
  else
    ClkTZ.setLocation(TZDefault);

  setInterval(timeDelay * 10); //Check clock every 5 minutes
  events();
  server.on("/wifisave", handleWifiSave);
  server.on("/exec", handlePost);
  server.onNotFound(handleNotFound);
  
  //Following is for moving from Temperature Page to Captive & viceVersa
  server.on("/captive",handleCaptive);
  server.on("/WebPage",handleWeb);

  // if the client posts to the upload page
  // Send status 200 (OK) to tell the client we are ready to receive
  // Receive and save the file
  server.on("/upload", HTTP_POST,[](){ server.send(200); }, handleFileUpload );

  if (bConnect) //Replace this with your Program
  {
    server.on("/", handleWeb);
  }
  else //Captive Screen Requirement
  {
    server.on("/", handleCaptive);
    bCaptive = true;
  }
  Serial.println("Starting Server service");
  startServer();
  ElegantOTA.begin(&server); 
  setupTelnet();
  MDNS.addService("ws", "tcp", 81);
  ws.begin();
  ws.onEvent(webSocketEvent); 
  MDNS.begin(HOSTNAME);
  GetTemperature();
  elapsedTime=millis();
  if (logFile.length() >= 2048)
  {
    if (bCard)
    {
      String fn = "/WM-" + ClkTZ.dateTime("Y/F/F-d") + ".log";
      writeFile(fn.c_str(),logFile.c_str());
    }
    logFile = String();
  }
}

void GetTemperature() 
{
  float tempC;
  float tempF;
  String tempList;
  int count = 0;

  do 
  {
    DS18B20.requestTemperatures(); 
    tempC = DS18B20.getTempCByIndex(0);
    tempF = DS18B20.getTempFByIndex(0);
    dtostrf(tempC, 2, 1, tempCString);
    dtostrf(tempF, 2, 1, tempFString);
    delay(100);
    if (count >=10) //Do not Wait for more than 1 second
      break;
    else
      count ++;

  } while ((tempC == 85 || tempC == -127) && (tempF == 185 || tempF == -196.6 )); 
  
  if (count < 10)
  {
    Serial.printf("Sensor Temperature: %s\n",tempCString);
    telnet.printf("%s->Current Temperature: %s°c\r\n", ClkTZ.dateTime("h:i:s A").c_str(),tempCString);
    telnet.printf("%s->Current Temperature: %s°F\r\n", ClkTZ.dateTime("h:i:s A").c_str(),tempFString);
    tempList = ClkTZ.dateTime("h:i:s A") + "," + tempCString + ",°C," + tempFString + ",°F\n";
    ws.broadcastTXT(("x" + tempList).c_str(), tempList.length() + 1);
    
    if (!bCard)
      SDInit(true); //Check to ensure Card is present

    if (bCard)
    { 
      String fn = "/WM-" + ClkTZ.dateTime("Y/F/F-d") + ".csv";
      if (!SD.exists(fn) ) //Need to write BOM
      {
        char BOM[] = "\xEF\xBB\xBF" ;
        logFile += ClkTZ.dateTime("H:i:s~ ")  + "File not Found: " + fn + "\r\n";
        writeFile(fn.c_str(),BOM);
      }
      writeFile(fn.c_str(),tempList.c_str());
    }
  }
  else
  {
    Serial.println("Temperature sensor failed");
    logFile += ClkTZ.dateTime("H:i:s~ ")  + String(" ->") + "Temperature sensor failed\r\n";
  }
      
}


void loop()
{
  MDNS.update();
  server.handleClient();
  ElegantOTA.loop();
  ws.loop();
  telnet.loop();
  events();

  if (!bConnect)
  {
    if ((millis() - rd) > resetDelay) //10 minutes over? then restart
    {
      ESP.restart();
    }
  }  
  else
  {
    while ((millis() - tempSense) > timeDelay * 20) //Check Temperature every 10 minutes
    {
      GetTemperature();
      tempSense = millis();
    }

    if (ClkTZ.hour() == 0)
    {
      if (!bDayChange)
      {
        SDInit(true); 
        if (bCard)
        {
          String fn = "/WM-" + ClkTZ.dateTime("Y/F/F-d") + ".log";
          writeFile(fn.c_str(),logFile.c_str());
        }

        logFile = String();
        GetTemperature(); //get the latest temperature
        bDayChange=true;
        
        //Change the date on the browser
        ws.broadcastTXT(ClkTZ.dateTime("~0l, jS ~o~f F Y").c_str(),ClkTZ.dateTime("~0l, jS ~o~f F Y").length());
      }
    }
    else
      bDayChange = false; //reset for next day
  }  
   
}
