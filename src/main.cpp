#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <assert.h>
#include <FastLED.h>

#define ledPin     2 //D4
#define COLOR_ORDER RGB
#define CHIPSET     WS2811
#define NUM_LEDS    1

#define KORT 393
#define LANG 786 // gemiddelde van veel dingen
const uint32_t kBaudRate = 115200; //bitrate
const uint16_t kCaptureBufferSize = 1024; 

const uint8_t kTimeout = 50;
const uint16_t kMinUnknownSize = 12;

const uint8_t kTolerancePercentage = kTolerance;  // kTolerance is normally 25%

const uint16_t kIrLed = 15;  // ESP8266 GPIO pin to use. Recommended: 4 (D2) // using 15 (D8)
const uint16_t kRecvPin = 14; // pin to receive ir data

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
int bullet1x = 6;
int bullet2x = 3;
int bullet3x = 1;


int teams = 0;
String gun = FFAGUN;
int guns = 100;
//codes for the damage
String Damagex1   = "01";
String Damagex2   = "10";
String Damagex3   = "11";
int buzzer = 0;  //  pin D3
const uint16_t ChangeTeams_Button = 12; //d6
int button_Shoot = 13;    // pushbutton connected to digital pin D4
int val = 0;      // variable to store the read value
const uint8_t ChangeGuns_Button = 16;//D3 button to change the gun type
const uint8_t Reload_Button = A0;//reload button on A0 
int bullets = 6;//amount of bullets for default gun
int Health = 9; //you start with 9hp
int bullet_type = bullet1x;

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
  FastLED.addLeds<WS2812B, ledPin, RGB>(leds, NUM_LEDS);

  Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
#if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH
  irrecv.setTolerance(kTolerancePercentage);  // Override the default tolerance.
  irrecv.enableIRIn();  // Start the receiver
}
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
      eigenteam = FFA;
      gun = FFAGUN;
      guns = 100;
      temp = eigenteam + gun;
      break;
    case 1:
      eigenteam = Blue;
      gun = BlueGunx1;
      guns = 10;
      temp = eigenteam + gun;
      break;
    case 2:
      eigenteam = Red;
      gun = RedGunx1;
      guns = 20;
      temp = eigenteam + gun;
      break;
    case 3:
      eigenteam = Green;
      gun = GreenGunx1;
      guns = 30;
      temp = eigenteam + gun;
      break;
    case 4:
      eigenteam = White;
      gun = WhiteGunx1;
      guns = 40;
      temp = eigenteam + gun;
      break;
    default:
      eigenteam = FFA;
      gun = FFAGUN;
      guns = 100;
      temp = eigenteam + gun;
      break;
    }
    return temp;
      
  //Team = UsableTeams[TeamIndex];
  

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
}

void decodeData(uint16_t * Datass){ //this function is to "decode" the data, since it is encoded "raw"
  String binairy_code = "";
  int i = 17;
  int iteration = 1;
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
    if (((temp == Blue) or (temp == Red) or (temp == Green) or (temp == White)) and ((temp != eigenteam) and (temp != FFA))) 
    // if (((temp == Blue) or (temp == Red) or (temp == Green) or (temp == White)) and (temp != eigenteam) and (eigenteam != FFA)) 
    {
      get_damage(binairy_code.substring(15,17));
      b = false;
    }
    else if ((temp == FFA) and (eigenteam == FFA))
    {
      get_damage(binairy_code.substring(5,17));
      b = false;
    }
    else {
      b = false;
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

    }
    if (Health == 9)
    {
      leds[0] = CRGB (0,255,0);
    }
    if (Health == 6)
    {
      leds[0] = CRGB (255,255,0);
    }
    if (Health == 3)
    {
      leds[0] = CRGB (255,0,0);
    }
    if (digitalRead(button_Shoot) == HIGH) //readout of pin to detect if button is pressed, and will if so run the function "gun" (aka, it fires the "laser")
    {
      prepare_shot(gun);
      bullets--;
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
      
      
    }
    if ((digitalRead(ChangeGuns_Button) == HIGH)and (eigenteam = FFA)) //reads the change guns pin, and will attempt to change 'gun type' if it is possible in the given team
    {
      changeGuns();
           
      Serial.print("Gun: ");
      Serial.println(gun);
      Serial.print("Gun ID: ");
      Serial.println(guns);
      Serial.print("Team: ");
      Serial.println(eigenteam); 
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
    delay(200);
  }
  delay(200);
  if (Health <= 0)
  {
    Serial.println("u dead m8");//this is the message you get when you die
    leds[0] = CRGB (255,0,0);
    delay(100  );
    leds[0] = CRGB (0, 0, 0);
  }

}