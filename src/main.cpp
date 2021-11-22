#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <assert.h>
#include <FastLED.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define BIT(n,i) (n>>i&1)

#define ledPin     2 //D4 The pin that Controlls the RGB LED's

#define CHIPSET     WS2811 //type of Individualy addressable LED's we use
#define NUM_LEDS    2 // currently we use 2 RGB LED's


WiFiClient espClient; //libary to use wifi
PubSubClient client(espClient); //libary to use mqtt
boolean connected = false; // a variable t

const uint32_t kBaudRate = 115200; //bitrate 115200 is fairly standard in these type of projects.

const uint16_t kCaptureBufferSize = 1024; //standard settings of the IRremoteESP8266 libary 
const uint8_t kTimeout = 50;
const uint16_t kMinUnknownSize = 12;
const uint8_t kTolerancePercentage = kTolerance;  // kTolerance is normally 25%

const uint16_t kIrLed = 15;  // ESP8266 GPIO pin to use. Recommended: 4 (D2) // using 15 (D8)
const uint16_t kRecvPin = 14; // pin to receive ir data (D5)
String ownTeam = "FFA";  //Defines the basic configurgiration
SSD1306Wire display(0x3c, SDA, SCL); //configures the display (address and i²c) (the SDA = D2,  SCL = D1)
OLEDDisplayUi ui     ( &display );
CRGB leds[NUM_LEDS];
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;  // Somewhere to store the results

IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

// Example of data captured by IRrecvDumpV2.ino
uint16_t rawData[] = { 1646 , 422 , 393, 393, 393, 393, 393, 393, 393, 786, 393, 786, 393, 786, 393, 786, 393, 1  , 2   ,3    ,4    ,5    ,6    ,7    ,8    ,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24, 5600};
//uint16_t rawData[] =   { 1646 , 422 , 393, 393, 393, 393, 393, 393, 393, 786, 393, 786, 393, 786, 393, 786, 393, 393, 393 ,393  ,393  ,393  ,393  ,393  ,786  ,393, 393 ,393  ,393  ,393  ,393  ,393  ,786  ,393, 393 ,393  ,393  ,393  ,786 ,786  ,393  , 5600};
// binary codes of the 'legacy guns', ffa gun is not legacy
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
//codes for the teams
String Blue   =   "001";
String Red    =   "010";
String Green  =   "011";
String White  =   "100";
String FFA    =   "101";
String eigenteam = "101";

int Totalbullets = 0;
// ammo amounts gun epending on gun type
int bullet1x = 6;
int bullet2x = 3;
int bullet3x = 1;

//setu up for default team and gun
int teams = 0;
String gun = FFAGUN;
int guns = 100;
//codes for the damage
String Damagex1   = "01";
String Damagex2   = "10";
String Damagex3   = "11";
int buzzer = 0;  //  pin D3
const uint16_t ChangeTeams_Button = 12; //d6
const uint16_t button_Shoot = 13;    // pushbutton connected to digital pin D7
int val = 0;      // variable to store the read value
const uint8_t ChangeGuns_Button = 16;//D0 button to change the gun type
const uint8_t Reload_Button = A0;//reload button on A0 
int bullets = 6;//amount of bullets for default gun
int Health = 9; //you start with 9hp
int bullet_type = bullet1x;
int time_wait = 0; //cooldown for actions
int gun_delay   = 10; //default cooldown for default gun

//wifi setup
const char* ssid = "NETGEAR";
const char* password = "";
const char* mqtt_server = "192.168.1.6";

//mqtt setup (part 1)
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

int arrayPosition = 0;
char Idarr[9]="00000000";
int Id = 1;

//wifi loading funcion
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//mqtt call back function (this function should be used to pull data from the server)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

//display functions
void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 0, String(millis()));
}
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y
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


void setup() {
  pinMode(kIrLed, OUTPUT);
  digitalWrite(kIrLed,LOW);
  pinMode(buzzer, OUTPUT);  // sets the digital pin 15 as output
  pinMode(button_Shoot, INPUT);    // sets the digital pin 7 as input
  pinMode(ChangeGuns_Button,INPUT);
  pinMode(Reload_Button,INPUT);
  pinMode(ChangeTeams_Button, INPUT);
  irsend.begin();
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  assert(irutils::lowLevelSanityCheck() == 0);
  FastLED.addLeds<WS2812B, ledPin, GRB>(leds, NUM_LEDS);
  leds[1] = CRGB::Purple;
  leds[0] = CRGB::Green;
  for(int a=128; a>=1; a=a/2){      // This loop will start at 128, then 64, then 32, etc.
    if((Id-a)>=0){         // This checks if the Int is big enough for the Bit to be a '1'
      Idarr[arrayPosition]='1';  // Assigns a '1' into that Array position.
      Id-=a;}              // Subracts from the Int.
    else{
      Idarr[arrayPosition]='0';} // The Int was not big enough, therefore the Bit is a '0'
      arrayPosition++;                // Move one Character to the right in the Array.
    }

  Serial.println(Idarr);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);






  // Initialising the UI will init the display too.
  
  Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
