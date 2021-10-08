#include <Arduino.h>
#include <IRremote.h>
#define DECODE_NEC          // Includes Apple and Onkyo
#define DECODE_DISTANCE

//defining gun types
uint32_t Gun1Damage =   0x10010001;
uint32_t Gun2Damage =   0x10010011;
uint32_t Gun3Damage =   0x10010111;
uint32_t Gun4Damage =   0x10011111;
//these guns require extra's
uint32_t TurretDamage = 0x11010001;
uint32_t Airstrike =    0x10011001;
uint32_t Grenade =      0x10011101;

uint32_t GunType = Gun1Damage; //defauld Gun

//defining teams
uint16_t TeamBlauw =    0x0101;
uint16_t TeamGroen =    0x0102;
uint16_t TeamRood =     0x0103;
uint16_t TeamGeel =     0x0104;
uint16_t TeamFFA =      0x0105;
uint16_t TeamNPC =      0x0106;

uint16_t Team = TeamFFA; //defauld team


int TeamIndex = 0;
int GunIndex = 0;

uint16_t Teams        [6] = {TeamFFA,        TeamRood,       TeamGeel,       TeamGroen,      TeamBlauw,      TeamNPC}; //array for ALL teams 
uint16_t UsableTeams  [5] = {TeamFFA,        TeamRood,       TeamGeel,       TeamGroen,      TeamBlauw}; //array for usable teams
uint32_t Guns         [7] = {Gun1Damage,     Gun2Damage,     Gun3Damage,     Gun4Damage,     TurretDamage,   Airstrike,      Grenade}; //array for ALL guns
uint32_t UsableGuns   [4] = {Gun1Damage,     Gun2Damage,     Gun3Damage,     Gun4Damage}; //array for usable guns

#define DELAY_AFTER_SEND 2000
#define DELAY_AFTER_LOOP 5000
int buzzer = 15;  //  pin D8
int ChangeTeams_Button = 12;
int button_Shoot = 5;    // pushbutton connected to digital pin D0
int val = 0;      // variable to store the read value
int ChangeGuns_Button = 16;
void setup() {
  // put your setup code here, to run once:
  pinMode(buzzer, OUTPUT);  // sets the digital pin 13 as output
  pinMode(button_Shoot, INPUT);    // sets the digital pin 7 as input
  pinMode(ChangeGuns_Button,INPUT);
  Serial.begin(9600);
  IrReceiver.begin(14);
  IrSender.begin(4, ENABLE_LED_FEEDBACK); // Specify send pin and enable feedback LED at default feedback LED pin
  

}
void ChangeGuns(){
  GunIndex ++;
  if (GunIndex == 4){
    GunIndex = 0;
  }
  GunType = UsableGuns[GunIndex];
  

}
void ChangeTeams(){
  TeamIndex ++;
  if (TeamIndex == 5){
    TeamIndex = 0;
  }
  Team = UsableTeams[GunIndex];
  

}

uint16_t sAddress = 0x0102;
uint8_t sCommand = 0x34;
uint8_t sRepeats = 1;
uint32_t testdata = 0x11010001;
void send_ir_data() {
    Serial.print(F("Sending: 0x"));
    Serial.print(sAddress, HEX);
    Serial.print(sCommand, HEX);
    Serial.println(sRepeats, HEX);
    //IrSender.sendNECRaw(data,1);
    Serial.print(testdata);

    // clip repeats at 4
    if (sRepeats > 4) {
        sRepeats = 4;
    }
    // Results for the first loop to: Protocol=NEC Address=0x102 Command=0x34 Raw-Data=0xCB340102 (32 bits)
    IrSender.sendNECRaw(testdata,1);//sAddress, sCommand, sRepeats);
    IrSender.sendNEC(GunType,testdata,1);
}

void receive_ir_data() {
    if (IrReceiver.decode()) {
        Serial.print(F("Decoded protocol: "));
        Serial.print(getProtocolString(IrReceiver.decodedIRData.protocol));
        Serial.print(F("Decoded raw data: "));
        Serial.print(IrReceiver.decodedIRData.decodedRawData, HEX);
        uint32_t Tempdata = IrReceiver.decodedIRData.decodedRawData;
        Serial.print(F(", decoded address: "));
        Serial.print(IrReceiver.decodedIRData.address, HEX);
        Serial.print(F(", decoded command: "));
        uint16_t TempAddress =IrReceiver.decodedIRData.address;
        Serial.println(IrReceiver.decodedIRData.command, HEX);
        IrReceiver.resume();

    }
    if (IrReceiver.decodeNEC()) {
        Serial.print(F("Decoded protocol: "));
        Serial.print(getProtocolString(IrReceiver.decodedIRData.protocol));
        Serial.print(F("Decoded raw data: "));
        Serial.print(IrReceiver.decodedIRData.decodedRawData, HEX);
        //uint32_t data = IrReceiver.decodedIRData.decodedRawData;
        Serial.print(F(", decoded address: "));
        Serial.print(IrReceiver.decodedIRData.address, HEX);
        Serial.print(F(", decoded command: "));
        Serial.println(IrReceiver.decodedIRData.command, HEX);
        IrReceiver.resume();
      
    }   
    
}
void buzzerfun(){
   digitalWrite(buzzer, LOW);
   delay(5);
   digitalWrite(buzzer, HIGH);
   delay(5);
   digitalWrite(buzzer, LOW);
   delay(5);
   digitalWrite(buzzer, HIGH);
}

void loop() {
  while (true)
  {
    
    val = digitalRead(button_Shoot);   // read the input pin
    //Serial.println(analogRead(analogInPin));
    while( digitalRead(button_Shoot) == HIGH)
    {
      buzzerfun();
      val = digitalRead(button_Shoot);


      Serial.println();
      Serial.print(F("address=0x"));
      Serial.print(sAddress, HEX);
      Serial.print(F(" command=0x"));
      Serial.print(sCommand, HEX);
      Serial.print(F(" repeats="));
      Serial.println(sRepeats);
      Serial.flush();

      send_ir_data();
      // wait for the receiver state machine to detect the end of a protocol
      delay((RECORD_GAP_MICROS / 1000) + 5);
      sAddress += 0x0101;
      sCommand += 0x11;
      sRepeats++;
      delay(200);

    }/* code */
    receive_ir_data();
    if (digitalRead(ChangeGuns_Button) == HIGH){
      ChangeGuns();
    }
    if (digitalRead(ChangeTeams_Button) == HIGH){
      ChangeTeams();
    }
    delay(200);
  }
    // sets the LED to the button's value
  // put your main code here, to run repeatedly:
}
