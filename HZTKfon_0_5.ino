/* ============================================
HZTKfon version 0.5

The MIT License

Copyright (c) 2017 Albert Gajšak and Robert Sedak

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */


#include <Keypad.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte colPins[ROWS] = {10, 9, 8, 7}; //connect to the row pinouts of the keypad
byte rowPins[COLS] = {6, 5, 4, 3}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


Adafruit_SH1106 display(13);

//SIM800 TX is connected to Arduino D11
#define SIM800_TX_PIN 11
//SIM800 RX is connected to Arduino D12
#define SIM800_RX_PIN 12

#define buzzerPin 13

//Create software serial object to communicate with SIM800
SoftwareSerial sim800(SIM800_TX_PIN, SIM800_RX_PIN);

void setup() {
  //Begin serial comunication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C, 1);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("HZTKfon v0.05"));
  display.display();
  display.setTextSize(1);
  display.println(F("created by:\nAlbert Gajsak\nRobert Sedak\ndesign:\nZvonimir. L. Padovan\n"));
  display.println(F("Starting up..."));
  display.display();
  delay(3000);
  //Begin serial communication with Arduino and SIM800
  sim800.begin(9600);
  delay(1000);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("start serial communication with SIM module"));
  display.display();
  Serial.println(F("start serial communication with SIM module"));
  delay(200);

  enterPIN();

  display.clearDisplay();
  display.setCursor(0, 0);
  simDetails();

  sim800.println(F("AT+CFUN=1")); //enable full functionality
  delay(1000);
  sim800.println(F("AT+CLVL=100"));
  sim800.println(F("AT+CRSL=100"));
  sim800.println(F("AT+CMIC=0,6"));

  tone(buzzerPin, 523, 125);
  delay(500);
  tone(buzzerPin, 587, 65);
  delay(125);
  tone(buzzerPin, 659, 65);
  delay(250);
  tone(buzzerPin, 784, 80);
  delay(500);

}

String phone_number;

void loop() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("INSTRUCTIONS:\nA: answer call\nC: initiate call\nD: reject call\n"));
  display.display();
  char key = pressedKey();
  switch (key) {
    case 'A':
      answerCall();
      break;
    case 'C':
      initiateCall();
      break;
    case 'D':
      refuseCall();
      break;
  }
} // end of loop()



void simDetails() {
  char findstring[] = ":";
  Serial.print(F("operator:"));
  sim800.println(F("AT+COPS?")); // OPERATOR
  delay(500);
  if (sim800.find(findstring)) { // decode reply
    while (sim800.available()) {
      char c = sim800.read();
      if (c != '\n') {
        Serial.write(c); // replace new line with space
        display.print(c);
      }
      else {
        Serial.print(F(" "));
        display.print(F(" "));
      }
      delay(5);
    }
  }
  else {
    Serial.print(F("n/a"));
    display.print(F("n/a"));
  }
  Serial.println();
  display.display();

  Serial.print(F("signal strength:"));
  display.print(F("signal strength:"));
  sim800.println(F("AT+CSQ")); // SIGNAL STRENGTH
  delay(500);
  if (sim800.find(findstring)) { // decode reply
    while (sim800.available()) {
      char c = sim800.read();
      if (c != '\n') {
        Serial.write(c); // replace new line with space
        display.print(c);
      }
      else {
        Serial.print(F(" "));
        display.print(F(" "));
      }
    }
  }
  else {
    Serial.print(F("n/a"));
    display.print(F("n/a"));
  }
  Serial.println();
  display.display();

  Serial.print(F("battery level:"));
  display.println(F("battery level:"));
  sim800.println(F("AT+CBC")); // BATTERY LEVEL
  delay(500);
  if (sim800.find(findstring)) { // decode reply
    while (sim800.available()) {
      char c = sim800.read();
      if (c != '\n') {
        Serial.write(c); // replace new line with space
        display.print(c);
      }
      else {
        Serial.print(F(" "));
        display.print(F(" "));
      }
    }
  }
  else {
    Serial.print(F("n/a"));
    display.print(F("n/a"));
  }
  
  Serial.println();
  display.display();

  sim800.println(F("AT+CLVL=100")); // set speaker volume 0-100
  simReply();
  sim800.println(F("AT+CRSL=100")); // set ringer volume 0-100
  simReply();
  sim800.println(F("AT+CMIC=0,15")); // set mic to gain level 0-15
  simReply();
  sim800.println(F("  ")); // set alert/ring tone
  simReply();

  Serial.println(F("Ready"));
  display.print(F("Ready"));
  display.display();
  
} // end of simDetails()



void simReply() { // SIM REPLY
  delay(500);
  while (sim800.available()) {
    char c = sim800.read();
    Serial.print(c);
    delay(5);
  }
} // end of simReply()


