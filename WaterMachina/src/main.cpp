// WIFI
#include <WiFiManager.h>
WiFiManager wifiManager;
// END WIFI


// MQTT


/************************* Adafruit.io Setup *********************************/

// Fabian's inlog
#define MQTT_SERVER      "mqtt.uu.nl"
#define MQTT_PORT         1883                   // use 8883 for SSL -> uu has no ssl
#define MQTT_USERNAME    "student036"
#define MQTT_PASSWORD         "bbkgsFeZ"


/*******************************************************************************/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "........";
const char* password = "........";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// void setup_wifi() {

//   delay(10);
//   // We start by connecting to a WiFi network
//   Serial.println();
//   Serial.print("Connecting to ");
//   Serial.println(ssid);

//   WiFi.mode(WIFI_STA);
//   WiFi.begin(ssid, password);

//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }

//   randomSeed(micros());

//   Serial.println("");
//   Serial.println("WiFi connected");
//   Serial.println("IP address: ");
//   Serial.println(WiFi.localIP());
// }

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}


unsigned long reconnectTimer = 0;
#define reconnectDelay 5000

void reconnect() {
  // Loop until we're reconnected
  if (!client.connected() && currentTime - reconnectTimer >= reconnectDelay) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      reconnectTimer = currentTime;
    }
  }
}

// void setup() {
//   pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
//   Serial.begin(115200);
//   setup_wifi();
//   client.setServer(mqtt_server, 1883);
//   client.setCallback(callback);
// }

// void loop() {

//   if (!client.connected()) {
//     reconnect();
//   }
//   client.loop();

//   unsigned long now = millis();
//   if (now - lastMsg > 2000) {
//     lastMsg = now;
//     ++value;
//     snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
//     Serial.print("Publish message: ");
//     Serial.println(msg);
//     client.publish("outTopic", msg);
//   }
// }

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
#define SERVO_UP 0
#define SERVO_DOWN 180
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
// 3: Watering
byte state = 0;

// threshold when the soil should we watered

#define WATER_DELAY 1000
unsigned long waterTimer = 0;
// END VARIABLES

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
  state = 1;
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
  delay(1000);
}

void printLDR(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("LDR: " + String(LDR_value()));
  display.display();
  delay(1000);
  startSoilTest();
}

void printWater(){

}

void startWater(){
  brrt.write(SERVO_DOWN);
  waterTimer = currentTime;
  state = 3;
  printWater();
}



void endWater(){
  brrt.write(SERVO_UP);
  state = 0;
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
      if(currentTime - waterTimer >= WATER_DELAY){
        endWater();
      }
      break;
    default:
      state = 2;
      break;
  }
}

void updateTime(){
  currentTime = millis();
}


void wifiSetup() {
  display.clearDisplay();
  display.setTextSize(10);
  const char* apName = "jemoederswifi";
  display.println("Trying wifi");
  display.println("AP: " + String(apName));
  display.display();
  wifiManager.autoConnect(apName);
}


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
  display.invertDisplay(false);
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  brrt.attach(SERVO_PWM);
}

void loop() { 
  updateTime();
  // if(wifiClient.connected()){
  //   MQTT_connect();
  // }
  stateLoop();
}
