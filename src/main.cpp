#include <Arduino.h>   //This libary adds some specific arduino functions


#include <IRremoteESP8266.h> //This import is the main libary to controll the IR led and receiver.
#include <IRsend.h>     //This import manages a specific part of the main IR libary
#include <IRrecv.h>     //This import manages a specific part of the main IR libary
#include <IRac.h>       //This import manages a specific part of the main IR libary
#include <IRtext.h>     //This import manages a specific part of the main IR libary
#include <IRutils.h>    //This import manages a specific part of the main IR libary


#include <assert.h> //This import contains a macro for debugging purpusses (used in the void setup to confirm the correct launch of the IR libary (sanity check))


#include <FastLED.h> //This libary manages the RGBLED's we used (replacing this libary might be a good idea)


#include <Wire.h> //this import, contains the I2c libary
#include "SSD1306Wire.h" //this manages the I2c connection  to the screen
#include "OLEDDisplayUi.h" //this imnport includes the 'driver' for the oled screen


#include <ESP8266WiFi.h> //this import includes the wifi libary: only the 2.4ghz band can be used!


#include <PubSubClient.h> //this import contains among other things the mqtt protocol

#define ledPin     2 //D4 The pin that Controlls the RGB LED's

#define CHIPSET     WS2811 //type of Individualy addressable LED's we use
#define NUM_LEDS    2 // currently we use 2 RGB LED's

#define idd '2'               //Here we define the ID of the 'gun', this shouild be set the same as the number of the 'gun' 
String idstring = "000010";   //Here we fill in the translated binary string of the ID (1=000001, 2=000010, 3=000011,...)

String CFFA = "0001";         //These strings are predifined binary strings for the teams.
String CGREEN = "0010";       //The strings are here to change the teams, and to compare against incomming messages.
String CBLUE = "0011";        // The C in the beginnen means that these strings are for when the guns are connected to the server.
String CRED = "0100";
String CWHITE = "0101";

String  Cown = "0001";        //this is the pre-set "own team", this can be changed in the code (not recomended) and can also be changed by using the server.

int totalbullets = 0;         //this is a counter that keeps track of all bullets shot.


boolean timeout = false;      //this is a boolean state, by default this is false, when connection with the server fails 5x, the state is changed to 'true'. it is used to prevent the gun to get stuk in a reconnect loop.

WiFiClient espClient;         //libary to use wifi
PubSubClient client(espClient); //libary to use mqtt
boolean connected = false;    // avariable to keep track if the gun is connected to the mqtt server, is mainly used to update the status on the screen and to  switch to different parts of the code.

const uint32_t kBaudRate = 115200; //bitrate 115200 is fairly standard in these type of projects.

const uint16_t kCaptureBufferSize = 1024; //standard settings of the IRremoteESP8266 libary 
const uint8_t kTimeout = 50;  
const uint16_t kMinUnknownSize = 12;
const uint8_t kTolerancePercentage = kTolerance;  // kTolerance is normally 25%

const uint16_t kIrLed = 15;  // ESP8266 GPIO pin to use. Recommended: 4 (D2) // using 15 (D8) // this defines the pin used to send the IR signals
const uint16_t kRecvPin = 14; // pin to receive ir data (D5) // Defines The pin used to receive data
String ownTeam = "FFA";  //Defines the basic configurgiration
SSD1306Wire display(0x3c, SDA, SCL); //configures the display (address and i�c) (the SDA = D2,  SCL = D1)
OLEDDisplayUi ui     ( &display );   
CRGB leds[NUM_LEDS];
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;  // Somewhere to store the results

IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

// Example of data captured by IRrecvDumpV2.ino
uint16_t MawData[] = { 1646 , 422 , 393, 393, 393, 393, 393, 393, 393, 786, 393, 786, 393, 786, 393, 786, 393, 1  , 2   ,3    ,4    ,5    ,6    ,7    ,8    ,9,10,11,12,13,5600}; //this is a "parody" of rawwData, It fuffils a simelair function, it sets the lenght and structure of the data neccesairy to send.
uint16_t rawData[] = { 1646 , 422 , 393, 393, 393, 393, 393, 393, 393, 786, 393, 786, 393, 786, 393, 786, 393, 1  , 2   ,3    ,4    ,5    ,6    ,7    ,8    ,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24, 5600}; //This sets the lenght and structure of the data neccesairy to send.
//uint16_t rawData[] =   { 1646 , 422 , 393, 393, 393, 393, 393, 393, 393, 786, 393, 786, 393, 786, 393, 786, 393, 393, 393 ,393  ,393  ,393  ,393  ,393  ,786  ,393, 393 ,393  ,393  ,393  ,393  ,393  ,786  ,393, 393 ,393  ,393  ,393  ,786 ,786  ,393  , 5600};

