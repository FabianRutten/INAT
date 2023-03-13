#include <Arduino.h>
#include <LiquidCrystal.h>
#include <NewPing.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

// timeCount variable
unsigned long myTime;

unsigned long printTime;

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
LiquidCrystal lcdScreen(7,8,9,10,11,12);


/// default door value
int default_door = 150;
/// overhead
#define door_overhead 10
/// max value
#define door_max 300
// Distance Sensor, which is sonar based
NewPing sonar(4,5,door_max);


// Light sensor, LDR
#define LDR A0

// magnet sensor, MAG
#define MAG 2

// Button bus, which is used for 3 buttons on analogPin 2
#define BUTTON_BUS A2
// 3 buttons with each 3 threshholds
// each is lower than value
#define BUTTON_ONE 100            // this button is the override!!!
#define BUTTON_TWO 600
#define BUTTON_THREE 800

// the buttons and other sensors might need debouncing, 
// so we define a standard debouncing time in milliseconds
// also a button current state
// and a time variable
#define DEBOUNCE_DURATION 10
byte pressedButton;
#define BUTTON_OVERRIDE      1
#define BUTTON_MENU_ITERATOR 2
#define BUTTON_SELECT        3
#define BUTTON_NON           0
byte lastpressedButton;
unsigned long buttonDebounceTimer;

//RGB
#define RGB_RED A4
#define RGB_GREEN A5
#define RGB_BLUE A3

// Bus for many digital signals on the bus
#define ONE_WIRE_BUS 6

// OneWire protocol to make use of the one_wire_bus
OneWire oneWire(ONE_WIRE_BUS);

// Temperature sensor from Dallas on the oneWire bus
DallasTemperature tempSensor = (&oneWire);

// PowerSwitch on mosfett
#define MOS A1

// Motion sensor digital output
// This sensor is "trigger once" mode
#define MOTION 3

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
#define STATE_NOT_IN_USE 0
#define STATE_UNKNOWN    1
#define STATE_CLEANING   2
#define STATE_NUM1       3
#define STATE_NUM2       4
#define STATE_TRIG1      5
#define STATE_TRIG2      6
#define STATE_MENU       7

// Representing different submenu's in the operator menu
// 1 -> configurable delay between sprays, minimum is 15 seconds (because of sprayer)
// 2 -> adjust number of sprays
// 3 -> overview
byte submenu;
#define SUBMENU_DELAY 1
#define SUBMENU_SPRAYS 2
#define SUBMENU_OVERVIEW 3

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
#define SELECTION_OVERVIEW_DELAY 1
#define SELECTION_OVERVIEW_SPRAY 2
#define SELECTION_OVERVIEW_RESET 3
#define SELECTION_OVERVIEW_EXIT  0
#define OVERVIEW_ITERATOR_NUMBER 3

#define SELECTION_DELAY_PLUS  1
#define SELECTION_DELAY_MINUS 2
#define SELECTION_DELAY_RESET 3
#define SELECTION_DELAY_EXIT  0
#define DELAY_ITERATOR_NUMBER 3

#define SELECTION_SPRAYS_RESET 1
#define SELECTION_SPRAYS_EXIT  0
#define SPRAYS_ITERATOR_NUMBER 1

#define SELECTION_DEFAULT 1

/////////////////////////////////////////////////////
// Sensor variables;
bool flushing;
bool isMotion;
bool buttonPressed;
bool buttonCanBeActivated;
// consume when used in spray
bool override;

bool doorClosed() {
  int reading = sonar.ping_cm();
  return (reading <= (default_door + door_overhead));
}

// update the myTime value with the current time
void updateTime() {
  // no roll-over protection necessary since the rest is safe code
  // all code just compares the two times by subtraction. 
  // Since its unsigned, it won't create problems :)
  myTime = millis();
}

// Write to EEPROM and also update the value in memory
void writeEEPROM_SPRAYS() {
  byte num_0 = sprays >> 8;
  byte num_1 = sprays & 0xFF;

  EEPROM.write(SPRAYS_0, num_0);
  EEPROM.write(SPRAYS_1, num_1);
}

