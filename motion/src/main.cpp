#include <Arduino.h>
#include <I2Cdev.h>
#include <Wire.h>
void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);

  Wire.begin();
  Serial.begin(38400);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN,LOW);
  delay(1000);
}