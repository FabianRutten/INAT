#include <Arduino.h>

// VARIABLES
unsigned long currentTime = 0;
#define SOIL_DELAY 100
boolean soilMeasurementActive = false;
unsigned long soilTimer   = 0;

boolean soilCalibrate = false;
unsigned int measuredSoil = 0;
unsigned int calibratedMoist = 1023;
unsigned int moistThreshold = 512;
unsigned int measuredLDR  = 0;
unsigned int calibratedLDR = 1023;
// state
// 0: default
// 1: soil measurement
// 2: LDR  measurement
// 3: Watering
byte state = 0;
#define STATE_DEFAULT  0
#define STATE_SOIL     1
#define STATE_LDR      2
#define STATE_WATERING 3

#define SENSOR_VERBOSE_DELAY 1000
unsigned long sensorVerboseTimer = 0;
boolean sensorVerbosePublishing = false;
boolean sensorSingularPublish  = false;



void setMoistThresholdFromPercentage(unsigned int per) {
  moistThreshold = (per * calibratedMoist) / 100;
}


// whether thingy is in in manual mode
boolean manual = false;

// threshold when the soil should we watered

unsigned int waterDelay = 5000;
unsigned long waterTimer = 0;
boolean watering = false;


// END VARIABLES

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


// WIFI
#include <WiFiManager.h>
WiFiManager wifiManager;

// legacy
// const char* WIFI_SSID = "Woestgaafsecure";
// const char* WIFI_PASSWORD = "fantazero";
// END WIFI

// SERVO
#include <Servo.h>
Servo brrt;
#define SERVO_PWM D6
#define SERVO_UP 0
#define SERVO_DOWN 175
#define SERVO_DELAY 10
int servoPos = SERVO_UP;
#define SERVO_PERIODIC_ATTACH_DELAY 5000
int servoTimer = 0;
boolean calibratrionDone = false;

void servoUp(){
  brrt.attach(SERVO_PWM);
  brrt.write(SERVO_UP);
  servoPos = SERVO_UP;
  delay(SERVO_DELAY);
  brrt.detach();
}

void servoDown(){
  brrt.attach(SERVO_PWM);
  brrt.write(SERVO_DOWN);
  servoPos = SERVO_DOWN;
  delay(SERVO_DELAY);
  brrt.detach();
}

// END SERVO


// BMP280 SENSOR
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp;
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

float measuredTemperature = 0;
float measuredPressure    = 0;

void bmpSetup() {
  unsigned status;
  status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  //status = bmp.begin();
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  bmp_temp->printSensorDetails();
}

void serialPrintBmp() {
  sensors_event_t temp_event, pressure_event;
  bmp_temp->getEvent(&temp_event);
  bmp_pressure->getEvent(&pressure_event);
  
  Serial.print(F("Temperature = "));
  Serial.print(temp_event.temperature);
  Serial.println(" *C");

  Serial.print(F("Pressure = "));
  Serial.print(pressure_event.pressure);
  Serial.println(" hPa");

  Serial.println();
}

// END BMP280 SENSOR



// GPS LOCATION
// very secret
#define UNWIREDLABS_API_TOKEN "pk.9c2639520bbf5413d6b9d1830ead8535";


// END GPS LOCATION



// OneWire
#include <Wire.h>
// END OneWire

// AnalogSwitch
#define ANALOG_SWITCH_SEL D5
// END AnalogSwitch


// ANALOG_SELECTOR
#define ANALOG_SEL D5
#define ANALOG_PIN A0
// END ANALOG_SELECTOR

// MQTT


/************************* Adafruit.io Setup *********************************/

// Fabian's inlog



#define MQTT_SERVER       "mqtt.uu.nl"
#define MQTT_PORT         1883                   // use 8883 for SSL -> uu has no ssl
#define MQTT_USERNAME     "student036"
#define MQTT_PASSWORD     "bbkgsFeZ"
#define MQTT_ROOT_TOPIC   "infob3it/036/" 
#define MQTT_ID           "espclient_somerandomnumber"
#define MQTT_WILL_TOPIC   "infob3it/036/WaterMachine/LWT" 
#define MQTT_WILL_QOS     1
#define MQTT_WILL_RETAIN  true
#define MQTT_WILL_MESSAGE "K.I.A."


