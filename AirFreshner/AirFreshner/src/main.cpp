#include <Arduino.h>
#include <LiquidCrystal.h>
#include <NewPing.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

// timeCount variable
unsigned long myTime;

// the EEPROM adresses where the amount of sprays is stored
// Since we want to store up to 2400 and even negative sprays
// We need to split the int into 2 bytes, because EEPROM saves only bytes
#define SPRAYS_0 0
#define SPRAYS_1 1

// the amount of sprays in memory, in order to have faster reads 
// and not deplete the EEPROM module
int sprays;

// Display
LiquidCrystal lcdScreen(12, 11, 5, 4, 3, 2);

// Distance Sensor, which is sonar based
NewPing sonar(7,6,200);

// Light sensor, LDR
#define LDR A0

// magnet sensor, MAG
#define MAG A1

// Button bus, which is used for 3 buttons on analogPin 2
#define BUTTON_BUS A2
// the 3 buttons have a defined range to what should be measurable

// Bus for many digital signals on the bus
#define ONE_WIRE_BUS 8

// OneWire protocol to make use of the one_wire_bus
OneWire oneWire(ONE_WIRE_BUS);

// Temperature sensor from Dallas on the oneWire bus
DallasTemperature tempSensor = (&oneWire);

// PowerSwitch on mosfett, coindicites with the built-in led for monitoring
#define MOS 13

// Motion sensor digital output
// This sensor is "trigger once" mode
#define MOTION 10

// Representing the different states in a byte
// 0 -> not in use
// 1 -> type of use unknown
// 2 -> cleaning
// 3 -> number one
// 4 -> number two
// 5 -> triggered one
// 6 -> triggered two
// 7 -> operator menu
// 8 -> sub-menu
byte state;

// Representing different submenu's in the operator menu
// 0 -> Main i.e. default
// 1 -> unknown
byte submenu;
// represeting different selections in the current menu
// this is close to an index of an array, i.e. 0 is the first item and 4 is the 3rd item
byte menuSelection;
/////////////////////////////////////////////////////
// Sensor variables;
bool door;




// update the myTime value with the current time
void updateTime() {
  // no roll-over protection necessary since the rest is safe code
  unsigned long currentTime = millis();
}

// Write to EEPROM and also update the value in memory
void writeEEPROM_SPRAYS(int value) {
  byte num_0 = value >> 8;
  byte num_1 = value & 0xFF;

  EEPROM.write(SPRAYS_0, num_0);
  EEPROM.write(SPRAYS_1, num_1);
  sprays = value;
}

// initialze EEPROM if necessary.
// If the first byte of the int is 255
// then we asume it is never written to
// because the value should not have exceeded 2400 in the first place
void initializeEEPROM_SPRAYS(){
  byte num_0 = EEPROM.read(SPRAYS_0);
  byte num_1 = EEPROM.read(SPRAYS_1);
  int num = (num_0 << 8) + num_1;

  if (num_0 == 255)
  {
      writeEEPROM_SPRAYS(2400);
  }
  else
  {
    sprays = num;
  }
}

// Generic printer, takes name of value and the value in string 
// as well as the x,y location for the LCD
void printSensor(String name, String value, byte line_x, byte line_y){
  lcdScreen.setCursor(line_x,line_y);
  name.concat(": " + value);
  lcdScreen.print(name);
}

// debug purposes for now
void printDistance(byte x, byte y){
  String value = String(sonar.ping_cm());
  value.concat(" cm");
  printSensor("dist", value, x, y);
}

// mandatory sensor reading
void printTemperature(byte x, byte y){
  tempSensor.requestTemperatures();
  // neccesary to convert to int first, because it will be a double otherwise
  int tempValue = tempSensor.getTempCByIndex(0);
  String tempOutput = String(tempValue);
  printSensor("temp", tempOutput, x, y);
}

void printLDR(byte x, byte y){
  int LDR_value = analogRead(LDR);
  String output = String(LDR_value);
  printSensor("LDR", output, x , y);
}

void printMotion(byte x, byte y){
  int motionValue = digitalRead(MOTION);
  String output = String(motionValue);
  printSensor("Motion", output, x, y);
}

void setup() {
  myTime = millis();
  lcdScreen.begin(2,16);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MOTION, INPUT);
  digitalWrite(LED_BUILTIN,HIGH);


  // delay to stabilize motion sensor
  // delay(60000);

  // set state default
  state = 0;
}

void loop() {
  // the running timer is constantly updated
  updateTime();
  
  // print temperate every 2 seconds on line 1
  if(myTime%2000 == 0){
    printTemperature(0,0);
  }

  // print light-readings every half second on line 2
  // if(myTime%500 == 0){
  //   printLDR(0,1);
  // }

  // print motion-readings every half second on line 2
  if(myTime%500 == 0){
    printMotion(0,1);
  }

  // Blink on pin13 and builtin led every half second
  if(myTime%1000 == 0){
    digitalWrite(LED_BUILTIN, HIGH);
  } else if(myTime%500 == 0){
    digitalWrite(LED_BUILTIN, LOW);
  }
}