// binary codes of the 'legacy guns', "FFAGUN" is NOT legacy. These guns can be used without a server. For improved readability this part can be put in an include document. 
String BlueGunx1    = "00000001 00000001 00000110"; 
String BlueGunx2    = "00000001 00000010 00000111";
String BlueGunx3    = "00000001 00000011 00001000";
String RedGunx1     = "00000010 00000001 00000111";
String RedGunx2     = "00000010 00000010 00001000";
String RedGunx3     = "00000010 00000011 00001001";
String GreenGunx1   = "00000011 00000001 00001000";
String GreenGunx2   = "00000011 00000010 00001001";
String GreenGunx3   = "00000011 00000011 00001011";
String WhiteGunx1   = "00000100 00000001 00001001";
String WhiteGunx2   = "00000100 00000010 00001011";
String WhiteGunx3   = "00000100 00000011 00001100";
String FFAGUN       = "00000101 00000001 00001100";

//codes for the teams. Notice: these are the "legacy" codes, these will not be used when using a server.
String Blue   =   "001";
String Red    =   "010";
String Green  =   "011";
String White  =   "100";
String FFA    =   "101";
String eigenteam = "101";

// ammo amounts gun epending on gun type. This is also only for "offline games" with optionally also the legacy guns
int bullet1x = 6;
int bullet2x = 3;
int bullet3x = 1;

//set-up for default team and gun (legacy)
int teams = 0;
String gun = FFAGUN;
int guns = 100;

//codes for the damage also used in server games
String Damagex1   = "01";
String Damagex2   = "10";
String Damagex3   = "11";

//buttons
const uint16_t buzzer = 0;              //D3    //As the name sugest, this is the pin used for the buzzer (or in our case the transisor driving the buzzer)
const uint16_t ChangeTeams_Button = 12; //d6    //This button as function to change the team when in a non connected status.
const uint16_t button_Shoot = 13;       //D7    pushbutton connected to digital pin D7. this button activates the shoot code.
const uint8_t ChangeGuns_Button = 16;   //D0    button to change the gun type //this button changes the type of gun when not connected to a server
const uint8_t Reload_Button = A0;       //A0    reload button on A0  //used to reload the bullets variable.

int val = 0;      // variable to store the read value

//some standard values
int bullets     = 6;        //amount of bullets for default gun
int Health      = 9;        //you start with 9hp (also true for server games)
int bullet_type = bullet1x; //sets the amount of bullets for each 'clip'
int time_wait   = 0;        //cooldown for actions
int gun_delay   = 10;       //default cooldown for default gun

//wifi setup
const char* ssid = "NETGEAR";             //fill in the 2.4ghz wifi name 
const char* password = "";                //fill in the wifi password
const char* mqtt_server = "192.168.1.5";  //the address of the mqtt-server NOTICE: this is a local ip address, use the same network on the server.

//mqtt setup (part 1)
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

String tempstring = Cown + Damagex1 + idstring;//the standaard string to send to the server: ownteamID + damage(type) + Id

