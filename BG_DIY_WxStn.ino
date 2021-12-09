/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

BG_WxStn.ino

Banggood "AOQDQDQD ESP8266 Weather Station Kit with Temperature Humidity Atmosphetic Pressure Light Sensor 0.96 Display
for Arduino IDE IoT Starter" 

Uses libraries from Adafruit, Adi Dax, Sparkfun, Christopher Laws and various individual contributers to the arguino
labrary ecosystem

Original wifi example file by pileofstuff.ca

Git repo with semi-useful info, that was added to source folder
https://github.com/GJKJ/WSKS

Revisions:
11 July 2021: Code clean-up due to my OCD
26 August 2021: First working sketch
23 November 2021: Added deep sleep

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
#include <ESP8266WiFi.h>               // library for web server

// Defines

#define DHTPIN 14                      // pin for DHT11
#define DHTTYPE DHT11                  // name for DHT11
DHT dht(DHTPIN, DHTTYPE);              // initialize DHT11 sensor
#define ALTITUDE 171.0                 // Altitude of Lock Haven, PA in meters
SFE_BMP180 pressure;                   // name for BMP180 
BH1750 lightMeter;                     // name for BH1750 (AKA GY-30)
const char* ssid = "XXXXXXXXXX";       // SSID of wireless network
const char* password = "XXXXXXXXXX";   // Password for wireless network
WiFiClient client;                     // use as wifi client
IPAddress ip(192, 168, 254, 128);      // IP address of device
IPAddress gateway(192, 168, 254, 1);   // Gateway IP address
IPAddress subnet(255, 255, 255, 0);    // Network Subnet Mask
#define RainSensor A0                  // pin for LM393 Rain Sensor

// Variables

float temp = 0;
float humid = 0;
float pres = 0;
int light = 0;
unsigned long LastEntry;
int RainSensorValue = 0;

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
  espServer.begin();                   // Start the HTTP web Server
  Serial.println("ESP8266 Web Server Started");
  Serial.println();
  Serial.print("The URL of ESP8266 Web Server is: ");
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.println("Use the above URL in your Browser to access ESP8266 Web Server\n");
  
  WxStats();
  ESP.deepSleep(300e6);
}

void WxStats() {
  // read DHT11 sensor
  debugln();
  float humid = dht.readHumidity();
  debug("humidity: ");
  debug(humid);
  debugln();

  // Read temperature as Fahrenheit (isFahrenheit = true)
  float temp = dht.readTemperature(true);
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

  // read BMP180 sensor (we could optinally use this for temperature)
  char status;
  double T, P, pres, a;

  // read LM393 Rain Sensor
  RainSensorValue = analogRead(RainSensor);
  debug("moisture: ");
  if (RainSensorValue <= 100) {
    debug("Heavy Rain - ");
    if (RainSensorValue > 100 and RainSensorValue <= 400) {
      debug("Raining - ");
      if (RainSensorValue > 400 and RainSensorValue <= 800) {
        debug("Light Rain - ");
      }
    }
  }
  debug(RainSensorValue);

  // Loop here getting pressure readings every 10 seconds.
  debugln();
  debug("provided altitude: ");
  debug(ALTITUDE);
  debug(" meters, ");
  debug(ALTITUDE*3.28084);
  debugln(" feet");

  // If you want to measure altitude, and not pressure, you will instead need
  // to provide a known baseline pressure.
  status = pressure.startTemperature();
  if (status != 0) {
    // Wait for the measurement to complete:
    delay(status);

    status = pressure.getTemperature(T);
    if (status != 0) {
      // Print out the measurement:
      debug("temperature: ");
      debug(T);
      debug(" deg C, ");
      debug((9.0/5.0)*T+32.0);
      debugln(" deg F");
    
      status = pressure.startPressure(3);
      if (status != 0) {
        // Wait for the measurement to complete:
        delay(status);

        status = pressure.getPressure(P,T);
        if (status != 0) {
          // Print out the measurement:
          debug("absolute pressure: ");
          debug(P);
          debug(" mb, ");
          debug(P*0.0295333727);
          debugln(" inHg");

          pres = pressure.sealevel(P,ALTITUDE);
          debug("relative (sea-level) pressure: ");
          debug(pres);
          debug(" mb, ");
          debug(pres*0.0295333727);
          debugln(" inHg");

          a = pressure.altitude(P,pres);
          debug("computed altitude: ");
          debug(a);
          debug(" meters, ");
          debug(a*3.28084);
          debugln(" feet");
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
}

void loop() {
}