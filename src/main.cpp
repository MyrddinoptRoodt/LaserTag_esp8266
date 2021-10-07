#include <Arduino.h>
#include <IRremote.h>
#define DECODE_NEC          // Includes Apple and Onkyo
#define DECODE_DISTANCE
const int analogInPin = A0;  // ESP8266 Analog Pin ADC0 = A0
int sensorValue = 0;  // value read from the pot
uint32_t TeamRood = 0x10010001;
uint32_t TeamBlauw = 0x10010011;
uint32_t TeamGeel = 0x10010111;
uint32_t TeamGroen = 0x10011111;
uint32_t TeamFFA = 0x11010001;


/*
 * Define macros for input and output pin etc.
 */
#define DELAY_AFTER_SEND 2000
#define DELAY_AFTER_LOOP 5000
int buzzer = 15;  //  pin D8
int button = 5;    // pushbutton connected to digital pin D0
int val = 0;      // variable to store the read value
void setup() {
  // put your setup code here, to run once:
  pinMode(buzzer, OUTPUT);  // sets the digital pin 13 as output
  pinMode(button, INPUT);    // sets the digital pin 7 as input
  Serial.begin(9600);
  IrReceiver.begin(14);
  IrSender.begin(4, ENABLE_LED_FEEDBACK); // Specify send pin and enable feedback LED at default feedback LED pin
  

}
void change_team(){
  sensorValue = analogRead(analogInPin);
}
uint32_t eigen_team = 0x11010001;
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
}

void receive_ir_data() {
    if (IrReceiver.decode()) {
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
        if (IrReceiver.decodedIRData.address != eigen_team){

        }
        
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
    
    val = digitalRead(button);   // read the input pin
    Serial.println(analogRead(analogInPin));
    while( digitalRead(button) == HIGH)
    {
      buzzerfun();
      val = digitalRead(button);


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
    delay(200);
  }
    // sets the LED to the button's value
  // put your main code here, to run repeatedly:
}