void simCall() {
  char findstring[] = "0,1";
  Serial.print(F("Start call.. "));
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("PHONE CALL\n INITIATED"));
  display.display();

  sim800.println(F("AT+CREG?")); // possible replies: 0,0=not registered 0,1=registered on home network 0,2=not registered but searching 0,3=denied 0,4=unknown 0,5=roaming
  delay(1000);
  if (sim800.find(findstring)) {
    Serial.println(F("network connection still OK"));
    display.println(F("CONNECTION TEST OK"));
    display.display();
    simReply();
  }

  Serial.println(F("Calling number "));
  display.println(F("Calling number "));
  Serial.println(phone_number);
  display.println(phone_number);
  display.display();

  String cmd = "ATD";
  cmd += phone_number;
  cmd += ";";
  Serial.println(cmd);
  sim800.println(cmd);
  delay(1000);
  simReply();
  
} // end of simCall()

boolean simReady() { // SIM READY
  char findstring[] = "OK";
  sim800.println(F("AT"));
  delay(100);
  if (sim800.find(findstring)) {
    Serial.println(F("OK"));
    Serial.print(F("Registering on the network.."));
    delay(2500); //2.5s to register to the network
  }
  Serial.print(F("."));
  sim800.println(F("AT+CREG?")); // possible replies: 0,0=not registered 0,1=registered on home network 0,2=not registered but searching 0,3=denied 0,4=unknown 0,5=roaming
  delay(1000);
  char findstring2[] = "0,1";
  if (sim800.find(findstring2)) {
    Serial.println(F(" OK"));
    return true;
  }
  else {
    Serial.println(F("sim800 module not OK, please check"));
    return false;
  }
} // end of simReady()

void answerCall() {
  sim800.println(F("ATA"));
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  Serial.println(F("CALL\nANSWER"));
  display.println(F("CALL\nANSWER"));
  display.display();
} //end of answerCall()

void refuseCall() {
  sim800.println("ATH");
  delay(100);
} //end of refuseCall()



void initiateCall() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("INSTRUCTIONS:\nA: erase\nC: call\nD: disconnect"));
  display.display();
  char key = 0;
  while (key != 'D') {
    key = pressedKey();
    switch (key) {
      case 'A':
        phone_number.remove(phone_number.length() - 1);
        Serial.println(phone_number);
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println(F("INSTRUCTIONS:\nA: erase\nC: call\nD: disconnect"));
        display.setTextSize(2);
        display.print(phone_number);
        display.display();
        break;
      case 'C':
        simCall();
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println(F("CONNECTED:\nD: disconnect\n"));
        display.setTextSize(2);
        display.print(phone_number);
        display.display();
        phone_number = "";
        break;
      case 'D':
        sim800.println("ATH");
        delay(100);
        break;
      default:
        phone_number += key;
        Serial.println(phone_number);
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println(F("INSTRUCTIONS:\nA: erase\nC: call\nD: disconnect"));
        display.setTextSize(2);
        display.print(phone_number);
        display.display();
        break;
    }
  }
} //end of initiateCall()


char pressedKey() {
  char key = NO_KEY;
  while (key == NO_KEY) {
    key = keypad.getKey();
    switch (key) {
      case '1':
        tone(buzzerPin, 200, 100);
        break;
      case '2':
        tone(buzzerPin, 300, 100);
        break;
      case '3':
        tone(buzzerPin, 400, 100);
        break;
      case '4':
        tone(buzzerPin, 500, 100);
        break;
      case '5':
        tone(buzzerPin, 600, 100);
        break;
      case '6':
        tone(buzzerPin, 700, 100);
        break;
      case '7':
        tone(buzzerPin, 800, 100);
        break;
      case '8':
        tone(buzzerPin, 900, 100);
        break;
      case '9':
        tone(buzzerPin, 1000, 100);
        break;
      case '0':
        tone(buzzerPin, 1100, 100);
        break;
      case '*':
        tone(buzzerPin, 1200, 100);
        break;
      case '#':
        tone(buzzerPin, 1300, 100);
        break;
    }
  }
  return key;
} // end of pressedKey()


void enterPIN() {
  String pin;
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("ENTER PIN:\nA: erase\nC: confirm\n"));
  display.display();
  char key = 0;
  while (key != 'C') {
    key = pressedKey();
    switch (key) {
      case 'A':
        pin.remove(pin.length() - 1);
        Serial.println(pin);
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println(F("ENTER PIN:\nA: erase\nC: confirm\n"));
        display.setTextSize(2);
        display.print(pin);
        display.display();
        break;
      case 'C':
        break;
      default:
        pin += key;
        Serial.println(pin);
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println(F("ENTER PIN:\nA: erase\nC: confirm\n"));
        display.setTextSize(2);
        display.print(pin);
        display.display();
        break;
    }
  }

  display.setTextSize(1);
  String cmd = "AT+CPIN=\"";
  cmd += pin;
  cmd += "\"";
  Serial.println(cmd);
  sim800.println(cmd);
  simReply();

} // end of enterPIN()

