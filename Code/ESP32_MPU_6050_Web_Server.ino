#include <WiFi.h>
#include <Wire.h>
#include <Arduino.h>
#include <Arduino_JSON.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_BMP280.h>
#include "SPIFFS.h"

#define ssid "******"
#define password "******"

AsyncWebServer server(80);
AsyncEventSource events("/events");
JSONVar readings;

Adafruit_BMP280 bmp; // I2C
#define sensor 35 //IR-Sensor
#define motorA1 25 // Motors
#define motorA2 32
#define motorB1 12
#define motorB2 13

bool go;
String header;
String skirtState = "off";
const long timeoutTime = 2000;
unsigned long previousTime = 0;
unsigned long lastTimeTemperature = 0;
unsigned long lastTimeAcc = 0;
unsigned long temperatureDelay = 1000;
unsigned long pressureAltitudeDelay = 1000;
unsigned long accelerometerDelay = 200;
unsigned long currentTime = millis();

Adafruit_ADXL345_Unified adxl;

sensors_event_t a;
float accX, accY, accZ;
float temperature, pressure, altitude;

void initADXL(){
  if (!adxl.begin()) {
    Serial.println("Failed to initialize ADXL345 sensor!");
    while (1);
  }
  Serial.println("ADXL345 sensor initialized successfully!");
}

void initBMP(){
  if (!bmp.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
  Serial.println("BMP sensor initialized successfully!");
}

void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  } else {
    Serial.println("SPIFFS mounted successfully");
  }
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.printf("Connected to WiFi. IP address: %s\n", WiFi.localIP().toString().c_str());
}

String getAccReadings() {
  adxl.getEvent(&a);
  // Get current acceleration values
  accX = a.acceleration.x;
  accY = a.acceleration.y;
  accZ = a.acceleration.z;
  readings["accX"] = String(accX);
  readings["accY"] = String(accY);
  readings["accZ"] = String(accZ);
  String accString = JSON.stringify(readings);
  Serial.printf("Accelerometer Readings: %s\n", accString.c_str());
  return accString;
}

String getTemperature(){
  temperature = bmp.readTemperature();
  Serial.print("Temperature: ");
  Serial.println(temperature);
  return String(temperature);
}

String getPressure() {
  pressure = bmp.readPressure() / 100.0;
  Serial.print("Pressure: ");
  Serial.println(pressure);
  return String(pressure);
}

String getAltitude() {
  altitude = bmp.readAltitude();
  Serial.print("Altitude: ");
  Serial.println(altitude);
  return String(altitude);
}

void setup() {
  Serial.begin(115200);
  initWiFi();
  initSPIFFS();
  //initADXL();
  initBMP();
  pinMode(sensor, INPUT);
  pinMode(motorA1, OUTPUT);
  pinMode(motorA2, OUTPUT);
  pinMode(motorB1, OUTPUT);
  pinMode(motorB2, OUTPUT);
  digitalWrite(motorA1, LOW);
  digitalWrite(motorA2, LOW);
  digitalWrite(motorB1, LOW);
  digitalWrite(motorB2, LOW);

  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
    Serial.println("Client requested index.html");
  });
  
  server.serveStatic("/", SPIFFS, "/");

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  // Define endpoint to turn skirt on
  server.on("/turnSkirtOn", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Turn skirt on endpoint called");
    turnSkirtOn();
    request->send(200, "text/plain", "Skirt turned on");
    Serial.println("Skirt turned on");
  });

  // Define endpoint to turn skirt off
  server.on("/turnSkirtOff", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Turn skirt off endpoint called");
    turnSkirtOff();
    request->send(200, "text/plain", "Skirt turned off");
    Serial.println("Skirt turned off");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  if ((millis() - lastTimeTemperature) > temperatureDelay) {
    // Send temperature readings to webserver
    events.send(getTemperature().c_str(), "temperature_reading", millis());
    lastTimeTemperature = millis();
  }
  if ((millis() - previousTime) > pressureAltitudeDelay) {
    // Send pressure and altitude readings to webserver
    events.send(getPressure().c_str(), "pressure_reading", millis());
    events.send(getAltitude().c_str(), "altitude_reading", millis());
    previousTime = millis();
  }

  if (digitalRead(sensor) == LOW) 5r{
    go = true;
    while (go == true) {
     turnSkirtOn();
      if (digitalRead(sensor) == HIGH) {
         turnSkirtOff(); 

        go = false;
      }
    }
  }
}

void turnSkirtOn() {
  digitalWrite(motorA1, 255);
  digitalWrite(motorA2, 0);
  digitalWrite(motorB1, HIGH);
  digitalWrite(motorB2, LOW);
}

void turnSkirtOff() {
  digitalWrite(motorA1, 0);
  digitalWrite(motorA2, 255);
  digitalWrite(motorB1, LOW);
  digitalWrite(motorB2, HIGH);
  delay(2000);
  digitalWrite(motorA1, 0);
  digitalWrite(motorA2, 0);   
  digitalWrite(motorB1, LOW);
  digitalWrite(motorB2, LOW);  
}
