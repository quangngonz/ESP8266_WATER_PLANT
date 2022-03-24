#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

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
String ipAddress = "";
int Button = 0;
int buttonState = LOW;

//----------------------------------
int Li          = 16;
int Lii         = 0; 
int Ri          = -1;
int Rii         = -1;
//----------------------------------


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

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
  Serial.print("Moisture: ");
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

// Print scroll right function
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

// Clear screen
int Clear_Scroll_LCD_Right(){
  Ri=-1;
  Rii=-1;
}


void setup(){
  // Serial port for debugging purposes
  Serial.begin(9600);

  lcd.init();                    
  lcd.backlight();
  lcd.print("Moisture : ");
  
  pinMode(Relay, OUTPUT);
  digitalWrite(Relay, LOW);
  pinMode(Button, INPUT);

  timeClient.begin();
  timeClient.setTimeOffset(25200);
  
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
  ipAddress = "IP Adress: " + WiFi.localIP().toString();
  
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
  
  timeClient.update();

  buttonState = digitalRead(Button);
  Serial.println(buttonState);
  
  if (buttonState == LOW){
      Serial.println("Watering");
      digitalWrite(Relay,HIGH);
      delay(3000);
      digitalWrite(Relay,LOW);
    };
  
  if(currentTime - previousTime_led >=1000){
    print_value();

    unsigned long epochTime = timeClient.getEpochTime();

    struct tm *ptm = gmtime ((time_t *)&epochTime); 

    int currentMonth = ptm->tm_mon+1;
    String formattedTime = timeClient.getFormattedTime();
    String weekDay = weekDays[timeClient.getDay()];
    int monthDay = ptm->tm_mday;
    String currentMonthName = months[currentMonth-1];
    int currentYear = ptm->tm_year+1900;

    ///Serial.print("Epoch Time: ");
    ///Serial.println(epochTime);
    
    ///Serial.print("Month: ");
    ///Serial.println(currentMonth);
      
    ///Serial.print("Formatted Time: ");
    ///Serial.println(formattedTime);
  
    ///Serial.print("Week Day: ");
    ///Serial.println(weekDay);

    ///Serial.print("Month day: ");
    ///Serial.println(monthDay);

    ///Serial.print("Month name: ");
    ///Serial.println(currentMonthName);
    
    ///Serial.print("Year: ");
    ///Serial.println(currentYear);

    ///Serial.print("");

    String timeValue = weekDay + " " + monthDay + " " + currentMonthName + " " + currentYear + " " + formattedTime ;
  
    Serial.print("LCD text: "); 
    Serial.println(timeValue);
    
    lcd.setCursor(0, 1);
    lcd.print(Scroll_LCD_Right(timeValue));
    
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
