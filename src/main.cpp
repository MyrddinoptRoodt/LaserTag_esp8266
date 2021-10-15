#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <assert.h>
#include <string>
#include <iostream>
#define KORT 393
#define LANG 786 // gemiddelde van veel dingen
const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 1024;

const uint8_t kTimeout = 50;
const uint16_t kMinUnknownSize = 12;

const uint8_t kTolerancePercentage = kTolerance;  // kTolerance is normally 25%

const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
const uint16_t kRecvPin = 14;


IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;  // Somewhere to store the results

IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

// Example of data captured by IRrecvDumpV2.ino
uint16_t rawData[] = { 1646 , 422 , 393, 393, 393, 393, 393, 393, 393, 786, 393, 786, 393, 786, 393, 786, 393, 1  , 2   ,3    ,4    ,5    ,6    ,7    ,8    ,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24, 5600};
//uint16_t rawData[] =   { 1646 , 422 , 393, 393, 393, 393, 393, 393, 393, 786, 393, 786, 393, 786, 393, 786, 393, 393, 393 ,393  ,393  ,393  ,393  ,393  ,786  ,393, 393 ,393  ,393  ,393  ,393  ,393  ,786  ,393, 393 ,393  ,393  ,393  ,786 ,786  ,393  , 5600};
// Example Samsung A/C state captured from IRrecvDumpV2.ino
// Example Samsung A/C state captured from IRrecvDumpV2.ino
std::string BlueGunx1    = "00000001 00000001 00000110";
std::string BlueGunx2    = "00000001 00000010 00000111";
std::string BlueGunx3    = "00000001 00000011 00001000";
std::string RedGunx1     = "00000010 00000001 00000111";
std::string RedGunx2     = "00000010 00000010 00001000";
std::string RedGunx3     = "00000010 00000011 00001001";
std::string GreenGunx1   = "00000011 00000001 00001000";
std::string GreenGunx2   = "00000011 00000010 00001001";
std::string GreenGunx3   = "00000011 00000011 00001011";
std::string WhiteGunx1   = "00000100 00000001 00001001";
std::string WhiteGunx2   = "00000100 00000010 00001011";
std::string WhiteGunx3   = "00000100 00000011 00001100";
std::string FFAGUN       = "00000101 00000001 00001100";

std::string Blue   =   "001";
std::string Red    =   "010";
std::string Green  =   "011";
std::string White  =   "100";
std::string FFA    =   "101";
std::string eigenteam = "101";
int teams = 0;
std::string gun = FFAGUN;
int guns = 100;

std::string Damagex1   = "01";
std::string Damagex2   = "10";
std::string Damagex3   = "11";
int buzzer = 15;  //  pin D8
int ChangeTeams_Button = 12;
int button_Shoot = 5;    // pushbutton connected to digital pin D0
int val = 0;      // variable to store the read value
int ChangeGuns_Button = 16;

int Health = 9;


void setup() {
  pinMode(buzzer, OUTPUT);  // sets the digital pin 13 as output
  pinMode(button_Shoot, INPUT);    // sets the digital pin 7 as input
  pinMode(ChangeGuns_Button,INPUT);
  irsend.begin();
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  assert(irutils::lowLevelSanityCheck() == 0);

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
  while (i < repeat)
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
  
}
void ChangeTeams(){
  teams ++;
  if (teams == 5){
    teams = 0;
  }
  switch (teams)
  {
  case 0:
    eigenteam = FFA;
    gun = FFAGUN;
    guns = 100; 
    break;
  case 1:
    eigenteam = Blue;
    gun = BlueGunx1;
    guns = 10;
    break;
  case 2:
    eigenteam = Red;
    gun = RedGunx1;
    guns = 20;
    break;
  case 3:
    eigenteam = Green;
    gun = GreenGunx1;
    guns = 30;
    break;
  case 4:
    eigenteam = White;
    gun = WhiteGunx1;
    guns = 40;
    break;
  default:
    eigenteam = FFA;
    gun = FFAGUN;
    guns = 100;
    break;
  }
  //Team = UsableTeams[TeamIndex];
  

}

