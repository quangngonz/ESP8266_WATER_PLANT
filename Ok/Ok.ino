#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

// Replace with your network credentials
const char* ssid = "P2608";
const char* password = "hakhoaikem";

// Variables
unsigned long previousTime_water = 0;
unsigned long previousTime_led = 0;

int sensor_pin = A0; 
int output_value ;
int value = HIGH;
int Relay = 16; // GPIO16 (D0)

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Read moisture value from sensor
String ReadMoistureSensor() {
  output_value= analogRead(sensor_pin);
  output_value = map(output_value,550,0,0,100);
  if (isnan(output_value)) {    
    Serial.println("Failed to read from moisture sensor!");
    return "";
  }
  else {
    return String(output_value);
  }
}

// Print moisture value on LCD
int print_value(){
  Serial.println("Checking Moisture ...");
  ReadMoistureSensor();
  Serial.print(output_value);
  Serial.println("%");
  if(output_value<=-10){
    lcd.setCursor(12,0);
    lcd.print(output_value);
    lcd.setCursor(15,0);
    lcd.print("%");
  };
  if(output_value>=10){
    lcd.setCursor(12,0);
    lcd.print(" ");
    lcd.setCursor(13,0);
    lcd.print(output_value);
    lcd.setCursor(15,0);
    lcd.print("%");
  };
  if(output_value>=0 and output_value<10){
    lcd.setCursor(12,0);
    lcd.print("  ");
    lcd.setCursor(14,0);
    lcd.print(output_value);
    lcd.setCursor(15,0);
    lcd.print("%");
  };
  if(output_value<0 and output_value>-10){
    lcd.setCursor(12,0);
    lcd.print(" ");
    lcd.setCursor(13,0);
    lcd.print(output_value);
    lcd.setCursor(15,0);
    lcd.print("%");
  };
}



void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  lcd.init();                    
  lcd.backlight();
  lcd.print("Moisture : ");
  
  pinMode(Relay, OUTPUT);
  digitalWrite(Relay, LOW);
  
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("/moisture", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", ReadMoistureSensor().c_str());
  });

  // Start server
  server.begin();
}
 
void loop(){
  unsigned long currentTime = millis();
  if(currentTime - previousTime_led >=1000){
    print_value();
    previousTime_led = currentTime;
  };

  if(output_value<=23){
    if(currentTime - previousTime_water >=15000){
      Serial.println("Watering");
      digitalWrite(Relay,HIGH);
      delay(3000);
      digitalWrite(Relay,LOW);
      previousTime_water = currentTime;
    };
  };
  
}
