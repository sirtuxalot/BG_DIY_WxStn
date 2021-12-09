/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
BG_DIY_WxStn.StringJson
Author: G. J. Yeomans
Last Updated: 26 Nov 2021
Notes: Added debug code to replace serial.print code in getWX function, add rain sensor code
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define DEBUG 1

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

// Libraries

#include <Wire.h>                      // common library for all I2C stuf
#include <DHT.h>                       // library for DHT11
#include <SFE_BMP180.h>                // library for BMP180
#include <BH1750.h>                    // library for BH1750 (AKA GY-30)
#include <ESP8266WiFi.h>               // library for wireless access
#include <ESP8266WebServer.h>          // library for web server

// Defines

#define DHTPIN 14                      // pin for DHT11
#define DHTTYPE DHT11                  // name for DHT11
DHT dht(DHTPIN, DHTTYPE);              // initialize DHT11 sensor
#define RainSensor A0                  // pin for LM393 Rain Sensor
#define ALTITUDE 171.0                 // Altitude of Lock Haven, PA in meters
SFE_BMP180 pressure;                   // name for BMP180 
BH1750 lightMeter;                     // name for BH1750 (AKA GY-30)
const char* ssid = "XXXXXXXXXX";       // SSID of wireless network
const char* password = "XXXXXXXXXX";   // Password for wireless network
IPAddress ip(192, 168, 254, 128);      // IP address of device
IPAddress gateway(192, 168, 254, 1);   // Gateway IP address
IPAddress subnet(255, 255, 255, 0);    // Network Subnet Mask
ESP8266WebServer server;

// Variables

float temp = 0;
float humid = 0;
float pres = 0;
int light = 0;
int moisture = 0;

void setup() {

  // for DHT11
  dht.begin();

  // for BMP180
  pressure.begin();

  // for BH1750 (AKA GY-30)
  lightMeter.begin();

  // connect to wireless network
  Serial.begin(115200);                // Begin Serial Communication with 115200 Baud Rate
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);                 // Configure ESP8266 in STA Mode
  WiFi.config(ip, gateway, subnet);    // Setup IP Addressing of device
  WiFi.begin(ssid, password);          // Connect to Wi-Fi based on above SSID and Password
  while(WiFi.status() != WL_CONNECTED) // Loop validating connection to wireless network
  {
    Serial.print("*");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to Wi-Fi: ");
  Serial.println(ssid);
  Serial.println();
  Serial.println("Starting ESP8266 Web Server...");
  server.begin();                      // Start the HTTP web Server
  Serial.println("ESP8266 Web Server Started");
  Serial.println();
  Serial.print("The URL of ESP8266 Web Server is: ");
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.println("Use the above URL in your Browser to access ESP8266 Web Server\n");

  // Setup default route
  server.on("/",default_route);
  server.on("/getWX",getWX);
}

void loop() {
  server.handleClient();
}

void getWX() {

  // read DHT11 sensor
  debugln();
  float humid = dht.readHumidity();
  debug("humidity: ");
  debug(humid);
  debugln();

  // Read temperature as Fahrenheit (isFahrenheit = true)
  float temp = dht.readTemperature(true);
  debug("current temperature: ");
  debug(temp);
  debugln();

  // Check if any reads failed and exit early (to try again).
  if (isnan(humid) || isnan(temp)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(temp, humid);
  debug("heat index: ");
  debug(hif);
  debugln();

  // read LM393 Rain Sensor
  moisture = analogRead(RainSensor);
  debug("moisture: ");
  debug(moisture);
  debugln();

  // read BMP180 sensor (we could optinally use this for temperature)
  char status;
  double T, P, pres, a;

  status = pressure.startTemperature();
  if (status != 0) {
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0) {
      status = pressure.startPressure(3);
      if (status != 0) {
        delay(status);
        status = pressure.getPressure(P,T);
        if (status != 0) {
          pres = pressure.sealevel(P,ALTITUDE);
          debug("relative (sea-level) pressure: ");
          debug(pres);
          debug(" mb, ");
          debug(pres*0.0295333727);
          debugln(" inHg");
        }
        else debugln("error retrieving pressure measurement\n");
      }
      else debugln("error starting pressure measurement\n");
    }
    else debugln("error retrieving temperature measurement\n");
  }
  else debugln("error starting temperature measurement\n");

  // read BH1750 sensor
  uint16_t light = lightMeter.readLightLevel();
  debug("light level: ");
  debug(light);
  debugln();

  // build and send json response as a string

  String WX_Response = "{";
  WX_Response += "\"temperature\": ";
  WX_Response += temp;
  WX_Response += "\, \"humidity\": ";
  WX_Response += humid;
  WX_Response += "\, \"heat_index\": ";
  WX_Response += hif;
  WX_Response += "\, \"pressure\": ";
  WX_Response += pres;
  WX_Response += "\, \"light\": ";
  WX_Response += light;
  WX_Response += "\, \"moisture\": ";
  WX_Response += moisture;
  WX_Response += "}";

  server.send(200,"text/json",WX_Response);
}

void default_route() {
  server.send(200,"text/json","{\"name\": \"Hello world\"}");
}
