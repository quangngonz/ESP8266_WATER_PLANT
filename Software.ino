#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


// Start the LCD 16x2
LiquidCrystal_I2C lcd(0x27,16,2);


// Replace with your network credentials
const char* ssid = "P2608";
const char* password = "hakhoaikem";

// Variables
unsigned long previousTime_water = 0; // Previous watering time
unsigned long previousTime_led = 0;   // Previous lcd update time
int sensor_pin = A0;                  // Analog pin for moisture sensor
int output_value ;                    // Output value of soil moisture
int Relay = 16;                       // GPIO16 (D0)
int timezone = 7;
int dst = 0;
int Li          = 16;
int Lii         = 0; 
int Ri          = -1;
int Rii         = -1;

//Lcd scrolling __________________________________________________________
String Scroll_LCD_Left(String StrDisplay){
  String result;
  String StrProcess = "                " + StrDisplay + "                ";
  result = StrProcess.substring(Li,Lii);
  Li++;
  Lii++;
  if (Li>StrProcess.length()){
    Li=16;
    Lii=0;
  }
  return result;
}

void Clear_Scroll_LCD_Left(){
  Li=16;
  Lii=0;
}
//----------------------------------
String Scroll_LCD_Right(String StrDisplay){
  String result;
  String StrProcess = "                " + StrDisplay + "                ";
  if (Rii<1){
    Ri  = StrProcess.length();
    Rii = Ri-16;
  }
  result = StrProcess.substring(Rii,Ri);
  Ri--;
  Rii--;
  return result;
}

void Clear_Scroll_LCD_Right(){
  Ri=-1;
  Rii=-1;
}
//_______________________________________________________________________________

// Read moisture value from sensor
String ReadMoistureSensor() {
  output_value= analogRead(sensor_pin);
  output_value = map(output_value,550,0,0,100);
  if (isnan(output_value)) {    
    Serial.println("Failed to read from moisture sensor!");
    return "";
  }
  else {
    Serial.println(output_value);
    Serial.print("%");
  }
}

int showtime(){
  time_t now = time(nullptr);
  lcd.setCursor(0,1);
  lcd.print(Scroll_LCD_Right(ctime(&now)));
}

// Water the plant 
int Water_plant(){
  digitalWrite(Relay,HIGH);                   // Turn on relay, water pump
  delay(3000);                                // Wait 3 seconds
  digitalWrite(Relay,LOW);                    // Turn off relay, water pump
}

// Print moisture value on LCD
int print_value(){
  Serial.println("Checking Moisture ...");
  ReadMoistureSensor();
  Serial.println(output_value);
  Serial.print("%");
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


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Set up LCD 16x2 
  lcd.init();                    
  lcd.backlight();
  lcd.print("Moisture : ");

  //Set up Relay
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

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  configTime(timezone * 3600, 0, "pool.ntp.org", "time.nist.gov");
  
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

void loop() {
  // Get snapshot of current time
  unsigned long currentTime = millis();
  File file = SPIFFS.open("/data.csv", "a");
  
  if(currentTime - previousTime_led >=1000){  // Check if 1 second has passed 
    print_value();                            // Print current moisture Percentage
    showtime();
    previousTime_led = currentTime;           // Change the previous time to current time
  
  //print current time on the txt doc
    time_t now = time(nullptr);
    file.println(ctime(&now));
    file.print(",");
    file.print(output_value);
    file.print("%");
  };

  if(output_value<=35){
    if(currentTime - previousTime_water >=20000){ // Check if 20 seconds have passed
      Serial.println("Watering");
      Water_plant();
      previousTime_water = currentTime;           // Change the previous time to current time
    };
  };
  file.close();
}