//wifi loading funcion
void setup_wifi() {

  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // As long as we are not connected, the sp wil try to reconnect REMINDME !!!
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//mqtt call back function (this function should be used to pull data from the server), listens on MQTT connection for incomming messages: (channel, message length)
void callback(char* topic, byte* payload, unsigned int length) {                              
  
  //For debugging, the esp prints whenever a message has arived, on with topic/channel it did.
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  //This statement checks if the channel is "time", if it is, the ESP will send a message to the server as follows: ID # Health # bullets # totalbullets. If it is not, the message is passed along
  if (topic == "time"){
    snprintf (msg, MSG_BUFFER_SIZE, "%ld#%ld#%ld#%ld", idd, Health, bullets, totalbullets);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("2", msg);     //Change the "2" to the correct ID of the ESP (this is important for communication with the server)
  }

                                  //If the topic is the same as its own ID, the message is regarded as a configuration. The esp will configure its stelf to the given parameters.
  if (topic[0] == idd){           //This way the ID can not be more than 9, first character of he topic has to be the same as the Id
    
    String Cteam = "";            //Firstly some 'fresh' Strings are made and cleared REMINDME !!!
    String Cdam = "";
    Cteam.clear();
    Cdam.clear();

   
    switch (payload[0])           //This switch statment tries to match the first character of the message with a certain case, This field will define to wich team the gun belongs.
    {
    case '1':
      Cteam = CBLUE;              //Sets the team value (the binary string) via the presets
      leds[1] = CRGB::Blue;       //Sets the correct collour of the RGBLED
      ownTeam = "CBLUE";          //used for debugging, nor real "purpose"
    case '2':
      Cteam = CRED;
      leds[1] = CRGB::Red;
      ownTeam = "CRED";
    case '3':
      Cteam = CGREEN;
      leds[1] = CRGB::Green;
      ownTeam = "CGREEN";
      
    case '4':
      Cteam = CWHITE;
      leds[1] = CRGB::White;
      ownTeam = "CWHITE";
      
    case '5':
      Cteam = CFFA;
      leds[1] = CRGB::Purple;
      ownTeam = "CFFA";
    default:                      //Sets the default values (it falls back to '1')
      Cteam = CFFA;
      leds[1] = CRGB::Purple;
      ownTeam = "CFFA";
    }


    switch (payload[1])           //Here the second haracter is compared against the cases. Here the damage type (1x, 2x, 3x) gets set. 
    {                             //The bullettype (the amount of bullets in a clip) gets set as well.
    case '1':
      Cdam = Damagex1;            //The damage gets set via the presets of damage type
      bullet_type = 9;            //The bullettype is set  to custom values, it can be overridden by the next function.
    case '2':
      Cdam = Damagex2;
      bullet_type = 6;   
    case '3':
      Cdam = Damagex3;
      bullet_type = 3;
    default:
      Cdam = Damagex1;
      bullet_type = 9;
    }


    if (payload[3])               //If the payload contains 3 charachers, the third one will overide
    {                             //the earlier defined bullet type with the value in payload[3]
      bullet_type = payload[3];
      if (payload[3] = '0')
      {
        bullet_type = 10;
      }
      
    }
    
    tempstring.clear();                         //The default send string is cleared so that it can be build with new values.
    tempstring = Cteam + Cdam + idstring;       //This string contains all data that wil be send over IR, you can compare to the realier defined default string
    //String tempstring = Cown + Damagex1 + idstring;
    Serial.println(" received configuration "); //To ease debugging, the esp prints to serial whenever it receives a configuration. 
  }
}

//display functions: this configures the screen overlay, and sets some values: like font, text alignment,... This is not actively used.
void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 0, String(millis()));
}

//this function contains information about the first 'screen tab'
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  String status = "Not connected";
  if (connected){
   status.clear();
   status.concat("Connected");
  }
  display->drawString(0 + x, 10 + y, status);

}
void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_24);
  
  String hp = "HP: ";
  hp.concat(Health);
  display->drawString(0 + x, 10 + y, hp);
 
}
void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y

  display->setTextAlignment(TEXT_ALIGN_LEFT);
   display->setFont(ArialMT_Plain_24);
  String team = "Team: " + ownTeam;
  display->drawString(0 + x, 10 + y, team);
  
}
void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_24);
  
  String Ammo = "Ammo: ";
  Ammo.concat(bullets);
  display->drawString(0 + x, 10 + y, Ammo);
}
//forms an array of the frames
FrameCallback frames[] = { drawFrame1, drawFrame2, drawFrame3,drawFrame4};


//the void setupconfigures the hardware side of things, and gives some initial values to some other functions. also starts connection wih wifi, mqtt,... and launches some other software.
void setup() {
  pinMode(kIrLed, OUTPUT);                // (D8) set the digital pin for the IR LED as output
  digitalWrite(kIrLed,LOW);               // makes shure the Ir led starts while truned off.
  pinMode(buzzer, OUTPUT);                // (D3) sets the digital pin 15 as output
  pinMode(button_Shoot, INPUT);           // (D7) sets the digital pin 7 as input
  pinMode(ChangeGuns_Button,INPUT);       // (D0) sets the digital pin 16 as input
  pinMode(Reload_Button,INPUT);           // (A0) sets the analog pin 0 as inoput (the pin is used as a digital pin with cutoff values)
  pinMode(ChangeTeams_Button, INPUT);     // (D6) sets the digital pun 12 as input
  irsend.begin();                         // starts initialises the IRsend libary
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);       //initialises the serial connection (this is used for debugging)
  assert(irutils::lowLevelSanityCheck() == 0);            //this runs a check to confirm correct behaviour of the IR urtils libary (part of the greater IR remote lib)
  FastLED.addLeds<WS2812B, ledPin, GRB>(leds, NUM_LEDS);  //configures the RGB LEDs , the type , the number, and the pin
  
  leds[1] = CRGB::Purple;                 // sets the default values of the 2 LED's
  leds[0] = CRGB::Green;
  
  setup_wifi();                           // runs the wifi setup
  client.setServer(mqtt_server, 1883);    // configures the mqtt server
  client.setCallback(callback);           // configures the call back function


  Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
