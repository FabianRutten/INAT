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

// the EEPROM adress for the configurable delay
#define SPRAY_DELAY 2
// the configurable delay in memory, in order to have faster reads 
// and not deplete the EEPROM module
byte sprayDelay;

// Display
LiquidCrystal lcdScreen(7,6,5,4,3,2);

// Distance Sensor, which is sonar based
NewPing sonar(10,9,200);

// Light sensor, LDR
#define LDR A0

// magnet sensor, MAG
#define MAG A1

// Button bus, which is used for 3 buttons on analogPin 2
#define BUTTON_BUS A2
// 3 buttons with each 3 threshholds
// each is lower than value
#define BUTTON_ONE 400             // this button is the override!!!
#define BUTTON_TWO 600
#define BUTTON_THREE 1024

// the buttons and other sensors might need debouncing, 
// so we define a standard debouncing time in milliseconds
// also a button current state
// and a time variable
#define DEBOUNCE_DURATION 10
byte buttonState;
byte lastButtonState;
// consume this boolean if you used the button
bool isPressed;
unsigned long buttonDebounceTimer;


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
byte state;

// Representing different submenu's in the operator menu
// 1 -> configurable delay between sprays, minimum is 15 seconds (because of sprayer)
// 2 -> adjust number of sprays
// 3 -> overview
// 0 -> exit
byte submenu;

// represeting different selections in the current menu
// this is close to an index of an array, i.e. 0 is the first item and 4 is the 3rd item

// overview: this correlates to 1,2 and 0 from the submenu comments

// number of sprays:
//                    0 = exit
//                    1 = reset
//                    maybe + and - later on

// configurable delay:
//                    1 = '+'
//                    2 = '-'
//                    3 = reset to 15 seconds
//                    0 = exit
byte menuSelection;

/////////////////////////////////////////////////////
// Sensor variables;
bool door;

// consume when used in spray
bool override;

// update the myTime value with the current time
void updateTime() {
  // no roll-over protection necessary since the rest is safe code
  myTime = millis();
}

// Write to EEPROM and also update the value in memory
void writeEEPROM_SPRAYS(int value) {
  byte num_0 = value >> 8;
  byte num_1 = value & 0xFF;

  EEPROM.write(SPRAYS_0, num_0);
  EEPROM.write(SPRAYS_1, num_1);
  sprays = value;
}

void writeEEPROM_DELAY(byte value) {
  EEPROM.write(SPRAY_DELAY, value);
  sprayDelay = value;
}

// initialze EEPROM if necessary.
// If the first byte of the int is 255
// then we asume it is never written to
// because the value should not have exceeded 2400 in the first place
void initializeEEPROM() {
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
  byte delay = EEPROM.read(sprayDelay);
  if (delay == 255) {
    writeEEPROM_DELAY(0);
  }
  else
  {
    sprayDelay = delay;
  }
}

// Generic printer, takes name of value and the value in string 
// as well as the x,y location for the LCD
void printSensor(String name, String value, byte line_x, byte line_y) {
  lcdScreen.setCursor(line_x,line_y);
  name.concat(": " + value);
  lcdScreen.print(name);
}

// debug purposes for now
void printDistance(byte x, byte y) {
  String value = String(sonar.ping_cm());
  value.concat(" cm");
  printSensor("dist", value, x, y);
}

// mandatory sensor reading
void printTemperature(byte x, byte y) {
  tempSensor.requestTemperatures();
  // neccesary to convert to int first, because it will be a double otherwise
  double tempValue = tempSensor.getTempCByIndex(0);
  String tempOutput = String(tempValue);
  printSensor("temp", tempOutput, x, y);
}

void printLDR(byte x, byte y) {
  int LDR_value = analogRead(LDR);
  String output = String(LDR_value);
  printSensor("LDR", output, x , y);
}

void printMotion(byte x, byte y) {
  int motionValue = digitalRead(MOTION);
  String output = String(motionValue);
  printSensor("Motion", output, x, y);
}

byte buttonFromValue(unsigned int value) {
  if (value < BUTTON_ONE) {
    return 1;
  }
  if (value < BUTTON_TWO) {
    return 2;
  }
  if (value < BUTTON_THREE) {
    return 3;
  }
  // no button pressed
  return 0;
}

void updateButtonState() {
  unsigned int reading = analogRead(BUTTON_BUS);
  byte currentButtonState = buttonFromValue(reading);
  if (currentButtonState != lastButtonState) {
    buttonDebounceTimer = millis();
  }  
  if ((millis() - buttonDebounceTimer) > DEBOUNCE_DURATION) {
      buttonState = currentButtonState;
      if (buttonState /= 0) {
        isPressed = true;
      }
  }
  lastButtonState = currentButtonState;
}

void spray(unsigned long time, byte x, bool isLow) {
  if (override){
    return;
  }

  if (isLow){
    digitalWrite(MOS, HIGH);
  }
  unsigned long currentTime = millis();
  if ((currentTime - time) < 16){
    spray(time, x, true);
  }
  else {
    digitalWrite(MOS,LOW);
    writeEEPROM_SPRAYS(sprays--);
    if( x > 1){    
      unsigned long newTime = millis();
      spray(newTime, x--, true);
    }
  }
}

