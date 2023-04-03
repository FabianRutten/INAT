// WIFI
#include <WiFiManager.h>
WiFiManager wifiManager;
// END WIFI

// MQTT
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* Adafruit.io Setup *********************************/
// Fabian's inlog
#define AIO_SERVER      "mqtt.uu.nl"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL -> uu has no ssl
#define AIO_USERNAME    "student036"
#define AIO_KEY         "bbkgsFeZ"

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient wifiClient;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&wifiClient, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);


/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");


// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

// END MQTT


// OneWire
#include <Wire.h>
// END OneWire

// AnalogSwitch
#define ANALOG_SWITCH_SEL D3
// END AnalogSwitch

// DISPLAY
// #include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an NodeMCU v2:       D2(SDA), D1(SCL)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< Data adress for oled
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// END DISPLAY

// SERVO
#include <Servo.h>
Servo brrt;
int servoPos = 0;
#define SERVO_PWM D0
// END SERVO

// ANALOG_SELECTOR
#define ANALOG_SEL D3
#define ANALOG_PIN A0
// END ANALOG_SELECTOR

// DOGE
#define doge_width 60
#define doge_height 60
static const unsigned char doge_xmb[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 
  0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x10, 0x00, 
  0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x40, 0x0C, 0x00, 
  0x00, 0x00, 0x50, 0x00, 0x00, 0x20, 0x1F, 0x00, 0x00, 0x00, 0xE0, 0x00, 
  0x00, 0xD0, 0x19, 0x00, 0x00, 0x00, 0x90, 0x02, 0x00, 0xA8, 0x0F, 0x00, 
  0x00, 0x00, 0x40, 0x24, 0x00, 0xDC, 0x0F, 0x00, 0x00, 0x00, 0x10, 0x43, 
  0x46, 0xEC, 0x17, 0x00, 0x00, 0x00, 0x80, 0x08, 0xD4, 0xFA, 0x1F, 0x00, 
  0x00, 0x00, 0x14, 0xBA, 0xA1, 0xF7, 0x1B, 0x00, 0x00, 0x00, 0x24, 0x49, 
  0x64, 0xF7, 0x0F, 0x00, 0x00, 0x00, 0x80, 0x96, 0x89, 0xFE, 0x03, 0x00, 
  0x00, 0x80, 0x41, 0x8D, 0x16, 0xF6, 0x09, 0x00, 0x00, 0x00, 0x00, 0x32, 
  0x62, 0xF9, 0x0F, 0x00, 0x00, 0x00, 0x90, 0x45, 0x6A, 0x78, 0x0E, 0x00, 
  0x00, 0x80, 0x42, 0x02, 0x96, 0xCA, 0x15, 0x00, 0x00, 0xE0, 0xB7, 0xA5, 
  0x05, 0x44, 0x0E, 0x00, 0x00, 0xE0, 0x47, 0x01, 0x70, 0x91, 0x0B, 0x00, 
  0x00, 0xC0, 0x97, 0xC4, 0x6B, 0x8A, 0x0E, 0x00, 0x00, 0xC0, 0x0B, 0xF2, 
  0x87, 0x21, 0x1D, 0x00, 0x00, 0xC0, 0x01, 0xBA, 0x1F, 0x14, 0x1C, 0x00, 
  0x00, 0x60, 0x92, 0xDD, 0x6F, 0x51, 0x1A, 0x00, 0x00, 0x00, 0x28, 0xFE, 
  0x0F, 0x14, 0x08, 0x00, 0x00, 0x90, 0x00, 0xFC, 0x27, 0x80, 0x1B, 0x00, 
  0x00, 0x00, 0x40, 0xE2, 0x54, 0x18, 0x10, 0x00, 0x00, 0x80, 0x04, 0x44, 
  0x01, 0x00, 0x25, 0x00, 0x00, 0xFC, 0x53, 0x22, 0x48, 0x00, 0x4C, 0x00, 
  0x00, 0xFC, 0x07, 0x84, 0x01, 0x20, 0x10, 0x00, 0x00, 0xFC, 0x07, 0x12, 
  0x40, 0x80, 0x06, 0x00, 0x00, 0xFC, 0x07, 0x00, 0x02, 0x08, 0x10, 0x00, 
  0x00, 0xFE, 0x07, 0x00, 0x00, 0x90, 0x10, 0x00, 0x00, 0xFE, 0x9F, 0x12, 
  0x20, 0x08, 0x4D, 0x00, 0x00, 0xFE, 0x07, 0x08, 0x00, 0x41, 0x19, 0x00, 
  0x00, 0xFE, 0x2B, 0x82, 0x42, 0x28, 0x14, 0x00, 0x00, 0xFE, 0xAF, 0xA1, 
  0xA4, 0x49, 0x0A, 0x00, 0x00, 0xFC, 0x87, 0x04, 0x01, 0x00, 0x66, 0x00, 
  0x00, 0xFE, 0x1F, 0x1A, 0x04, 0x99, 0x09, 0x00, 0x00, 0xFC, 0xEF, 0x5F, 
  0x62, 0x00, 0x14, 0x00, 0x00, 0xF8, 0xFF, 0x17, 0x40, 0x22, 0x0D, 0x00, 
  0x00, 0xF0, 0xFF, 0x47, 0x06, 0x88, 0x12, 0x00, 0x00, 0xEA, 0xBD, 0x52, 
  0x60, 0x80, 0x09, 0x00, 0x00, 0xA0, 0x62, 0x09, 0x05, 0x32, 0x12, 0x00, 
  0x00, 0x20, 0x09, 0x88, 0x10, 0x4C, 0x09, 0x00, 0x00, 0x80, 0xA8, 0xA5, 
  0x4A, 0xA2, 0x06, 0x00, 0x00, 0x20, 0x85, 0x41, 0x02, 0x30, 0x00, 0x00, 
  0x00, 0x00, 0x30, 0x1C, 0x40, 0x4A, 0x01, 0x00, 0x00, 0x00, 0x80, 0x81, 
  0x58, 0x04, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x24, 0x01, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x42, 0x1A, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x11, 
  0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  };