#if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH
  irrecv.setTolerance(kTolerancePercentage);  // Override the default tolerance.
  irrecv.enableIRIn();                        // Start the receiver

  ui.setTargetFPS(10);   //sets the fps of the screen

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, 4);
  ui.init();
  
}


//function to keep mqtt connection alive
void reconnect() {
  // Loop until we're reconnected
  int i = 0;
  while ((!client.connected())and(!timeout)) { //tries connection for a few times where after it stops for serverles operation.
    Serial.print("Attempting MQTT connection...");
    // Replace the client ID with 'any' chosen string.
    String clientId = "ESP8266Client-02";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("2", "connected!");  //replace "2" with the esp ID (idd)
      // ... and resubscribe
      connected = true;
      client.subscribe("2");              //replace "2" with the esp ID (idd)
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      i++;
    }
    if (i > 4){
      timeout = true;
    }
  }
}

//function to output sound please change this to something less annoying, we can only controll whether or not the buzzer is powerd or not.
void buzzerfun(int repeat){
  int i = 0;
  while (i < repeat*2)
  {
    digitalWrite(buzzer, LOW);
    delay(5);
    digitalWrite(buzzer, HIGH);
    delay(5);
    digitalWrite(buzzer, LOW);
    delay(5);
    digitalWrite(buzzer, HIGH);
    i++;
  }
  digitalWrite(buzzer, LOW);
  
}

//this function changes moves the player to a different team. (this is only possible while not connected to a server)
String ChangeTeams(int teams){ 
  gun.clear();
  eigenteam.clear();
  ownTeam.clear();
  teams ++;                     // the gun cycles through an array of teams
  if (teams == 5){
    teams = 0;
  }


  String temp = "";             // the team is decided by an "index int", and via a swich statement the correct configuration is chosen. (optimisation is possible)
  switch (teams)
  {
    case 0:                     // when the indew number is the same of the case
      leds[1] = CRGB::Purple;   // sets the colour of the 2th LED (the team LED)
      eigenteam = FFA;          // sets the ownteam string to the changed value, this is used to determine if a shot was friendly or not.
      gun = FFAGUN;             // sets the string that has to be fired
      guns = 100;               // sets the gun value (mostly used to keep track)
      temp = eigenteam + gun;   // makes a temperary string that will be used in this functon to return a value (this is used for debugging)
      ownTeam.concat("FFA");    // sets the ownTeam variable, also mostly for debugging 
      break;
    case 1:
      leds[1] = CRGB::Blue;
      eigenteam = Blue;
      gun = BlueGunx1;
      guns = 10;
      temp = eigenteam + gun;
      ownTeam.concat("Blue");
      break;
    case 2:
      leds[1] = CRGB::Red;
      eigenteam = Red;
      gun = RedGunx1;
      guns = 20;
      temp = eigenteam + gun;
      ownTeam.concat("Red");
      break;
    case 3:
      leds[1] = CRGB::Green;
      eigenteam = Green;
      gun = GreenGunx1;
      guns = 30;
      temp = eigenteam + gun;
      ownTeam.concat("Green");
      break;
    case 4:
      leds[1] = CRGB::White;
      eigenteam = White;
      gun = WhiteGunx1;
      guns = 40;
      temp = eigenteam + gun;
      ownTeam.concat("White");
      break;
    default:
      leds[1] = CRGB::Purple;
      eigenteam = FFA;
      gun = FFAGUN;
      guns = 100;
      temp = eigenteam + gun;
      ownTeam.concat("FFA");
      break;
  }
  delay(60);
  return temp;                  // returns the 'temp' string
}