void overrideSpray() {

  //
  override = false;
}

void updateSensors() {

}

byte iterateMenu(byte maxMenuValue) {
  if (menuSelection > 0) {
    menuSelection++;
    if (menuSelection > maxMenuValue) {
      return 0;
    }
    return menuSelection;
  }
}

void nonMenuButtonAction() {
  switch (buttonState) {
      break;
    case 2:
      state = 7;
      break;
    default:
      break;
  }
}

void menuOverviewButtonAction() {
  switch (buttonState) {
    case 2:
      menuSelection = iterateMenu(2);
      break;
    case 3:
     submenu = menuSelection;
     menuSelection = 1;
     break;
  }
}

void returnToMenuOverview() {
  submenu = 3;
  menuSelection = 1;
}

void menyDelayAction() {
  switch (menuSelection) {
    case 1:
      if (sprayDelay < 254) {sprayDelay++;}
      break;
    case 2:
      if(sprayDelay > 0) {sprayDelay--;}
      break;
    case 3:
      sprayDelay = 0;
    case 0:
      writeEEPROM_DELAY(sprayDelay);
      returnToMenuOverview();
      break;
  }
}

void menuDelayButtonAction() {
      switch (buttonState) {
    case 2:
      menuSelection = iterateMenu(3);
      break;
    case 3:
      menyDelayAction();
      break;
  }
}

void selectNumOfSpraysAction() {
  if (menuSelection == 1) {
    writeEEPROM_SPRAYS(2400);
    return;
  }
  returnToMenuOverview();
}

void menuNumOfSpraysButtonAction() {
  switch (buttonState) {
    case 2:
      menuSelection = iterateMenu(1);
      break;
    case 3:
      selectNumOfSpraysAction();
      break;
  }
}

void menuButtonAction() {
  switch (submenu) {
    case 1:
      menuDelayButtonAction();
      break;
    case 2:
      menuNumOfSpraysButtonAction();
      break;
    case 3:
      menuOverviewButtonAction();
      break;
    default:
      break;
  }
}

void actOnStateWithButton() {
  if (buttonState == 1){
    override = true;
    return;
  }
  if (state == 7) {
    menuButtonAction();
    return;
  }
  nonMenuButtonAction();
}

void actOnMenuSelection() {

}

void printMenuOveriewToLCD() {

}

void printMenuDelay() {

}

void printMenuSprays() {
  char arrowChar = 27;
  String arrow = String(arrowChar);
  //String plus = "+";
  //String minus = "-";
  String value = String(sprays);
  String reset = "reset";
  String exit = "exit";
  switch (menuSelection) {
    case 0:
      exit.concat(arrow);
      break;
    case 1:
      reset.concat(arrow);
      break;
  }
  String lineZero = "sprays left:" + value;
  String lineOne = reset + "    " + exit;
  lcdScreen.setCursor(0,0);
  lcdScreen.print(lineZero);
  lcdScreen.setCursor(0,1);
  lcdScreen.print(lineOne);
}

void printMenuToLCD() {
  switch (submenu) {
    case 1:
      printMenuDelay();
      break;
    case 2:
      printMenuSprays();
      break;
    case 3: 
      printMenuOveriewToLCD();
      break;
  }
}

void printToLCD() {
  lcdScreen.clear();
  if (state == 7) {
    printMenuToLCD();
    return;
  }
  printTemperature(0,0);
  printLDR(0,1);
}

void setup() {
  myTime = millis();
  lcdScreen.begin(2,16);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MOTION, INPUT);
  pinMode(LDR, INPUT);
  digitalWrite(LED_BUILTIN,HIGH);

  // default values for selection
  submenu = 3;
  menuSelection = 1;


  // delay to stabilize motion sensor
  // delay(60000);

  // set state default
  state = 0;
}

void loop() {
  // the running timer is constantly updated
  updateTime();

  // // clear LCD? maybe -> quickfix for now
  // if(myTime%500 == 0){
  //   lcdScreen.clear();
  // }  
  // // print temperate every 2 seconds on line 1
  // // if(myTime%2000 == 0){
  // //   printTemperature(0,0);
  // // }
  // // print light-readings every half second on line 2
  // if(myTime%500 == 0 & true){
  //   printLDR(0,1);
  // }
  // // print motion-readings every half second on line 2
  // // if(myTime%500 == 0){
  // //   printMotion(0,1);
  // // }
  // // Blink on pin13 and builtin led every half second
  // if(myTime%1000 == 0){
  //   digitalWrite(LED_BUILTIN, HIGH);
  // } else if(myTime%500 == 0){
  //   digitalWrite(LED_BUILTIN, LOW);
  // }
  // if(myTime%500 == 0){
  //   printTemperature(0,0);
  // }

  // update the current buttonState
  updateButtonState();

  //
  if (isPressed){
  actOnStateWithButton();
  isPressed = false;
  }

  // print to LCD every 100ms
  if(myTime%100 == 0){
    printToLCD();
  }

}


