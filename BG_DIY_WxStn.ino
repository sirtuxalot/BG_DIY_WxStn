/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
BG_DIY_WxStn.ino (InfluxDB)
Author: sirtuxalot@gmail.com
Last Updated: 19 Jun 2022
Notes: Adding code to utilize InfluxDb and WifiManager
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

#include <Wire.h>                                     // common library for all I2C stuf
#include <DHT.h>                                      // library for DHT11
#include <SFE_BMP180.h>                               // library for BMP180
#include <BH1750.h>                                   // library for BH1750 (AKA GY-30)
#include <Adafruit_SI1145.h>                          // library for SI1145
#include <ESP8266WiFi.h>                              // library for web access
#include <InfluxDbClient.h>                           // library for sending data to influxdb
#include <WiFiManager.h>                              // library for managing wifi connectivity

// Defines

const int DHTPIN {14};                                // pin for DHT11
const char DHTTYPE {DHT11};                           // name for DHT11
DHT dht(DHTPIN, DHTTYPE);                             // initialize DHT11 sensor
const char RainSensor {A0};                           // pin for LM393 Rain Sensor
const float ALTITUDE {171.0};                         // Altitude in meters
SFE_BMP180 pressure;                                  // name for BMP180 
BH1750 lightMeter;                                    // name for BH1750 (AKA GY-30)
Adafruit_SI1145 uv;                                   // name for SI1145
#define INFLUXDB_URL "http://192.168.254.203:8096"    // URL for server, and TCP port for influxdb application
#define INFLUXDB_DB_NAME "weather"                    // InfluxDB v1 database name
//#define INFLUXDB_TOKEN "toked-id"                   // InfluxDB 2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>)
//#define INFLUXDB_ORG "org"                          // InfluxDB 2 organization id (Use: InfluxDB UI -> Settings -> Profile -> <name under tile> )
//#define INFLUXDB_BUCKET "bucket"                    // InfluxDB 2 bucket name (Use: InfluxDB UI -> Load Data -> Buckets)
#define TZ_INFO "EST5EDT"                             // Set Time Zone for InfluxDB

// Variables

float temp = 0;
float humid = 0;
float pres = 0;
int light = 0;
int RainSensorValue = 0;
float unIndex = 0;

// Setup connection to InfluxDB instance (v1)
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB_NAME);
// Setup connection to InfluxDB instance (v2)
//InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

Point sensor("weather");

void setup() {

  // for DHT11
  dht.begin();

  // for BMP180
  pressure.begin();

  // for BH1750 (AKA GY-30)
  lightMeter.begin();

  // for SI1145
  uv.begin();

  // connect to wireless network
  Serial.begin(115200);                               // Begin Serial Communication with 115200 Baud Rate
  Serial.println();
  WiFi.mode(WIFI_STA);                                // Configure ESP8266 in STA Mode
  WiFiManager wfm;                                    // Initialize WiFiManager
  //wfm.resetSettings();                              // Uncomment to reset all settings when testing
  wfm.setHostname("wx_station");                      // Set hostname
  bool result;
  result = wfm.autoConnect("WxStnAP");
  if (!result) {
    Serial.println("Failed to Connect!");
    //ESP.Restart();
  } else {
    Serial.print("Connecting to AP! ");
    //Serial.println(ssid);
  }
  Serial.println();
  // Syncing progress and the time will be printed to Serial
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  // Add constant tags - only once
  sensor.addTag("device", "wx_station");
  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  WxStats();
  ESP.deepSleep(120e6);                               // Deep sleep for 2 minutes
}

void WxStats() {
  // Store measured value into point
  sensor.clearFields();

  // read DHT11 sensor
  debugln();
  float humid = dht.readHumidity();
  sensor.addField("humidity",humid);
  debug("humidity: ");
  debug(humid);
  debugln();

  // Read temperature as Fahrenheit (isFahrenheit = true)
  float temp = dht.readTemperature(true);
  sensor.addField("temperature",temp);
  // Check if any reads failed and exit early (to try again).
  if (isnan(humid) || isnan(temp)) {
	  Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(temp, humid);
  sensor.addField("heatindex",hif);
  debug("heat index: ");
  debug(hif);
  debugln();

  // read BMP180 sensor (we could optinally use this for temperature)
  char status;
  double T, P, pres, a;

  // read LM393 Rain Sensor
  RainSensorValue = analogRead(RainSensor);
  debug("moisture: ");
  sensor.addField("rain_intensity",RainSensorValue);
  debug(RainSensorValue);
  debugln();

  // read SI1145
  float uvIndex = uv.readUV();
  unIndex /= 100.0;
  sensor.addField("uvindex",unIndex);
  debug("uv index: ");
  debug(uvIndex);
  debugln();

  // Loop here getting pressure readings every 10 seconds.
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
          sensor.addField("abs_pressure",P);
          debug(" mb, ");
          debug(P*0.0295333727);
          debugln(" inHg");

          pres = pressure.sealevel(P,ALTITUDE);
          debug("relative (sea-level) pressure: ");
          debug(pres);
          sensor.addField("sea_pressure",pres);
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
  sensor.addField("light",light);
  debugln();
  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(client.pointToLineProtocol(sensor));
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
}
