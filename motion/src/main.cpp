#include <Arduino.h>
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"

MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;
float yaw, pitch, roll;

//handeling state for the gyroscope
#define STATE_BASE_STATE 0
#define STATE_WATER_MOTION 1
#define STATE_SHOW_SENSORS_MOTION 2
#define STATE_CHAOTIC 3 //for when it's not sure what movement to do
#define STATE_UNKNOWN 4

byte currentState = 0;
byte stateToActUpon = 0;

bool isThresholdReached = false;
bool isBackFromThreshold = false;

//to be serially written 
#define SENSOR_GESTURE_SENT 0
#define WATER_GESTURE_SENT 1

//thanks to Eva Imbens for informing me about the use of a struct here. (tuple does not go brr)
struct MyData {
  int X; //standard is ~134 deg
  int Y; //standard is ~129 deg
};

MyData data;

void serialShowSensor(){
  Serial.println(0);
  //Serial.println("sensor gesture completed"); 
}

void serialWaterGesture(){
  Serial.println(1);
  //Serial.println("water gesture completed");
}

byte getState(int xAngle, int yAngle){
  if(yAngle > 160 || yAngle < 20){
    return STATE_WATER_MOTION;
  } 
  if(xAngle < 20 || xAngle > 160){
    return STATE_SHOW_SENSORS_MOTION;
  }
  if((yAngle > 160 || yAngle < 20) && (xAngle < 20 || xAngle > 160)){
    return STATE_CHAOTIC;
  }
  if((yAngle > 70 || yAngle < 110) && (xAngle > 70 || xAngle < 110)){
    return STATE_BASE_STATE;
  }
  else{
    return STATE_UNKNOWN;
  }
}

void checkMotionThreshold(byte state){
  //if the gesture is not precise enough we don't want to check motion threshold
  if(state == STATE_CHAOTIC){
    return;
  }
  if (state == STATE_WATER_MOTION || state == STATE_SHOW_SENSORS_MOTION){
    isThresholdReached = true;
    stateToActUpon = state;
  } 
}

void checkStasisAfterMotion(byte state){
  if(state == STATE_BASE_STATE && isThresholdReached){
    isBackFromThreshold = true;
  }
}

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();
  pinMode(LED_BUILTIN, OUTPUT);
}

unsigned long previousMillis = 0;
const long interval = 500;  // interval time in milliseconds

void loop()
{
  unsigned long currentMillis = millis();

  //for some reason we need this timer, otherwise buggy wuggy serial monitor and we don;t like that
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    data.X = map(ax, -17000, 17000, 0, 180 ); // X axis data
    data.Y = map(ay, -17000, 17000, 0, 180);  // Y axis data
    
    //get new/current state
    byte state = getState(data.X, data.Y);
    //Serial.print("current state = ");
    //Serial.println(state);

    //adapt the two booleans based on current state
    checkMotionThreshold(state);
    checkStasisAfterMotion(state);

    //serially write if the gesture is made
    if(stateToActUpon == STATE_WATER_MOTION && isBackFromThreshold){
      isThresholdReached = false;
      isBackFromThreshold = false;
      stateToActUpon = STATE_BASE_STATE;
      serialWaterGesture();
    } 
    if(stateToActUpon == STATE_SHOW_SENSORS_MOTION && isBackFromThreshold){
      isThresholdReached = false;
      isBackFromThreshold = false;
      stateToActUpon = STATE_BASE_STATE;
      serialShowSensor();
    } 
  }
}