void prepare_shot(String shot){ // this function prepares, and fires the shot, first it will create a string to send in binary, then it will translate the string to a raw array, wich will be send as is
  String list = "01";           // this is an array of sorts to compare values against.
  u_int i = 0;                  // this is is used instead of a for loop.
  int array_place = 17;         // defines the starting poition in the raw- or mawdata
  String temp = "";             // initialises the string
  
  // when the esp is not connected, this piece of the code is used.
  while ((i <= shot.length()) and (!connected)) 
  {
    
    // This part builds the array in a correct way, it reads a binary string, if it encounters a 1: it adds 786, if it encounters a 0 it places a 393  
    if (shot.charAt(i) == list[1]){
      rawData[array_place] = 786;
      array_place++;
      temp = temp + 786;
    }
    if (shot.charAt(i) == list[0])
    {
      rawData[array_place] = 393;
      array_place++;
      temp = temp + 786;
      }
    i++;


  }

  // when the is IS connected, this part of the code is executed. (the difference is the shorter templkate string, MawData is shorter than RawData)
  while ((i <= shot.length())and(connected))
  {
    // This part builds the array in a correct way, it reads a binary string, if it encounters a 1: it adds 786, if it encounters a 0 it places a 393
    if (shot.charAt(i) == list[1]){
      MawData[array_place] = 786;
      array_place++;
      temp = temp + 786;
    }
    if (shot.charAt(i) == list[0])
    {
      MawData[array_place] = 393;
      array_place++;
      temp = temp + 786;
          }
    i++;
  }


  Serial.print("Shoot: ");            //while the esp is connected to a serial monitor, this wil display when the esp shoots: it is usefull to get more insight on what the esp is doeing
  Serial.println(temp);
  

  if (connected)                      //when the esp is connected, it shoots the MawData array, else, it shoots the rawData array and waits for a short while to prevent spam shooting
  {
    irsend.sendRaw(MawData,30,38);
  }else{  
  irsend.sendRaw(rawData,42,38);
  }
  time_wait = (10 / bullet_type)*15;
}


//as the name sugests, this function stands in to calculate the received damage, it also calls the buzzer funcion for audible feedback when shot
void get_damage(String temp){         
  // First this function tests what type of damage is received, whereafter it also suptracts the approirate amount of health.
  // it also gives a shot beeb after being shot, this beep is differenbt for each case, when the HP dops below 1, it once again wil give a longer beeb.
  // to aid debugging, there are also serial prints here to validate propper functionning of the gun
  if (temp == Damagex1)         
  {
    Health --;
    buzzerfun(1);
  }
  if (temp == Damagex2)
  {
    Health -= 2;
    buzzerfun(2);
  }
  if (temp == Damagex3)
  {
    Health -= 3;
    buzzerfun(3);
  }
  if (Health < 1)
  {
    Serial.println("Dead");
    buzzerfun(5);
  } 
  Serial.println(Health);
  Serial.println(temp);
}


//as the name sugests, this function stands in to calculate the received damage, it also calls the buzzer funcion for audibel feedback when shot. This overload is used when the gun is connected to the server
//
//needs the damage string, and the int of the enemy
void get_damage(String temp, int enemy){ 
  // First this function tests what type of damage is received, whereafter it also suptracts the approirate amount of health.
  // it also gives a shot beeb after being shot, this beep is differenbt for each case, when the HP dops below 1, it once again wil give a longer beeb.
  // to aid debugging, there are also serial prints here to validate propper functionning of the gun
  // lastly it also sends a message to the server: the message identifies the esp, the remaining health, the total amount of bullets shot, and the enemy that has shot the player.
  if (temp == Damagex1)
  {
    Health --;
    buzzerfun(1);
  }
  if (temp == Damagex2)
  {
    Health -= 2;
    buzzerfun(2);
  }
  if (temp == Damagex3)
  {
    Health -= 3;
    buzzerfun(3);
  }
  if (Health < 1)
  {
    Serial.println("Dead");
    buzzerfun(5);
  } 
  Serial.println(Health);
  Serial.println(temp);

  if (connected)
  {
    snprintf (msg, MSG_BUFFER_SIZE, "%ld#%ld#%ld#%ld#%ld", idd, Health, bullets, totalbullets,enemy);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("2", msg); //change the '2' to the ID of the esp gun!!! this changes the channel to it's own.
  }  
}