// END DOGE

// VARIABLES
unsigned long currentTime = 0;
#define SOIL_DELAY 100
unsigned long soilTimer   = 0;

// state
// 0: default
// 1: soil measurement
// 2: LDR  measurement
// 3: else
byte state = 0;

// threshold when the soil should we watered

#define WATER_DELAY 
// END VARIABLES

/*
void testdrawline() {
  int16_t i;

  display.clearDisplay(); // Clear display buffer

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn line
    delay(1);
  }
  for(i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  for(i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, SSD1306_WHITE);
    display.display();
    delay(1);
  }

  delay(2000); // Pause for 2 seconds
}

void testdrawrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn rectangle
    delay(1);
  }

  delay(2000);
}

void testfillrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2; i+=3) {
    // The INVERSE color is used so rectangles alternate white/black
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, SSD1306_INVERSE);
    display.display(); // Update screen with each newly-drawn rectangle
    delay(1);
  }

  delay(2000);
}

void testdrawcircle(void) {
  display.clearDisplay();

  for(int16_t i=0; i<max(display.width(),display.height())/2; i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void testfillcircle(void) {
  display.clearDisplay();

  for(int16_t i=max(display.width(),display.height())/2; i>0; i-=3) {
    // The INVERSE color is used so circles alternate white/black
    display.fillCircle(display.width() / 2, display.height() / 2, i, SSD1306_INVERSE);
    display.display(); // Update screen with each newly-drawn circle
    delay(1);
  }

  delay(2000);
}

void testdrawroundrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2-2; i+=2) {
    display.drawRoundRect(i, i, display.width()-2*i, display.height()-2*i,
      display.height()/4, SSD1306_WHITE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void testfillroundrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2-2; i+=2) {
    // The INVERSE color is used so round-rects alternate white/black
    display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i,
      display.height()/4, SSD1306_INVERSE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void testdrawtriangle(void) {
  display.clearDisplay();

  for(int16_t i=0; i<max(display.width(),display.height())/2; i+=5) {
    display.drawTriangle(
      display.width()/2  , display.height()/2-i,
      display.width()/2-i, display.height()/2+i,
      display.width()/2+i, display.height()/2+i, SSD1306_WHITE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void testfilltriangle(void) {
  display.clearDisplay();

  for(int16_t i=max(display.width(),display.height())/2; i>0; i-=5) {
    // The INVERSE color is used so triangles alternate white/black
    display.fillTriangle(
      display.width()/2  , display.height()/2-i,
      display.width()/2-i, display.height()/2+i,
      display.width()/2+i, display.height()/2+i, SSD1306_INVERSE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void testdrawchar(void) {
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Not all the characters will fit on the display. This is normal.
  // Library will draw what it can and the rest will be clipped.
  for(int16_t i=0; i<256; i++) {
    if(i == '\n') display.write(' ');
    else          display.write(i);
  }

  display.display();
  delay(2000);
}

void testdrawstyles(void) {
  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Hello, world!"));

  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  display.println(3.141592);

  display.setTextSize(2);             // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.print(F("0x")); display.println(0xDEADBEEF, HEX);

  display.display();
  delay(2000);
}

void testscrolltext(void) {
  display.clearDisplay();

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(F("scroll"));
  display.display();      // Show initial text
  delay(100);

  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
  delay(1000);
}
*/

void drawDoge(void) {
  display.clearDisplay();

  display.drawXBitmap(
    ((display.width()  - 16 )  / 2) - 20,
    ((display.height() - 16) / 2) - 20,
    doge_xmb, doge_width, doge_height, 1);
  display.display();
  delay(1000);
  display.invertDisplay(true);
  delay(1000);
}