void writeEEPROM_DELAY() {
  EEPROM.write(SPRAY_DELAY, sprayDelay);
}

// initialze EEPROM if necessary.
// If the first byte of the int is 255
// then we asume it is never written to
// because the value should not have exceeded 2400 in the first place
void initializeEEPROM() {
  byte num_0 = EEPROM.read(SPRAYS_0);
  byte num_1 = EEPROM.read(SPRAYS_1);
  int num = (num_0 << 8) + num_1;
  sprays = num;
  byte delay = EEPROM.read(SPRAY_DELAY);
  sprayDelay = delay;
}

// Generic printer, takes name of value and the value in string 
// as well as the x,y location for the LCD
void printSensor(String name, String value, byte line_x, byte line_y) {
  lcdScreen.setCursor(line_x,line_y);
  name.concat(": " + value);
  while (name.length() < 16) {
    name.concat(" ");
  }
  lcdScreen.print(name);
}

void printButtonAnalog(byte x, byte y) {
  String value = String (analogRead(BUTTON_BUS));
  printSensor("button", value, x, y);
}

void printpressedButton(byte x, byte y) {
  String value = String (pressedButton);
  printSensor("button", value, x, y);
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

byte buttonFromValue(int value) {
  if (value < BUTTON_ONE) {
    return BUTTON_OVERRIDE;
  }
  if (value < BUTTON_TWO) {
    return BUTTON_MENU_ITERATOR;
  }
  if (value < BUTTON_THREE) {
    return BUTTON_SELECT;
  }
  return BUTTON_NON;
}

void updatepressedButton() {
  int reading = analogRead(BUTTON_BUS);
  byte currentpressedButton = buttonFromValue(reading);
  if (currentpressedButton != lastpressedButton) {
    buttonDebounceTimer = millis();
  }  
  else if ((millis() - buttonDebounceTimer) > DEBOUNCE_DURATION) {
    if(!buttonPressed){
      pressedButton = currentpressedButton;
      if(buttonCanBeActivated){
        buttonPressed = true;
        buttonCanBeActivated = false;
      }
    }
    if(currentpressedButton == BUTTON_NON){
      buttonCanBeActivated = true;
    }
  }
  lastpressedButton = currentpressedButton;
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
    sprays--;
    writeEEPROM_SPRAYS();
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
  if (digitalRead(MAG) == LOW){
    flushing = false;
  }
}

void printButtonValues(byte x, byte y) {
  String press = "pBG: ";
  press.concat(String(pressedButton) + "   pB");
  printSensor(press, String(buttonFromValue(analogRead(BUTTON_BUS))), 0,1);
}

// interupts
// when magnets meet
// when magnets dont meet
void magnetInterrupt_rising() {
  flushing = true;
}

void motionInterrupt() {
  isMotion = true;
}

// menu

void iterateMenu(byte maxMenuValue) {
  menuSelection++;
  if (menuSelection > maxMenuValue)
    {menuSelection = 0; }
}

void nonMenuButtonAction() {
  if (pressedButton == BUTTON_MENU_ITERATOR){
    state = STATE_MENU;
  }
}

void returnToMenuOverview() {
  submenu = SUBMENU_OVERVIEW;
  menuSelection = SELECTION_DEFAULT;
}

void menuOverviewButtonAction() {
  switch (pressedButton) {
    case BUTTON_MENU_ITERATOR:
      iterateMenu(OVERVIEW_ITERATOR_NUMBER);
      break;
    case BUTTON_SELECT:
      if (menuSelection == SELECTION_OVERVIEW_EXIT){
        state = STATE_UNKNOWN;
        returnToMenuOverview();
      }
      else if (menuSelection == SELECTION_OVERVIEW_RESET){
        default_door = sonar.ping_cm();
      }
      else {
        submenu = menuSelection;
        menuSelection = SELECTION_DEFAULT;
      }
      break;
  }
}

void menuDelayAction() {
  switch (menuSelection) {
    case SELECTION_DELAY_PLUS:
      if (sprayDelay < 254) {sprayDelay++;}
      break;
    case SELECTION_DELAY_MINUS:
      if(sprayDelay > 0) {sprayDelay--;}
      break;
    case SELECTION_DELAY_RESET:
      sprayDelay = 0;
      break;
    case SELECTION_DELAY_EXIT:
      writeEEPROM_DELAY();
      returnToMenuOverview();
      break;
  }
}

void menuDelayButtonAction() {
      switch (pressedButton) {
    case BUTTON_MENU_ITERATOR:
      iterateMenu(DELAY_ITERATOR_NUMBER);
      break;
    case BUTTON_SELECT:
      menuDelayAction();
      break;
  }
}

void selectNumOfSpraysAction() {
  if (menuSelection == SELECTION_SPRAYS_RESET) {
    sprays = 2400;
    writeEEPROM_SPRAYS();
  }
  else {
    returnToMenuOverview();
  }
}

void menuNumOfSpraysButtonAction() {
  switch (pressedButton) {
    case BUTTON_MENU_ITERATOR:
      iterateMenu(SPRAYS_ITERATOR_NUMBER);
      break;
    case BUTTON_SELECT:
      selectNumOfSpraysAction();
      break;
  }
}

void menuButtonAction() {
  switch (submenu) {
    case SUBMENU_DELAY:
      menuDelayButtonAction();
      break;
    case SUBMENU_SPRAYS:
      menuNumOfSpraysButtonAction();
      break;
    case SUBMENU_OVERVIEW:
      menuOverviewButtonAction();
      break;
  }
}

void actOnStateWithButton() {
  if (pressedButton == BUTTON_OVERRIDE){
    override = true;
  }
  else if (state == STATE_MENU) {
    menuButtonAction();
  }
  else {
  nonMenuButtonAction();
  }
}


// printing

void printMenuOveriewToLCD() {
  String arrow = "<";
  String delay = "delay";
  String sprays = "sprays";
  String reset = "reset";
  String exit = "exit";
  switch (menuSelection) {
    case SELECTION_OVERVIEW_EXIT:
      exit.concat(arrow);
      break;
    case SELECTION_OVERVIEW_DELAY:
      delay.concat(arrow);
      break;
    case SELECTION_OVERVIEW_SPRAY:
      sprays.concat(arrow);
      break;
    case SELECTION_OVERVIEW_RESET:
      reset.concat(arrow);
      break;
  }
  String lineZero = delay + "  " + sprays + "    ";
  String lineOne =  reset + "    " + exit + "     ";
  lcdScreen.setCursor(0,0);
  lcdScreen.print(lineZero);
  lcdScreen.setCursor(0,1);
  lcdScreen.print(lineOne);
}

void printMenuDelay() {
  String arrow = "<";
  String plus = "+";
  String minus = "-";
  String value = String(sprayDelay+15);
  String reset = "reset";
  String exit = "exit";
  switch (menuSelection) {
    case SELECTION_DELAY_EXIT:
      exit.concat(arrow);
      break;
    case SELECTION_DELAY_PLUS:
      plus.concat(arrow);
      break;
    case SELECTION_DELAY_MINUS:
      minus.concat(arrow);
      break;
    case SELECTION_DELAY_RESET:
      reset.concat(arrow);
      break;
  }
  String lineZero = "delay:" + value + " " + plus + " " + minus + "    ";
  String lineOne = reset + "    " + exit + "    ";
  lcdScreen.setCursor(0,0);
  lcdScreen.print(lineZero);
  lcdScreen.setCursor(0,1);
  lcdScreen.print(lineOne);
}

void printMenuSprays() {
  String arrow = "<";
  //String plus = "+";
  //String minus = "-";
  String value = String(sprays);
  String reset = "reset";
  String exit = "exit";
  switch (menuSelection) {
    case SELECTION_SPRAYS_EXIT:
      exit.concat(arrow);
      break;
    case SELECTION_SPRAYS_RESET:
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
    case SUBMENU_DELAY:
      printMenuDelay();
      break;
    case SUBMENU_SPRAYS:
      printMenuSprays();
      break;
    case SUBMENU_OVERVIEW: 
      printMenuOveriewToLCD();
      break;
  }
}

void printToLCDWithButton() {
  if (state == STATE_MENU) {
    printMenuToLCD();
    return;
  }
}

void printDefaultToLCD() {
  printSensor("mag", String(flushing), 0,0);
  printMotion(0,1);
}


// view states
void chanceRGBToState(){
  switch(state){
    case STATE_NOT_IN_USE:
      //NO LIGHT (energy savings ;) )
      digitalWrite(RGB_RED, LOW);
      digitalWrite(RGB_GREEN, LOW);
      digitalWrite(RGB_BLUE, LOW);
      break;
    case STATE_UNKNOWN:
      //RED
      digitalWrite(RGB_RED, HIGH);
      digitalWrite(RGB_GREEN, LOW);
      digitalWrite(RGB_BLUE, LOW);
      break;
    case STATE_CLEANING:
      // purple
      digitalWrite(RGB_RED, 1024 * 0.627);
      digitalWrite(RGB_GREEN, 1024 * 0.125);
      digitalWrite(RGB_BLUE, 1024 * 0.941);
      break;
    case STATE_NUM1:
      //GREEN
      digitalWrite(RGB_RED, LOW);
      digitalWrite(RGB_GREEN, HIGH);
      digitalWrite(RGB_BLUE, LOW);
      break;
    case STATE_NUM2:
      //BLUE
      digitalWrite(RGB_RED, LOW);
      digitalWrite(RGB_GREEN, LOW);
      digitalWrite(RGB_BLUE, HIGH);
      break;
    case STATE_TRIG1:
      //WHITE
      digitalWrite(RGB_RED, HIGH);
      digitalWrite(RGB_GREEN, HIGH);
      digitalWrite(RGB_BLUE, HIGH);
      break;
    case STATE_TRIG2:
      //YELLOW
      digitalWrite(RGB_RED, HIGH);
      digitalWrite(RGB_GREEN, HIGH);
      digitalWrite(RGB_BLUE, LOW);
      break;
    case STATE_MENU:
      digitalWrite(RGB_RED, HIGH);
      digitalWrite(RGB_GREEN, 1024 * 0.647);
      digitalWrite(RGB_BLUE, LOW);
      break;
  }
}

void setup() {
  myTime = millis();
  lcdScreen.begin(2,16);
  lcdScreen.setCursor(0,0);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MOTION, INPUT);
  pinMode(LDR, INPUT);
  digitalWrite(LED_BUILTIN,HIGH);

  // default values for selection
  returnToMenuOverview();


  pressedButton = BUTTON_NON;
  lastpressedButton = BUTTON_NON;
  buttonDebounceTimer = 0;
  override = false;

  tempSensor.begin();
  //
  initializeEEPROM();

  lcdScreen.print("eeprom done");
  delay(1000);

  lcdScreen.clear();
  lcdScreen.print("stabilizing...");
  // delay to stabilize motion sensor
  delay(60000);


  // RGB
  pinMode(RGB_RED, OUTPUT);
  pinMode(RGB_GREEN, OUTPUT);
  pinMode(RGB_BLUE, OUTPUT);

  // sensor defaults
  

  // set state default
  state = STATE_NOT_IN_USE;

  buttonPressed = false;
  buttonCanBeActivated = true;

  lcdScreen.clear();
  lcdScreen.print("starting loop");
  delay(1000);
  printTime = 0;

  // attach interrupts
  pinMode(MAG, INPUT);
  attachInterrupt(digitalPinToInterrupt(MAG), magnetInterrupt_rising, RISING);
  pinMode(MOTION, INPUT);
  attachInterrupt(digitalPinToInterrupt(MOTION), motionInterrupt, RISING);
}

void loop() {
  // the running timer is constantly updated
  updateTime();

  // update the current pressedButton
  updatepressedButton();

  updateSensors();

  //
  if (buttonPressed){
  actOnStateWithButton();
  printToLCDWithButton();
  chanceRGBToState();
  buttonPressed = false;
  }
  
  if (state != 7 && (myTime - printTime >= 200)) {
    printDefaultToLCD();
    printTime = myTime;
  }


}