/*******************************************************************************/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

WiFiClient espClient;
PubSubClient pubClient(MQTT_SERVER, MQTT_PORT,espClient);
#define MQTT_VERSION MQTT_VERSION_3_1_1

/*******************************   TOPICS   ***********************************/
#define TOPIC_WATERING "infob3it/036/WaterMachine/Controls/Watering"
#define TOPIC_MANUAL   "infob3it/036/WaterMachine/Controls/Manual"
#define TOPIC_SENSORS_SINGULAR_RETRIEVAL "infob3it/036/WaterMachine/Sensors/SingularRetrieval"
#define TOPIC_SENSORS_VERBOSE "infob3it/036/WaterMachine/Sensors/Verbose"
#define TOPIC_BMP_TEMP "infob3it/036/WaterMachine/Sensors/Temperature"
#define TOPIC_BMP_PRESS "infob3it/036/WaterMachine/Sensors/Pressure"
#define TOPIC_SENSORS_LIGHT "infob3it/036/WaterMachine/Sensors/Light"
#define TOPIC_SENSORS_SOIL "infob3it/036/WaterMachine/Sensors/Soil"
#define TOPIC_SENSORS_CALIBRATE_SOIL "infob3it/036/WaterMachine/Sensors/Calibration/Soil"
#define TOPIC_SENSORS_CALIBRATE_LIGHT "infob3it/036/WaterMachine/Sensors/Calibration/Light"
#define TOPIC_DELAY "infob3it/036/WaterMachine/Delay"

boolean extractBooleanPayload(byte* payload) {
  char load = payload[0];
  return (load == '1');
}

char* BooleanToPayload(boolean boo) {
  char* ch = new char[2];
  if (boo) {
    strcpy(ch,"1");
  } 
  else {
    strcpy(ch,"0");
  }
  return ch;
}

void printWater(){
  display.clearDisplay();
  display.setCursor(20,25);
  display.println("Watering plant...");
  display.display();
  Serial.println("PRINT WATER");
}

void displayBMPData(){
  display.clearDisplay();
  display.setCursor(0, 5);
  display.println("BMP info");
  display.println(" ");
  display.print("temperature = ");
  display.println(measuredTemperature);
  display.print("air pressure = ");
  display.println(measuredPressure);
  display.display();
}

void startWater(){
  if (!watering) {
    servoDown();
    state = 3;
    Serial.println("water started");
    watering = true;
  }
}

void endWater(){
  if (watering) {
    servoUp();
    state = 0;
    Serial.println("water stopped");
    watering = false;
  }
}

void topicWaterHandler(boolean boo) {
  if(boo) {
    Serial.println("mqtt tells to water");
    startWater();
  }
  else {
    Serial.println("mqtt tells to stop water");
    endWater();
  }
}

void publishManual() {
  pubClient.publish(TOPIC_MANUAL, BooleanToPayload(manual), true);
}

void setToManual() {
  digitalWrite(LED_BUILTIN, LOW);
  manual = true;
}

void setToAutomatic() {
  digitalWrite(LED_BUILTIN, HIGH);
  manual = false;
}

void topicManualHandler(boolean boo) {
  if(boo) {
    if(!manual) {
      setToManual();
    }
  }
  else {
    setToAutomatic();
  }
}

void topicSensorsSingularRetrievalHandler(boolean boo) {
  if (boo) {
  sensorSingularPublish = boo;
  pubClient.publish(TOPIC_SENSORS_SINGULAR_RETRIEVAL, "0");
  }
}

void topicSensorsVerboseHandler(boolean boo) {
  sensorVerbosePublishing = boo;
}


void startPublishSoil() {
  soilMeasurementActive = true;
  digitalWrite(ANALOG_SEL, HIGH);
  soilTimer = currentTime;
}


void topicSensorsCalibrationSoilHandler() {
  soilCalibrate = true;
  startPublishSoil();
}

void topicSensorsCalibrationLightHandler() {
  topicSensorsSingularRetrievalHandler(true);
  calibratedLDR = measuredLDR;
}

void topicDelayHandler(byte* payload) {
  unsigned int newDelay = (int)payload;
  waterDelay = newDelay;
}

