# BG_DIY_WxStn.ino

Branch ReadMe, for full details see ReadMe in master branch

Common Downloaded Libraries
- DHT11 - [https://github.com/adafruit/DHT-sensor-library](https://github.com/adafruit/DHT-sensor-library)
- BMP180 - [https://github.com/LowPowerLab/SFE_BMP180.git](https://github.com/LowPowerLab/SFE_BMP180.git)
- BH1750 (AKA GY-30) - [https://github.com/claws/BH1750.git](https://github.com/claws/BH1750.git)
- ESP8266 [https://github.com/esp8266/Arduino](https://github.com/esp8266/Arduino)

ThingSpeak Branch Libraries
- ThingSpeak - [https://github.com/mathworks/thingspeak-arduino](https://github.com/mathworks/thingspeak-arduino)

Settings you will need to change for your station
- const char* ssid {"XXXXXXXXXX"};       // SSID of your wireless network
- const char* password = {"XXXXXXXXXX"}; // Password for your wireless network
- const float ALTITUDE {171.0};          // Altitude of your location (in meters)
- IPAddress ip(###, ###, ###, ###);      // IP address of your device, or remove completely for DHCP
- IPAddress gateway(###, ###, ###, ###); // Gateway IP address of your network, or remove completely for DHCP
- IPAddress subnet(###, ###, ###, ###);  // Network Subnet Mask of your network, or remove completely for DHCP

Revisions:
- 11 July 2021: Code clean-up due to my OCD
- 26 August 2021: First working sketch
- 27 August 2021: More code clean-up and replace web output with json output
- 20 November 2021: Added rain sensor with assistance from [here](https://www.youtube.com/watch?v=2layMOhue7M&t)
- 23 November 2021: Added deep sleep with assistance from [here](https://randomnerdtutorials.com/esp8266-deep-sleep-with-arduino-ide/)
- 23 November 2021: Replace Serial(ln) with Debug(ln) to allow enable/disable of debug content in serial monitor
- 26 Nobember 2021: Code preped for ThingSpeak testing
- 22 December 2021: Standardize format of constants with the sketch