//this function is to "decode" the data, since it is encoded "raw" (manchester encoding?)
// this requires the rewceived array.
void decodeData(uint16_t * Datass){
  //this part is a mess please continue wwith patience

  
  String binairy_code = "";   	  // at the start we initialise a string to store the 'translated' array
  int i = 17;                     // sets the start place for decoding, the first 17 characters in the array are always the same and don't contain anny data.
  Serial.print("Eigen team: ");   // a bit of feedback for debugging
  
  
  
  if (!connected)                 // this opart is the code for when the gun isn't connected to a server.
  {
    Serial.println(eigenteam);
    while (i<41)                  // this part decodes the received values to binary, the first values of the array are static, and already present in this code, the part between place 17 and 41 (not inclusive, also we are programmers: indexes start at 0)
    {
      
      uint16_t value = Datass[i]; 
      if (value > 600)                  //if the value in the array exeedes 600 it is a binnary 1, if not, it is a binnary 0
      {
        binairy_code.concat("1");
      }else {
        binairy_code.concat("0");
      }
      i++;                              //steps a place forward in the array
      
    }



    Serial.print("code: ");
    Serial.println(binairy_code);
    Serial.print("code damage: ");                  //this gets a substring which contain the 'damage' digets to be printed, this will be seen again in the next short while
    
    // Here we isolate a few values from the received string, such as: the team code(temp) and the damage(damage).
    Serial.println(binairy_code.substring(15,17));      
    String temp = binairy_code.substring(5,8);      //this gets the team value from the string
    String damage = binairy_code.substring(15,17);  //this (once again) will remember the damage value
    // jsut fome prints to ease testing/developping
    Serial.println(damage);
    Serial.println(temp);
    
    // here we compare the team of the shooter, and the team of this esp 
    if ((temp == Blue) or (temp == Red) or (temp == Green) or (temp == White)) //here the code checks if the team is valid, and not ffa(because of the different rules)
    {
      if ((temp != eigenteam) and (eigenteam != FFA)){ //if the team id is valid, and not ffa, amd your team id isn't the same or FFA, the get damage function is called
        get_damage(damage); //passes the damage data to the get_damage function
      }
    }
    if (temp == FFA)        //if both teams are in the FFA team, the shot will count, and the damage data will be send trough.
    {
      if (eigenteam == FFA)
      {
        get_damage(damage);
      }
    }
  }
  else  //AKA if the esp IS connected this part is run.
  {
    // this code is largly the same as the code above
    
    while (i<28) //this part decodes the received values to binary, the first values of the array is static, and already present in this code, the part between place 17 and 41 (not inclusive, also we are programmers: indexes start at 0)
    {

      uint16_t value = Datass[i]; 
      if (value > 600)              //if the value in the array exeedes 600 it is a binnary 1, if not, it is a binnary 0
      {
        binairy_code.concat("1");
      }else {
        binairy_code.concat("0");
      }
      i++;
    }
    

    // Here a few strings are initialised with data froom the array
    String temp = binairy_code.substring(0,3);      //the team of the shooting gun
    String damage = binairy_code.substring(4,6);    //The damge type of the shooting gun
    String enemyS = binairy_code.substring(7,12);   //The enemy id in binary
    char enemy[6];                                  //An array to store the binary ID
    
    
    enemyS.toCharArray(enemy,5);                    //The array gets filed
    int EnemyId = strtol(enemy, 0, 2);              //The array gets converted to an int
    if (temp == Cown)                               //Checks if teams are equal
    {
      if (temp == CFFA)                             //checks if both teams are CFFA, if so the shot is valid, and wil be piped through to get damage with te enemy ID
      {
        get_damage(damage, EnemyId);
      }
      
    }else{
      if ((temp != CFFA) and (Cown != CFFA)){   //If both teams are differend and neither is part of CFFA,
        get_damage(damage, EnemyId);            //the hit is also counted and will be send trough to the get_damage function with the damage code and the enemy ID
      }
    }
  }
}


