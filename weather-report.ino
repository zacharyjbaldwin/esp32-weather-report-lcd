/*
 *  File:      weather-report.ino
 *  Author:    Zachary Baldwin
 *  Email:     zacharyjbaldwin@gmail.com
 *  Desc:      Gets the weather using the OpenWeatherMap API and displays it on a 20x4 LCD.
 *  License:   MIT
 *
 *  This software is written to work on the SparkFun ESP32 Thing Plus.
 *  Use as your own risk.
 *
 *  See wiring diagrams and schematics at https://www.github.com/zacharyjbaldwin/esp32-weather-station
 */

#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <SPI.h>
#include <HTTPClient.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
  
// WiFi network details:
const char* ssid = "BALDWIN";
const char* password = "4696301617";

// API details:
const String HOST_NAME = "api.openweathermap.org";
const String API_KEY = "ca5824a6fe1ed63c19c2f45c04a1d078";
const String UNITS = "imperial";
const String ZIP_CODE = "75495";

// Denotes the LED pin used for status blinking:
const int LED_PIN = LED_BUILTIN;

// Weather variables:
bool gotWeather = false;
int temp = 0;
int temp_min = 0;
int temp_max = 0;
String desc = "0";

// LCD object. This is a 20x4 LCD.
LiquidCrystal_I2C lcd(0x3F, 20, 4);

void setup()
{
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  delay(10);

  lcd.init();
  lcd.backlight();
  displayWaitingOnWiFiMessage();

  // Connect to WiFi:

  Serial.print("Attemping to connect to "); \
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
    Serial.print(".");
    delay(250);
  }

  Serial.print("done.");
  digitalWrite(LED_PIN, HIGH);

  Serial.println();

  Serial.print("My IP address is ");
  Serial.println(WiFi.localIP());

  // EVERYTHING BELOW PERTAINS TO LCD
  
  displayWiFiConnectSuccessMessage();
  delay(1000);

  getWeather();
}

void loop()
{
  if (gotWeather)
  {
    displayWeatherInfo();
  }
  else
  {
    displayFailedToFetchWeatherMessage();
  }

  for (int i = 10; i >= 1; i--)
  {
    lcd.setCursor(0, 3);
    lcd.print(" Next update: ");
    lcd.print(i);
    Serial.print(i);
    lcd.print("m ");
    delay(1000);
  }
  
  Serial.println("done.");
//  delay(10000);

  
}

String getURL()
{
  String url = "https://" + HOST_NAME + "/data/2.5/weather?zip=" + ZIP_CODE + "&appid=" + API_KEY + "&units=" + UNITS;
  return url.c_str();
}

void getWeather()
{
  Serial.println("Starting connection to api.openweathermap.org...");
  HTTPClient http;

  http.begin(getURL());
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);

    String payload = http.getString();
    Serial.println(payload);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    JsonObject obj = doc.as<JsonObject>();

    temp = obj["main"]["temp"];
    temp_min = obj[String("main")][String("temp_min")];
    temp_max = obj[String("main")][String("temp_max")];
    desc = obj["weather"][0]["description"].as<String>();

    Serial.println(temp);
    Serial.println(temp_min);
    Serial.println(temp_max);
    Serial.println(desc);

    gotWeather = true;
  }
  else
  {
    Serial.print("Failed to connect to ");
    Serial.print(HOST_NAME);
    Serial.print(" Error code: ");
    Serial.println(httpResponseCode);

    gotWeather = false;
  }

  http.end();
}

void displayWaitingOnWiFiMessage()
{
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("Waiting on WiFi...");

  lcd.setCursor(0, 2);
  
  lcd.print("Trying to connect to");
  lcd.setCursor(0, 3);
  lcd.print(ssid);
}

void displayWiFiConnectSuccessMessage()
{
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("Success! My IP is:");

  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
}

// Displays the weather info on the LCD.
void displayWeatherInfo()
{
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(" Current Temp:  ");
  lcd.print(temp);
  lcd.print("F");

  lcd.setCursor(0, 1);
  lcd.print(" Min: ");
  lcd.print(temp_min);
  lcd.print("F");

//  lcd.setCursor(0, 2);
  lcd.print("  Max: ");
  lcd.print(temp_max);
  lcd.print("F");

  lcd.setCursor(0, 2);
  lcd.print(" Desc: ");
  lcd.print(desc);
}

// Displays a message on the LCD that the weather could not be fetched.
void displayFailedToFetchWeatherMessage()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Failed to fetch");
  lcd.setCursor(0, 1);
  lcd.print("weather data.");
  lcd.setCursor(0, 2);
  lcd.print("Restart device or");
  lcd.setCursor(0, 3);
  lcd.print("wait for refresh.");
}
