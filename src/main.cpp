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
const char ID_TOP = '2';

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
uint16_t MawData[] = { 1646 , 422 , 393, 393, 393, 393, 393, 393, 393, 786, 393, 786, 393, 786, 393, 786, 393, 1  , 2   ,3    ,4    ,5    ,6    ,7    ,8    ,9,10,11,5600}; //this is a "parody" of rawwData, It fuffils a simelair function, it sets the lenght and structure of the data neccesairy to send.
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
    client.publish("1", msg);
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
  FastLED.addLeds<WS2812B, ledPin, RGB>(leds, NUM_LEDS);  //configures the RGB LEDs , the type , the number, and the pin
  
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
    String clientId = "ESP8266Client-01";
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
  return temp;
}


void prepare_shot(String shot){ //this function prepares, and fires the shot, first it will create a string to send in binary, then it will translate the string to a raw array, wich will be send as is
  String list = "01";
  u_int i = 0;
  int array_place = 17;
  String temp = "";
  while ((i <= shot.length()) and (!connected))
  {
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
  while ((i <= shot.length())and(connected))
  {
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
    Serial.print("Shoot: ");
  Serial.println(temp);
  if (connected)
  {
    irsend.sendRaw(MawData,30,38);
  }else{  
  irsend.sendRaw(rawData,42,38);
  }
  time_wait = (10 / bullet_type)*15;
}

void get_damage(String temp){ //as the name sugests, this function stands in to calculate the received damage, it also calls the buzzer funcion for audibel feedback when shot
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
void get_damage(String temp, int enemy){ //as the name sugests, this function stands in to calculate the received damage, it also calls the buzzer funcion for audibel feedback when shot
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
    client.publish("1", msg);
  }  
}

void decodeData(uint16_t * Datass){ //this function is to "decode" the data, since it is encoded "raw"
  String binairy_code = "";
  int i = 17;
  int iteration = 1;
  Serial.print("Eigen team: ");
  if (!connected)
  {
    Serial.println(eigenteam);
    while (i<41) //this part decodes the received values to binary, the first values of the array is static, and already present in this code, the part between place 17 and 41 (not inclusive, also we are programmers: indexes start at 0)
    {
      String bin_old = binairy_code;
      uint16_t value = Datass[i]; 
      if (value > 600)
      {
        binairy_code = bin_old + "1";
      }else {
        binairy_code = bin_old + "0";
      }
      iteration++;
      i++;
      
    }
    Serial.print("code: ");
    Serial.println(binairy_code);
    Serial.print("code damage: "); //this gets a substring which contain the 'damage' digets to be printed, this will be seen again in the next short while
    Serial.println(binairy_code.substring(15,17));
    bool b = true; 
    while (b)
    {
    
      String temp = binairy_code.substring(5,8); //this gets the team value from the string
      String damage = binairy_code.substring(15,17);//this (once again) will remember the damage value
      Serial.println(damage);
      Serial.println(temp);
      if (((temp == Blue) or (temp == Red) or (temp == Green) or (temp == White)) and (temp != FFA)) 
      // if (((temp == Blue) or (temp == Red) or (temp == Green) or (temp == White)) and (temp != eigenteam) and (eigenteam != FFA)) 
      {
        if ((temp != eigenteam) and (eigenteam != FFA)){
          get_damage(binairy_code.substring(15,17));
        }
        b = false;
      }
      if (temp == FFA)
      {
        if (eigenteam == FFA)
        {
          get_damage(damage);
        }
        b = false;
      }
      b = false;
    }
  }
  else
  {
    while (i<28) //this part decodes the received values to binary, the first values of the array is static, and already present in this code, the part between place 17 and 41 (not inclusive, also we are programmers: indexes start at 0)
    {
      String bin_old = binairy_code;
      uint16_t value = Datass[i]; 
      if (value > 600)
      {
        binairy_code = bin_old + "1";
      }else {
        binairy_code = bin_old + "0";
      }
      iteration++;
      i++;
    }
    
    Serial.println(binairy_code.substring(0,4));

    String temp = binairy_code.substring(0,3);
    String damage = binairy_code.substring(4,6);
    String enemyS = binairy_code.substring(8,10);
    char enemy[4];
    enemyS.toCharArray(enemy,5);
    int meh = strtol(enemy, 0, 2);
    if (temp == Cown)
    {
      if (temp == CFFA)
      {
        get_damage(damage, meh);
      }
      
    }else{
      if (temp != CFFA){
        get_damage(damage, meh);
      }
    }
  }
}



void changeGuns (){ //function that changes the gun type, it uses the switch method
  guns ++;
  switch (teams) // guns are 'different' for each team, so it first checks what team the gun currently is part of.
  {
  case 0:
    gun.clear();
    gun = FFAGUN;
    bullets = bullet1x;
    bullet_type = bullet1x;
    break;
  case 1:
    switch (guns)
    {
    case 11:
      gun = BlueGunx2;
      bullets = bullet2x;
      bullet_type = bullet2x;
      break;
    case 12:
      gun = BlueGunx3;
      bullets = bullet3x;
      bullet_type = bullet3x;
      break;
    case 13:
      gun = BlueGunx1;
      bullets = bullet1x;
      guns = 10;
      bullet_type = bullet1x;
      break;

    default:
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
          
  default:
    break;
  }
  
}


void loop() {

  ui.update();
  FastLED.show();
  if ((!client.connected()) and (!timeout)) {
    reconnect();
  }
   client.loop();

  
  if ((Health > 0)and(!connected)) // as long as you have health you are part of the game, when your hp is drained, the gun doesn't read inputs anymore.
  {
    //int Reload_Button_Value = digitalRead(Reload_Button);
    //Serial.println(Reload_Button_Value);
    //Serial.println(analogRead(Reload_Button));
    if (analogRead(Reload_Button) == 1024)
    {
      bullets = bullet_type;
      Serial.print("reload gun: ");
      Serial.println(bullets);
      Serial.println(analogRead(Reload_Button));
      time_wait = 40;
      

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
    FastLED.show();

    if (digitalRead(button_Shoot) == HIGH) //readout of pin to detect if button is pressed, and will if so run the function "gun" (aka, it fires the "laser")
    {
      if (bullets>=1){
        prepare_shot(gun);
        bullets--;
        Serial.print("Remaining bullets: ");
        Serial.println(bullets);
        buzzerfun(2);
      }
    }
    if (digitalRead(ChangeTeams_Button) == HIGH) //reads the teams pin, if the pin is high, the gun will change team, this is done by running the change teams function
    {
      Serial.println("change team");
      String tijdelijk = ChangeTeams(teams);
      teams ++;
      if (teams > 4)
      {
        teams = 0;
      }
      eigenteam = tijdelijk.substring(0,3);
      gun = tijdelijk.substring(3,tijdelijk.length());
      Serial.print("gun: ");
      Serial.println(gun);
      Serial.print("team: ");
      Serial.println(eigenteam);
      time_wait = 5; 
      
      
    }
    if ((digitalRead(ChangeGuns_Button) == HIGH)and (eigenteam != FFA)) //reads the change guns pin, and will attempt to change 'gun type' if it is possible in the given team
    {
      changeGuns();
           
      Serial.print("Gun: ");
      Serial.println(gun);
      Serial.print("Gun ID: ");
      Serial.println(guns);
      Serial.print("Team: ");
      Serial.println(eigenteam);
      time_wait = 5; 
    }    
    if (irrecv.decode(&results)) { //this code will try to read/decode the incomming  signals, and will also post them in the serial interface.
                                   //this will also call the function to take damage (and to recognise the other gun).
    
      Serial.println(resultToSourceCode(&results));
      Serial.println();
      uint16_t * Datass = resultToRawArray(&results);    // Blank line between entries
      decodeData(Datass);
      yield();             // Feed the WDT (again)
      //String hit = resultToTimingInfo(&results);
    }
    time_wait--;
  }
  if ((Health > 0)and(connected)) // as long as you have health you are part of the game, when your hp is drained, the gun doesn't read inputs anymore.
  {
    //int Reload_Button_Value = digitalRead(Reload_Button);
    //Serial.println(Reload_Button_Value);
    //Serial.println(analogRead(Reload_Button));
    if (analogRead(Reload_Button) == 1024)
    {
      bullets = bullet_type;
      Serial.print("reload gun: ");
      Serial.println(bullets);
      Serial.println(analogRead(Reload_Button));
      

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
    FastLED.show();

    if (digitalRead(button_Shoot) == HIGH) //readout of pin to detect if button is pressed, and will if so run the function "gun" (aka, it fires the "laser")
    {
      if (bullets>=1){
        prepare_shot(tempstring);
        bullets--;
        Serial.print("Remaining bullets: ");
        Serial.println(bullets);
        buzzerfun(2);
        totalbullets++;
      }
    }
    if (irrecv.decode(&results)) { //this code will try to read/decode the incomming  signals, and will also post them in the serial interface.
                                   //this will also call the function to take damage (and to recognise the other gun).
    
      Serial.println(resultToSourceCode(&results));
      Serial.println();
      uint16_t * Datass = resultToRawArray(&results);    // Blank line between entries
      decodeData(Datass);
      yield();             // Feed the WDT (again)
      //String hit = resultToTimingInfo(&results);
    }
    time_wait--;
  }
  delay(200);
  if (Health <= 0) //this part runs if the health points run out.
  {
    FastLED.clear();
    Serial.println("u dead m8");//this is the message you get when you die
    
    FastLED.clear();
    delay(50);
    leds[0] = CRGB::Red;
    FastLED.show();
    delay(50);
  }
}