#if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH
  irrecv.setTolerance(kTolerancePercentage);  // Override the default tolerance.
  irrecv.enableIRIn();  // Start the receiver



  ui.setTargetFPS(10);

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
String IdShoot = Idarr;

//function to keep mqtt connection alive
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-01";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//function to output sound please change this to something less annoying
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

//change teams function
String ChangeTeams(int teams){
  teams ++;
  if (teams == 5){
    teams = 0;
    gun.clear();
    eigenteam.clear();
  }
  String temp = "";
  switch (teams)
  {
    case 0:
      leds[1] = CRGB::Purple;
      
      eigenteam = FFA;
      gun = FFAGUN;
      guns = 100;
      temp = eigenteam + gun;
      ownTeam.clear();
      ownTeam.concat("FFA");
      break;
    case 1:
      leds[1] = CRGB::Blue;
      eigenteam = Blue;
      gun = BlueGunx1;
      guns = 10;
      temp = eigenteam + gun;
      ownTeam.clear();
      ownTeam.concat("Blue");
      break;
    case 2:
      leds[1] = CRGB::Red;
      eigenteam = Red;
      gun = RedGunx1;
      guns = 20;
      temp = eigenteam + gun;
      ownTeam.clear();
      ownTeam.concat("Red");
      break;
    case 3:
      leds[1] = CRGB::Green;
      eigenteam = Green;
      gun = GreenGunx1;
      guns = 30;
      temp = eigenteam + gun;
      ownTeam.clear();
      ownTeam.concat("Green");
      break;
    case 4:
      leds[1] = CRGB::White;
      eigenteam = White;
      gun = WhiteGunx1;
      guns = 40;
      temp = eigenteam + gun;
      ownTeam.clear();
      ownTeam.concat("White");
      break;
    default:
      leds[1] = CRGB::Purple;
      eigenteam = FFA;
      gun = FFAGUN;
      guns = 100;
      temp = eigenteam + gun;
      ownTeam.clear();
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
  while (i <= shot.length())
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
    Serial.print("Shoot: ");
  Serial.println(temp);
  irsend.sendRaw(rawData,42,38);
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

  snprintf (msg, MSG_BUFFER_SIZE, "%i#Health#%ld",Id, Health);
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish("Health", msg);
  snprintf (msg, MSG_BUFFER_SIZE, "%i#Bullets#%ld",Id, bullets);
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish("Bullets", msg);
  snprintf (msg, MSG_BUFFER_SIZE, "%i#HitBy#%ld",Id, Health);
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish("HitBy", msg);
   snprintf (msg, MSG_BUFFER_SIZE, "%i#TotalBullets#%ld",Id, Totalbullets);
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish("Totalbullets", msg);
  if (Health > 6)
  {
      leds[0].LimeGreen;
      FastLED.show();
  }else if ((Health>3) && (Health<=6))
  {
    leds[0].Orange;
    FastLED.show();
  }else if ((Health>0)&&(Health<=3))
  {
    leds[0].Red;
    FastLED.show();
  }
}

void decodeData(uint16_t * Datass){ //this function is to "decode" the data, since it is encoded "raw"
  String binairy_code = "";
  int i = 17;
  int iteration = 1;
  Serial.print("Eigen team: ");
  Serial.println(eigenteam);
  while (i<41) //this part decodes the received values to binary, the first values of the array is static, and already present in this code, the part between place 17 and 41 (not inclusive, also we are programmers: indexes start at 0)
  {
   String bin_old = binairy_code;
   uint16_t value = Datass[i]; 
   if (value > 600)
   {
     binairy_code = bin_old + "1";
   }
   else {
     binairy_code = bin_old + "0";
   }
   if ((iteration%8 == 0)){
     String bin_old = binairy_code;
     binairy_code = bin_old + " ";
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
  if (!client.connected()) {
    reconnect();
  }
   client.loop();

  
  if (Health > 0) // as long as you have health you are part of the game, when your hp is drained, the gun doesn't read inputs anymore.
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


    if (digitalRead(button_Shoot) == HIGH) //readout of pin to detect if button is pressed, and will if so run the function "gun" (aka, it fires the "laser")
    {
      if (bullets>=1){
        prepare_shot(gun);
        bullets--;
        Totalbullets++;
        prepare_shot(IdShoot);
        Serial.print("Remaining bullets: ");
        Serial.println(bullets);
        buzzerfun(2);
        time_wait = bullet_type*10;
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