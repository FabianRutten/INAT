#include <MPU6050.h>
#include "Wire.h"
#include <MPU6050_6Axis_MotionApps20.h>
#include <MPU6050_6Axis_MotionApps612.h>
#include <MPU6050_9Axis_MotionApps41.h>
#include <helper_3dmath.h>
#include <Arduino.h>
#include <I2Cdev.h>

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