int extractData(byte* payload, unsigned int lenght) {
  int value;
  std::memcpy(&value, payload, lenght);
  return value;
}

void displayMQTT(char* topic, byte* payload, unsigned int length) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("last message ->");
  display.println("topic: " + String(topic));
  Serial.println("topic: "   + String(topic));
  int value = extractData(payload, length);

  display.println("int payload: " + String(value));
  Serial.println("int payload" + String(value));
  display.display();
}

void callback(char* topic, byte* payload, unsigned int length)
{
  // handle received message
  displayMQTT(topic,payload, length);
  if (!(strcmp(topic,TOPIC_WATERING))) {
    topicWaterHandler(extractBooleanPayload(payload));
  }
  else if (!(strcmp(topic,TOPIC_MANUAL))) {
    topicManualHandler(extractBooleanPayload(payload));
  }
  else if (!(strcmp(topic,TOPIC_SENSORS_SINGULAR_RETRIEVAL))) {
    topicSensorsSingularRetrievalHandler(extractBooleanPayload(payload));
  }
  else if (!(strcmp(topic,TOPIC_SENSORS_VERBOSE))) {
    topicSensorsVerboseHandler(extractBooleanPayload(payload));
  }
  else if (!(strcmp(topic,TOPIC_SENSORS_CALIBRATE_SOIL))) {
    topicSensorsCalibrationSoilHandler();
  }
  else if (!(strcmp(topic,TOPIC_SENSORS_CALIBRATE_LIGHT))) {
    topicSensorsCalibrationLightHandler();
  }
  else if (!(strcmp(topic,TOPIC_DELAY))) {
    topicDelayHandler(payload);
  }
}


void sub(int subNum, const char* topic) {
  boolean sub = pubClient.subscribe(topic);
  if(!sub){
    delay(2000);
    boolean sub_attempt2 = pubClient.subscribe(topic);
    Serial.println("sub" + String(subNum) + ":" + String(sub_attempt2));
  }
}

void setSubscriptions() {
  const int amount = 7;
  const char* topicToSub [amount] = {
    TOPIC_WATERING,
    TOPIC_MANUAL,
    TOPIC_SENSORS_SINGULAR_RETRIEVAL,
    TOPIC_SENSORS_VERBOSE,
    TOPIC_SENSORS_CALIBRATE_SOIL,
    TOPIC_SENSORS_CALIBRATE_LIGHT,
    TOPIC_DELAY
  };

  for (size_t i = 0; i < amount; i++)
  {
    sub(i,topicToSub[i]);
  }
  
  /*
  boolean sub1 = pubClient.subscribe(TOPIC_WATERING);
  if(!sub1){
    delay(2000);
    boolean sub1_attempt2 = pubClient.subscribe(TOPIC_WATERING);
    Serial.println("sub1: " + String(sub1_attempt2));
  }
  boolean sub2 = pubClient.subscribe(TOPIC_MANUAL);
  if(!sub2){
    delay(2000);
    boolean sub2_attempt2 = pubClient.subscribe(TOPIC_MANUAL);
    Serial.println("sub2: " + String(sub2_attempt2));
  }
  boolean sub3 = pubClient.subscribe(TOPIC_SENSORS_SINGULAR_RETRIEVAL);
  if(!sub3){
    delay(2000);
    boolean sub3_attempt2 = pubClient.subscribe(TOPIC_SENSORS_SINGULAR_RETRIEVAL);
    Serial.println("sub3: " + String(sub3_attempt2));
  }
  boolean sub4 = pubClient.subscribe(TOPIC_SENSORS_VERBOSE);
  if(!sub4){
    delay(2000);
    boolean sub4_attempt2 = pubClient.subscribe(TOPIC_SENSORS_VERBOSE);
    Serial.println("sub4: " + String(sub4_attempt2));
  }
  boolean sub5 = pubClient.subscribe(TOPIC_SENSORS_CALIBRATE_SOIL);
  if(!sub5){
    delay(2000);
    boolean sub5_attempt2 = pubClient.subscribe(TOPIC_SENSORS_CALIBRATE_SOIL);
    Serial.println("sub5: " + String(sub5_attempt2));
  }
  boolean sub6 = pubClient.subscribe(TOPIC_SENSORS_CALIBRATE_LIGHT);
  if(!sub6){
    delay(2000);
    boolean sub6_attempt2 = pubClient.subscribe(TOPIC_SENSORS_CALIBRATE_LIGHT);
    Serial.println("sub6: " + String(sub6_attempt2));
  }
  boolean sub7 = pubClient.subscribe(TOPIC_DELAY);
  if(!sub7){
    delay(2000);
    boolean sub7_attempt2 = pubClient.subscribe(TOPIC_DELAY);
    Serial.println("sub7: " + String(sub7_attempt2));
  }
  */
  Serial.println("subs done");
}