//This function is used to change the gun type when in "ofline/legacy" mode.
void changeGuns (){   // function that changes the gun type, it uses the switch method
  guns ++;            // here it increments the gun index.
  switch (teams)      // guns are 'different' for each team, so it first checks what team the gun currently is part of.
  {
  case 0:                     //If the gun is currently part of the FFA team, the gun cannot be swithched to another gun
    gun.clear();
    gun = FFAGUN;
    bullets = bullet1x;
    bullet_type = bullet1x;
    break;
  case 1:                     
    switch (guns)             //here it checks the gun index, this wil only run this switch statement after having determened in what team the gun currently is in.
    {
    case 11:                  //each team has valid gun indexes according to: their TeamID and a range between 1 to 3 (inclusive)
      gun = BlueGunx2;        //first the gun is given the binary string defined in the variable
      bullets = bullet2x;     //reloads the bullets
      bullet_type = bullet2x; //defines the amount of bullets per clip
      break;
    case 12:
      gun = BlueGunx3;
      bullets = bullet3x;
      bullet_type = bullet3x;
      break;
    case 13:
      gun = BlueGunx1;
      bullets = bullet1x;
      guns = 10;              // here it sets the 'guns' index as 10 so that when the function is rerun, the next type will still make sense 
      bullet_type = bullet1x;
      break;

    default:                  // here we have a catch to redirect undefined behaviour so that the data makes sence once again, the variabls become reset to a valid state.
      guns = 10;
      bullets = bullet1x;
      gun = BlueGunx1;
      teams = 1;
      bullet_type = bullet1x;
      eigenteam = Blue;
      break;
    }
    break;
  case 2:
    switch (guns)
    {
    case 21:
      gun = BlueGunx2;
      bullets = bullet2x;
      bullet_type = bullet2x;
      break;
    case 22:
      gun = BlueGunx3;
      bullets = bullet3x;
      bullet_type = bullet3x;
      break;
    case 23:
      gun = BlueGunx1;
      bullets = bullet1x;
      bullet_type = bullet1x;
      guns = 10;
      break;

    default:
      guns = 20;
      gun = RedGunx1;
      bullets = bullet1x;
      teams = 2;
      eigenteam = Red;
      bullet_type = bullet1x;
      break;
    }
    break;
  case 3:
    switch (guns)
    {
    case 31:
      gun = GreenGunx2;
      bullets = bullet2x;
      bullet_type = bullet2x;
      break;
    case 32:
      gun = GreenGunx3;
      bullets = bullet3x;
      bullet_type = bullet3x;
      break;
    case 33:
      gun = GreenGunx1;
      bullets = bullet1x;
      bullet_type = bullet1x;
      guns = 30;
      break;

    default:
      guns = 30;
      gun = GreenGunx1;
      bullets = bullet1x;
      bullet_type = bullet1x;
      teams = 3;
      eigenteam = Green;
      break;
    }
    break;
  case 4:
    switch (guns)
    {
    case 41:
      gun = WhiteGunx2;
      bullets = bullet2x;
      bullet_type = bullet2x;
      break;
    case 42:
      bullets = bullet3x;
      gun = WhiteGunx3;
      bullet_type = bullet3x;
      break;
    case 43:
      bullets = bullet1x;
      gun = WhiteGunx1;
      guns = 40;
      bullet_type = bullet1x;
      break;

    default:
      bullets = bullet1x;
      guns = 40;
      gun = WhiteGunx1;
      teams = 4;
      eigenteam = White;
      bullet_type = bullet1x;
      break;
    }
    break;
          
  default: //stops function if no valid team id is given
    break;
  }  
}