void startSoilTest(){
  soilTimer = currentTime;
  digitalWrite(ANALOG_SEL, HIGH);
}

uint endSoilTest(){
  uint value = analogRead(ANALOG_PIN);
  digitalWrite(ANALOG_SEL, LOW);
  state = 0;
  return value;
}

uint LDR_value(){
  // assuming this is only called when LDR is selected
  return analogRead(ANALOG_PIN);
}

void printSoil(){
  display.clearDisplay();
  uint value = endSoilTest();
  display.println("SOIL: " + String(value));
  display.display();
}

void printLDR(){
  display.clearDisplay();
  display.println("LDR: " + String(LDR_value()));
  display.display();
}

void stateLoop(){
  switch (state){
    case 1:
      if(currentTime - soilTimer >= SOIL_DELAY){
        printSoil();
      }
      break;
    case 2:
      printLDR();
      break;
    case 3:
      break;
    default:
      state = 2;
      break;
  }
}

void updateTime(){
  currentTime = millis();
}

// #define XPOS   0 // Indexes into the 'icons' array in function below
// #define YPOS   1
// #define DELTAY 2

/* void testanimate(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  int8_t f, icons[NUMFLAKES][3];

  // Initialize 'snowflake' positions
  for(f=0; f< NUMFLAKES; f++) {
    icons[f][XPOS]   = random(1 - LOGO_WIDTH, display.width());
    icons[f][YPOS]   = -LOGO_HEIGHT;
    icons[f][DELTAY] = random(1, 6);
    Serial.print(F("x: "));
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(F(" y: "));
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(F(" dy: "));
    Serial.println(icons[f][DELTAY], DEC);
  }

  for(;;) { // Loop forever...
    display.clearDisplay(); // Clear the display buffer

    // Draw each snowflake:
    for(f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, SSD1306_WHITE);
    }

    display.display(); // Show the display buffer on the screen
    delay(200);        // Pause for 1/10 second

    // Then update coordinates of each flake...
    for(f=0; f< NUMFLAKES; f++) {
      icons[f][YPOS] += icons[f][DELTAY];
      // If snowflake is off the bottom of the screen...
      if (icons[f][YPOS] >= display.height()) {
        // Reinitialize to a random position, just off the top
        icons[f][XPOS]   = random(1 - LOGO_WIDTH, display.width());
        icons[f][YPOS]   = -LOGO_HEIGHT;
        icons[f][DELTAY] = random(1, 6);
      }
    }
  }
}
*/

void wifiSetup() {
  display.clearDisplay();
  display.setTextSize(10);
  const char* apName = "jemoederswifi";
  display.println("Trying wifi");
  display.println("AP: " + String(apName));
  display.display();
  wifiManager.autoConnect(apName);
}

/*void screenTester() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);
  // display.display() is NOT necessary after every single drawing command,
  // unless that's what you want...rather, you can batch up a bunch of
  // drawing operations and then update the screen all at once by calling
  // display.display(). These examples demonstrate both approaches...

  testdrawline();      // Draw many lines

  testdrawrect();      // Draw rectangles (outlines)

  testfillrect();      // Draw rectangles (filled)

  testdrawcircle();    // Draw circles (outlines)

  testfillcircle();    // Draw circles (filled)

  //testdrawroundrect(); // Draw rounded rectangles (outlines)

 // testfillroundrect(); // Draw rounded rectangles (filled)

 // testdrawtriangle();  // Draw triangles (outlines)

  //testfilltriangle();  // Draw triangles (filled)

  //testdrawchar();      // Draw characters of the default font

  //testdrawstyles();    // Draw 'stylized' characters

  //testscrolltext();    // Draw scrolling text

  drawDoge();    // Draw a small bitmap image

  // Invert and restore display, pausing in-between
  display.invertDisplay(true);
  delay(1000);
  display.invertDisplay(false);
  delay(1000);

  //testanimate(logo_bmp, LOGO_WIDTH, LOGO_HEIGHT); // Animate bitmaps
}
*/

void setup() {
  // start display
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  //connect serial for debug
  Serial.begin(115200);
  delay(10);
  // setup wifi
  //wifiSetup();
  // mqtt setup

  // Setup MQTT subscription for onoff feed.
  //mqtt.subscribe(&onoffbutton);

  // declaring pins and their modes
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(ANALOG_PIN, INPUT);
  pinMode(ANALOG_SEL, OUTPUT);

  drawDoge();
  delay(1000);
  brrt.attach(SERVO_PWM);
}

void loop() { 
  updateTime();
  if(wifiClient.connected()){
    MQTT_connect();
  }
  digitalWrite(LED_BUILTIN, HIGH);
  state = 1;
  brrt.write(180);
  delay(2000);
  brrt.write(0);
  digitalWrite(LED_BUILTIN, LOW);
  delay(2000);
}