boolean mqttConnect(){
  boolean isConnected = pubClient.connect(MQTT_ID
  , MQTT_USERNAME,MQTT_PASSWORD
  , MQTT_WILL_TOPIC, MQTT_WILL_QOS, MQTT_WILL_RETAIN,MQTT_WILL_MESSAGE
  , true);
  display.clearDisplay();
  display.println("MQTT connected:");
  display.println(String(isConnected));
  display.display();
  Serial.println("MQTT: " + String(isConnected));
  
  boolean isAlivePub = pubClient.publish(MQTT_WILL_TOPIC, "Alive", true);
  // delay(3000);
  pubClient.setCallback(callback);
  setSubscriptions();
  return (isConnected && isAlivePub);
}

void mqttSetup() {
  mqttConnect();
  delay(100);
}

unsigned long reconnectTimer = 0;
#define reconnectDelay 5000

void reconnect() {
  // Loop until we're reconnected
  if (!pubClient.connected() && currentTime - reconnectTimer >= reconnectDelay) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttConnect()) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(pubClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      reconnectTimer = currentTime;
    }
  }
}
// END MQTT




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

// must invert back after drawDoge()
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

// END DOGE

//old sensors
/*
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
*/


boolean flashButtonPressed = false;
boolean flashButtonActive  = false;
#define DEBOUNCE_DURATION 50
unsigned long buttonDebounceTimer;
void updateFlashButton() {
  int reading = digitalRead(D3);
  if ((millis() - buttonDebounceTimer) > DEBOUNCE_DURATION) {
    if (!flashButtonPressed){
      if (reading == LOW) {
        flashButtonActive = true;
        buttonDebounceTimer = millis();
      }
      else if (flashButtonActive) {
        flashButtonActive = false;
        flashButtonPressed = true;
        buttonDebounceTimer = millis();
      }
    }
  }
}

void buttonLoop() {
  updateFlashButton();
  if(flashButtonPressed) {
    if (manual) {
      setToAutomatic();
    }
    else {
      setToManual();
    }
    publishManual();
    flashButtonPressed = false;
  }
}


void publishTemp() {
  String str = String(measuredTemperature) + " *C";
  char* payload = new char[str.length()+1];
  strcpy(payload, str.c_str());
  boolean published = pubClient.publish(TOPIC_BMP_TEMP, payload , true);
  Serial.println("published Temp: " + String(payload) + "  , succes: " + String(published));
  delete [] payload;
}

void publishPressure() {
  String str = String(measuredPressure) + " hPa";
  char* payload = new char[str.length()+1];
  strcpy(payload, str.c_str());
  boolean published = pubClient.publish(TOPIC_BMP_PRESS, payload , true);
  Serial.println("published Press: " + String(payload) + "  , succes: " + String(published));
  delete [] payload;
}



#define sensorDelay 60000
unsigned long sensorTimer = 0;

void retrieveBMPSensors() {
  sensors_event_t temp_event, pressure_event;
  bmp_temp->getEvent(&temp_event);
  bmp_pressure->getEvent(&pressure_event);
  
  measuredTemperature = temp_event.temperature;
  measuredPressure    = pressure_event.pressure;
}

void publishBMP() {
  retrieveBMPSensors();
  // serialPrintBmp();
  publishTemp();
  publishPressure();
}

void publishLDR() {
  if(!soilMeasurementActive) {
    double value = analogRead(ANALOG_PIN);
    double percentileValue = 100 * value / 1023;
    String str = String(percentileValue) + "% light";
    char* payload = new char[str.length()+1];
    strcpy(payload, str.c_str());
    boolean published = pubClient.publish(TOPIC_SENSORS_LIGHT, payload , true);
    Serial.println("published LDR: " + String(payload) + "  , succes: " + String(published));
    delete [] payload;
  }
}



