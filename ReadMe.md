# BG_Wx_Stn.ino

[Banggood AOQDQDQD ESP8266 Weather Station Kit](https://www.banggood.com/AOQDQDQD-ESP8266-Weather-Station-Kit-with-Temperature-Humidity-Atmosphetic-Pressure-Light-Sensor-0_96-Display-for-Arduino-IDE-IoT-Starter-p-1751604.html) 

Uses libraries from Adafruit, Adi Dax, Sparkfun, Christopher Laws and various individual contributers to the arduino
library ecosystem

Common Downloaded Libraries
- DHT11 - [https://github.com/adafruit/DHT-sensor-library](https://github.com/adafruit/DHT-sensor-library)
- BMP180 - [https://github.com/LowPowerLab/SFE_BMP180.git](https://github.com/LowPowerLab/SFE_BMP180.git)
- BH1750 (AKA GY-30) - [https://github.com/claws/BH1750.git](https://github.com/claws/BH1750.git)

Revisions:
- 11 July 2021: Code clean-up due to my OCD
- 26 August 2021: First working sketch
- 27 August 2021: More code clean-up and replace web output with json output
- 28 August 2021: Utilize Arduino Json library to construct json output
- 20 November 2021: Added rain sensor with assistance from [here](https://www.youtube.com/watch?v=2layMOhue7M&t)
- 23 November 2021: Added deep sleep with assistance from [here](https://randomnerdtutorials.com/esp8266-deep-sleep-with-arduino-ide/)
