#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>

String apiKey = "HP0GPWEO0K9W1FX9"; // Enter your Write API key from ThingSpeak
const char *ssid = "iPad ของกฤษณพง";     // replace with your wifi ssid
const char *pass = "11111111"; // replace with your wifi password
const char* server = "api.thingspeak.com";

#define SENSOR  2

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

WiFiClient client;

void setup()
{
  Serial.begin(115200);
  delay(100);

  WiFi.begin(ssid, pass); // Connect to Wi-Fi

  pinMode(SENSOR, INPUT_PULLUP);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
}

void loop()
{
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) 
  {
    pulse1Sec = pulseCount;
    pulseCount = 0;

    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    flowMilliLitres = (flowRate / 60) * 1000;
    flowLitres = (flowRate / 60);

    totalMilliLitres += flowMilliLitres;
    totalLitres += flowLitres;

    Serial.print("Flow rate: ");
    Serial.print(float(flowRate));  
    Serial.print("L/min");
    Serial.print("\t");       

    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalLitres);
    Serial.println("L");

    if (client.connect(server, 80)) 
    {
      String postStr = apiKey;
      postStr += "&field1=";
      postStr += String(float(flowRate)); // Send flow rate to Field 1
      postStr += "&field2=";
      postStr += String(totalLitres * 4); // Calculate cumulative cost and send to Field 2
      postStr += "\r\n\r\n";
    
      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);
    }
    client.stop();
  }
}
