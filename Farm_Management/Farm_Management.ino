#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "DHT.h"  // Library for the DHT sensor
#define ANALOG_IN_PIN A0
#include <SPI.h>


const char* ssid = "Quantanics"; // Your WiFi SSID
const char* password = "Qu@nt@nics18"; // Your WiFi password

const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char* mqttClientID = "Farm_Management_TCE/Dashboard";
const char* mqttTopic = "Farm_Management";

#define DHTTYPE DHT11   // Define the type of DHT sensor
#define DHTPIN 14        // DHT11 sensor's data line is connected to pin D2

#define RAIN_DROP_PIN 12    // Digital pin for rain drop sensor
#define FLOAT_SENSOR  13      // the number of the float sensor pin
#define LED 16              // the number of the LED pin
#define BUZZER 13           // the number of the buzzer pin


#define SENSOR D4
#define RELAY_PIN 4// Replace 12 with the actual pin number connected to the relay
const int soilMoisturePin = A0; // Analog pin for soil moisture sensor
const int moistureThreshold = 500; // Set your desired threshold here

float h;               // Variable to store humidity
float t;               // Variable to store temperature
int soilMoisture;      // Variable to store soil moisture level
int rainDropValue;     // Variable to store rain drop sensor reading

int floatsensor;       // Variable to store water float sensor reading
DHT dht(DHTPIN, DHTTYPE);


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

WiFiClient espClient;
PubSubClient client(espClient);

void setupWiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqttClientID)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
   setupWiFi();
  client.setServer(mqttServer, mqttPort);

  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect
  }
    dht.begin();           // Start the DHT sensor
    pinMode(RAIN_DROP_PIN, INPUT); // Set rain drop sensor pin as input
    pinMode(FLOAT_SENSOR, INPUT_PULLUP); // Set float sensor pin as input with internal pull-up resistor
    pinMode(LED, OUTPUT);  // Set LED pin as output
    pinMode(BUZZER, OUTPUT);  // Set buzzer pin as output
   
    pinMode(soilMoisturePin, INPUT);
    pinMode(RELAY_PIN, OUTPUT);

    pinMode(SENSOR, INPUT_PULLUP);

    pulseCount = 0;
    flowRate = 0.0;
    flowMilliLitres = 0;
    totalMilliLitres = 0;
    previousMillis = 0;
    attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
    
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }

    // Read DHT sensor
    h = dht.readHumidity();
    t = dht.readTemperature();

    int moistureLevel = analogRead(soilMoisturePin);
    int moisturedata  = map(moistureLevel, 0, 1023, 100, 0);
  
    if (moistureLevel > moistureThreshold) {
    // Soil is dry, activate relay
    digitalWrite(RELAY_PIN, HIGH);
    } else {
    // Soil is moist enough, deactivate relay
    digitalWrite(RELAY_PIN, LOW);
    }
    

    // Read rain drop sensor
    rainDropValue = digitalRead(RAIN_DROP_PIN);

        // Control LED based on soil moisture
    if (rainDropValue == 1) {
        digitalWrite(LED, LOW); // Turn on LED
    } else {
        digitalWrite(LED, HIGH); // Turn off LED
    }

    // Read float sensor
    floatsensor = digitalRead(FLOAT_SENSOR);

    // Control buzzer based on float sensor
    if (floatsensor == LOW) {
        digitalWrite(BUZZER, HIGH); // Turn on buzzer
    } else {
        digitalWrite(BUZZER, LOW); // Turn off buzzer
    }

   
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
    Serial.print(float(flowRate)); // Print the integer part of the variable
    Serial.println(" L/min");

    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print(" mL / ");
    Serial.print(totalLitres);
    Serial.println(" L");
  }
    // Publish sensor readings to MQTT
    String data = String(rainDropValue) + "," + String(moisturedata) + "," +
                  String(floatsensor) + "," + String(t) + "," + String(h) + "," + String(flowRate);
    String topic = "Farm_Management"; // MQTT topic to publish to
    client.publish(mqttTopic, data.c_str());

    Serial.println("Published to MQTT: " + data);
    delay(1000);
}