//the main function loop, this is function loops forever, and calls other functions
void loop() {

  ui.update();      // this function updates the display, and updates the pages if hey have been edited
  FastLED.show();   // this function updates the RGBLEDs so that collor changes become visable.
  
  
  if ((!client.connected()) and (!timeout)) {
    reconnect();    // the function to reconnect to the server if connection suddenly dropps
  }
   client.loop();   // listens on mqtt

  
  if ((Health > 0)and(!connected))          // as long as you have health you are part of the game, when your hp is drained, the gun doesn't read inputs anymore.
  {
    if (analogRead(Reload_Button) == 1024)  // if the reloadbutton pin 1024 recaives as a value (happens when bridged to 3.3V) the reload function wil restock the bullets
    {
      bullets = bullet_type;                // this overdides the remaing bullets with the clip siz (reloaing)
      Serial.print("reload gun: ");         // for testing, a few serial prints to make shure it works
      Serial.println(bullets);
      Serial.println(analogRead(Reload_Button));
      time_wait = 40;                       // a preset time to wait (this is purly artificial waithing, this way you can cannot spam actions (appart for reloading))
      
    }
    
    
    switch (Health)//just a check to change the led colour depenging on the remaining health points
    {
    case 1:
    case 2:
    case 3:
      leds[0] = CRGB::Red;
      break;
    case 4:
    case 5:
    case 6:
      leds[0] = CRGB::Orange;
      break;
    case 7:
    case 8:
    case 9:
      leds[0] = CRGB::LimeGreen;
      break;
    default:
      break;

    }
    FastLED.show();                        //refresh led's

    if (digitalRead(button_Shoot) == HIGH) //readout of pin to detect if button is pressed, and will if so run the function "gun" (aka, it fires the "laser")
    {
      if (bullets>=1){
        prepare_shot(gun);                      //runs the prepare shot with a selected template 
        bullets--;                              //bullets remaining reduced everytime a shot is fired by the gun.
        Serial.print("Remaining bullets: ");
        Serial.println(bullets);                
        buzzerfun(2);                           //the gun gives a short beeb when fired.
        time_wait = (100/(bullet_type*3))*20;
        
      }
    }
    
    
    if (digitalRead(ChangeTeams_Button) == HIGH)        //reads the teams pin, if the pin is high, the gun will change team, this is done by running the change teams function
    {
      Serial.println("change team");
      String tijdelijk = ChangeTeams(teams);            // here a temperary value is made to store the returned string
      teams ++;                                         //team is automaticaly incremented
      if (teams > 4)                                    //if the team is 5 or more, the team id wil be reset to 0
      {
        teams = 0;
      }
      eigenteam = tijdelijk.substring(0,3);             //the own team string is reassigned with a new value selected from the returned string
      gun = tijdelijk.substring(3,tijdelijk.length());  //the 'gun' string is also reassigned to the newly selected value
      
    
      Serial.print("gun: ");
      Serial.println(gun);
      Serial.print("team: ");
      Serial.println(eigenteam);  //a bunch of debugging stuff 
      
      time_wait = 30;              // to prevent accidental double presses.
      
      
    }
    
    
    if ((digitalRead(ChangeGuns_Button) == HIGH)and (eigenteam != FFA)) //reads the change guns pin, and will attempt to change 'gun type' if it is possible in the given team
    {
      changeGuns();                //The change guns function is run if the own team isn't FFA (FFA only contains 1 type), the corresponding amount of bullets are also set in this function.
           
      Serial.print("Gun: ");       //there is also a lot of testing info present to check correct behaviour.
      Serial.println(gun);         
      Serial.print("Gun ID: ");
      Serial.println(guns);        
      Serial.print("Team: ");
      Serial.println(eigenteam);
      time_wait = 30;               //here to prevent accidental spam clicking
      
    }    
    
    
    if (irrecv.decode(&results)) { //this code will try to read/decode the incomming  signals, and will also post them in the serial interface.
                                   //this will also call the function to take damage (and to recognise the other gun).
    
      Serial.println(resultToSourceCode(&results));      //prints the received array partialy
      Serial.println();
      //this is part of some example code from the IRremote libary
      uint16_t * Datass = resultToRawArray(&results);    // Blank line between entries
      decodeData(Datass);
      yield();             // Feed the WDT (again)  
    }
              // the time waits are in actuality a few loop runs that some functions do not react.
  }
  
  
  if ((Health > 0)and(connected))           // as long as you have health you are part of the game, when your hp is drained, the gun doesn't read inputs anymore.
  {
    if (analogRead(Reload_Button) == 1024)  // if the button connected to the button is closed, the reload function wil run
    {
      bullets = bullet_type;                // the bullets are reset to the same value as the clip size, future expantion could ad a system of limmited clips. 
      Serial.print("reload gun: ");         // some debugg/testing features
      Serial.println(bullets);
      Serial.println(analogRead(Reload_Button));
      time_wait = 50;
      
    }
    
    
    switch (Health)//just a check to change the led colour depenging on the remaining health points
    {
    case 1:
    case 2:
    case 3:
      leds[0] = CRGB::Red;
      break;
    case 4:
    case 5:
    case 6:
      leds[0] = CRGB::Orange;
      break;
    case 7:
    case 8:
    case 9:
      leds[0] = CRGB::LimeGreen;
      break;
    default:
      break;

    }
    FastLED.show(); // refreshes the LED's

    if (digitalRead(button_Shoot) == HIGH) //readout of pin to detect if button is pressed, and will if so run the function "gun" (aka, it fires the "laser")
    {
      if (bullets>=1){
        prepare_shot(tempstring);             // the function is run with the srtring containing the info for a game connected to the server
        bullets--;                            //when shooting, the bullets wil go down by one everytime the function is run
        Serial.print("Remaining bullets: ");  
        Serial.println(bullets);
        buzzerfun(2);                         //shooting makes the gun give a short beeb
        totalbullets++;                       //everytime this function is run, the total amount of bullets goes up, this is done so that an accuracy can be calculated
        time_wait = (100/(bullet_type*3))*20;
        
      }
    }
    
    
    if (irrecv.decode(&results)) { //this code will try to read/decode the incomming  signals, and will also post them in the serial interface.
                                   //this will also call the function to take damage (and to recognise the other gun).

      //this is a part of some example code from the IRremote libary
      Serial.println(resultToSourceCode(&results));
      Serial.println();
      uint16_t * Datass = resultToRawArray(&results);    // Blank line between entries
      decodeData(Datass);
      yield();             // Feed the WDT (again)
      
    }
        //an complete run is fufilled, so the 'inactive' counter goes down 1.
  }
  
  
  
  if (Health <= 0)  //this part runs if the health points run out.
  {
    delay(200);
    FastLED.clear();              //should trun the led's off
    Serial.println("u dead m8");  //this is the message you get when you die
    
    FastLED.clear();
    delay(50);
    leds[0] = CRGB::Red; //RGBLEDs should flicker red (altough they do all different kinds of things exept that)
    FastLED.show();
    
  }
  time_wait--;
  delay(200);
  Serial.println("this is a loop");
}
