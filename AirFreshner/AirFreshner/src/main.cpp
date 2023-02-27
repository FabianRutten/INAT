#include <Arduino.h>
#include <LiquidCrystal.h>
#include <NewPing.h>
#include <DallasTemperature.h>

// timeCount variable
unsigned long myTime;

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

// Bus for many digital signals on the bus
#define ONE_WIRE_BUS 8

// OneWire protocol to make use of the one_wire_bus
OneWire oneWire(ONE_WIRE_BUS);

// Temperature sensor from Dallas on the oneWire bus
DallasTemperature tempSensor = (&oneWire);


void updateTime(){
  myTime = millis();
}

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

// setup and loop on the bottem, do not place voids below this comment pls thank you

void setup() {
  myTime = millis();
  lcdScreen.begin(2,16);
}

void loop() {
  // the running timer is constantly updated
  updateTime();
  
  // print temperate every 2 seconds on line 1
  if(myTime%2000){
    printTemperature(0,0);
  }

  // print light-readings every half second on line 2
  if(myTime%500){
    printLDR(0,1);
  }
}


