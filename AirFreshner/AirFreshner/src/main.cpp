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
#define LDR A0;

// Button bus, which is used for 3 buttons on analogPin 2
#define BUTTON_BUS A2;

// Bus for many digital signals on the bus
#define ONE_WIRE_BUS 8

// OneWire protocol to make use of the one_wire_bus
OneWire oneWire(ONE_WIRE_BUS);

// Temperature sensor from Dallas on the oneWire bus
DallasTemperature tempSensor = (&oneWire);


void updateTime(){
  myTime = millis();
}

void setup() {
  myTime = millis();
  lcdScreen.begin(2,16);
}

void loop() {
  // the running timer is constantly updated
  updateTime();

}


