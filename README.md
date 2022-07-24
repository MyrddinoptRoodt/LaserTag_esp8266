# **LaserTag_esp8266**
## **installation**
To use this project, we recommend using the provided schema: ![a picture detailing the shematics](https://github.com/MyrddinoptRoodt/LaserTag_esp8266/blob/Version1/schemas%20and%20photos/schematics%20of%20lazer%20gun%20MK4.png?raw=true)
### **requirements (hardware):**
**Here we use:**
- 1 raspberry pi 4b (optional: for dashboard and mqtt server)
  
**and per gun:**
- 1 ESP8266 (nodeMCU v3)
- 4 buttons (we reccomend using 2 triggers (conected to A0: reload, and D7: shoot), and 2 buttons)
- 1 standaard pizzo buzzer
- 1 3mm IR LED
- 1 Ir receiver
- 1 oled screen
- 1 2n2222 transistor
- 4 10K Ω resistors (to ground the buttons)
- 2 individualy adressable RGB LEDs
- 1 4x AAA battery holder
- 1 3.3K Ω resistor (for the transistor)
- 1 100Ω resistor (for the IR lED)


**Warning!!**

an aditional switch between the transisor and the D3 pin is required: at boot the switch has to be off, else the ESP8266 will fail to boot.



### **requirements (software):**
Make sure you have visual studio code installed.
Install the platform IO extention into visual studio code.

1. clone the repo to a chosen folder
2. open the folder with visual studio code
3. open the source folder, and edit main.cpp:
   1. change the IDs in the correct places (indicated by "CHANGE!!!")
   2. change the wifi credentials, and the mqtt server address
      ~~~~     
	    const char* ssid = "name of your 2.4ghz Wifi";
        const char* password = "";
		const char* mqtt_server = "192.168.1.5"; 
4. connect the esp via usb cable to the pc
5. upload the edited code to the ESP

## **about:**
This is a group project to build laser tag guns with some basic featuers (up to 4 teams, multiple game modes, multiple guns,...).
This was originaly started with the intention of producing a few lasertag guns for the science fair. 

This project is written using Visual Studio Code with the platformIO extention.
For this project we use a (LoLin) NodeMcu v3 (using the esp 8266)(in platformIO seen as nodeMcu v1.0), and the IRremoteESP8266 libary.
Their github: https://github.com/crankyoldgit/IRremoteESP8266


### **updates**


#### update: since: 12/11/2021
Added added [Pubsubclient](https://github.com/knolleary/pubsubclient) libary:

by: [Knolleary ](https://github.com/knolleary)

----------


#### **update: since: 27/10/2021**
This update we introduced  the u8glib-HAL libary:  [u8glib-HAL libary](https://github.com/MarlinFirmware/U8glib-HAL) 

by:	
[Scott Lahteine](https://github.com/thinkyhead)

We also added the ESP8266 and 
ESP32 OLED driver for SSD1306 
displays libary: [ESP8266 and 
ESP32 OLED driver for SSD1306 
displays](https://github.com/ThingPulse/esp8266-oled-ssd1306)	

by:
 - Daniel Eichhorn, ThingPulse
 - squix78@gmail.com
 - https://thingpulse.com
 - Fabrice Weinberg: fabrice@weinberg.me
 
----------

#### **update: since: 19/10/2021**
This update we started using the [FastLED libary](https://github.com/FastLED/FastLED)

by:
- [Daniel Garcia](https://github.com/focalintent)
- [Mark Kriegsman](https://github.com/kriegsman)
- [Sam Guyer](https://github.com/samguyer)
- [Jason Coon](https://github.com/jasoncoon)
- [Josh Huber](https://github.com/uberjay)

----------
