#include <Arduino.h>
#include <LiquidCrystal.h>
#include <NewPing.h>
#include <DallasTemperature.h>

// timeCount variable
unsigned long myTime;

// Display
LiquidCrystal lcdScreen(12, 11, 5, 4, 3, 2);




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