void prepare_shot(std::string shot){
  std::string list = "01";
  int(i) = 0;
  int(array_place) = 17;
  while (i < shot.length())
  {
    if (shot[i] == list[1]){
      rawData[array_place] = 786;
      array_place++;
    }
    if (shot[i] == list[0])
    {
      rawData[array_place] = 393;
      array_place++;
      
    }
    i++;
  }
  irsend.sendRaw(rawData,42,38);

  

}




void get_damage(std::string temp){
  std::string temp2 = "";
  temp2.append(temp.substr(15,2));
  if (temp2 == Damagex1)
  {
    Health --;
    buzzerfun(1);
  }
  if (temp2 == Damagex2)
  {
    Health -= 2;
    buzzerfun(2);
  }
  if (temp2 == Damagex3)
  {
    Health -= 3;
    buzzerfun(3);
  }
  if (Health < 1)
  {
    Serial.println("Dead");
    buzzerfun(5);
  } 
}

void decodeData(uint16_t * Datass){
  std::string binairy_code = "";
  int(i) = 17;
  int(iteration) = 1;
  while (i<41)
  {
   uint16_t value = Datass[i];
   if (value > 600)
   {
     binairy_code.append("1");
   }
   else {
     binairy_code.append("0");
   }
   if ((iteration%8 == 0)){
     binairy_code.append(" ");
   }
   iteration++;
   i++;
  }
  Serial.println("code: ");
  bool b = true; 
  while (b)
  {
    std::string temp = "";
    temp.append(binairy_code.substr(5,3));
    if ((temp != eigenteam) and (temp != FFA))
    {
      get_damage(binairy_code);
      b = false;
    }
    else if ((temp == FFA) and (eigenteam == FFA))
    {
      get_damage(binairy_code);
      b = false;
    }
    else {
      b = false;
    }
  }
  
}
void changeGuns (){
  guns ++;
  switch (teams)
  {
  case 0:
    gun = FFAGUN;
    break;
  case 1:
    switch (guns)
    {
    case 11:
      gun = BlueGunx2;
      break;
    case 12:
      gun = BlueGunx3;
      break;
    case 13:
      gun = BlueGunx1;
      guns = 10;
      break;

    default:
      guns = 10;
      gun = BlueGunx1;
      teams = 1;
      eigenteam = Blue;
      break;
    }
    break;
  case 2:
    switch (guns)
    {
    case 21:
      gun = BlueGunx2;
      break;
    case 22:
      gun = BlueGunx3;
      break;
    case 23:
      gun = BlueGunx1;
      guns = 10;
      break;

    default:
      guns = 20;
      gun = RedGunx1;
      teams = 2;
      eigenteam = Red;
      break;
    }
    break;
  case 3:
    switch (guns)
    {
    case 31:
      gun = GreenGunx2;
      break;
    case 32:
      gun = GreenGunx3;
      break;
    case 33:
      gun = GreenGunx1;
      guns = 30;
      break;

    default:
      guns = 30;
      gun = GreenGunx1;
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
      break;
    case 42:
      gun = WhiteGunx3;
      break;
    case 43:
      gun = WhiteGunx1;
      guns = 40;
      break;

    default:
      guns = 40;
      gun = WhiteGunx1;
      teams = 4;
      eigenteam = White;
      break;
    }
    break;
          
  default:
    break;
  }
  
}


void loop() {
  while (Health > 1)
  {
    if (digitalRead(button_Shoot) == HIGH)
    {
      //prepare_shot(gun);
    }
    if (digitalRead(ChangeTeams_Button) == HIGH)
    {
      //ChangeTeams();
    }
    /*if ((digitalRead(ChangeTeams_Button) == HIGH) and (eigenteam == FFA))
    {
      //changeGuns();
    }    
    */
    
    if (irrecv.decode(&results)) {
   
      uint32_t now = millis();
   
      Serial.println(resultToSourceCode(&results));
      Serial.println();
      uint16_t * Datass = resultToRawArray(&results);    // Blank line between entries
      decodeData(Datass);
      yield();             // Feed the WDT (again)
      //String hit = resultToTimingInfo(&results);
    delay(200);
    }
    
  }
  delay(2000);
}