double soilPercentile(double value) {
  return 100 * (1+value) / (calibratedMoist +1);
}

void endPublishSoil() {
  soilMeasurementActive = false;
  double value = analogRead(ANALOG_PIN);
  measuredSoil = value;
  if (soilCalibrate) {
    soilCalibrate = false;
    unsigned int oldPer = (100 * moistThreshold) / calibratedMoist;
    calibratedMoist = measuredSoil;
    setMoistThresholdFromPercentage(oldPer);
  }
  digitalWrite(ANALOG_SEL, LOW);
  String str = String(soilPercentile(value)) + "% moist"; 
  char* payload = new char[str.length()+1];
  strcpy(payload, str.c_str());
  boolean published = pubClient.publish(TOPIC_SENSORS_SOIL, payload , true);
  Serial.println("published Soil: " + String(payload) + "  , succes: " + String(published));
  delete [] payload;
}

void publishAnalogSensors() {
  publishLDR();
  startPublishSoil();
}

void publishSensors() {
  publishBMP();
  publishAnalogSensors();
  sensorTimer = currentTime;
}

void sensorLoop() {
  if (sensorSingularPublish){
    sensorSingularPublish = false;
    publishSensors();
  }
  else if (currentTime - sensorTimer >= sensorDelay) {
    publishSensors();
  } 
  else if (sensorVerbosePublishing && currentTime - sensorVerboseTimer >= SENSOR_VERBOSE_DELAY) {
    publishSensors();
    sensorVerboseTimer = currentTime;
  }
  if (soilMeasurementActive && currentTime - soilTimer >= SOIL_DELAY) {
    endPublishSoil();
  }  
}

boolean isMoist() {
  return measuredSoil >= moistThreshold;
}

boolean shouldWater() {
  return !isMoist();
}

void automaticLoop(){
  if(!manual) {
    if(shouldWater() && !watering) {
      startWater();
      pubClient.publish(TOPIC_WATERING, "1", true);
      waterTimer = currentTime;
    }
    if(currentTime - waterTimer >= waterDelay && watering) {
      endWater();
      pubClient.publish(TOPIC_WATERING, "0", true);
    }
  }
}

void periodicServoAttach() {
  if(currentTime - servoTimer >= SERVO_PERIODIC_ATTACH_DELAY) {
    servoTimer = currentTime;
    if(!watering) {
      servoDown();
    }
  }
}

void updateTime(){
  currentTime = millis();
}


void wifiSetup() {
  display.clearDisplay();
  display.setTextSize(1);
  const char* apName = "ESP_wifi";
  display.println("Trying wifi");
  display.println("AP: " + String(apName));
  display.display();
  wifiManager.autoConnect(apName);
  display.clearDisplay();
  display.println("WIFI connected");
  display.display();
  delay(1000);
}

// legacy
// void manualWifiSetup(){
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
//   display.println("wifi: " + String(WiFi.isConnected()));
//   Serial.println("wifi: " + String(WiFi.isConnected()));
//   display.display();
// }

void setup() {
  // connect serial for debug
  Serial.begin(9600);
  delay(100);
  // start display
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.setTextSize(1); // Draw 2X-scale text
  delay(200);

  // start sensors
  bmpSetup();

  // setup wifi
  wifiSetup();
  delay(100);
  // mqtt setup
  mqttSetup();
  // Setup MQTT subscription for onoff feed.
  //mqtt.subscribe(&onoffbutton);

  // declaring pins and their modes
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ANALOG_PIN, INPUT);
  pinMode(ANALOG_SEL, OUTPUT);

  drawDoge();
  delay(1000);
  // must invert back after drawDoge()
  display.invertDisplay(false);

  display.setTextColor(SSD1306_WHITE);

  publishSensors();
}

void loop() { 
  updateTime();

  buttonLoop();

  sensorLoop();
  
  automaticLoop();

  periodicServoAttach();

  if (!pubClient.connected() && WiFi.isConnected()) {
    reconnect();
  }
  pubClient.loop();
}
