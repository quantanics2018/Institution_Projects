#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <SoftwareSerial.h>
#include <Servo.h>

Adafruit_BMP085 bmp;
Servo myservo;  // create servo object to control a servo

int gasSensorPin = A0; // Assuming analog input for gas sensor
float gasSensorValue = 0.0;

int ledPin = 13; // Assuming the LED is connected to digital pin 13
unsigned long previousMillis = 0; // Store the previous millis value
const long interval = 1000; // Interval at which to blink the LED (milliseconds)

// Define pins for IR sensor
int irSensorPin = 12; // Assuming IR sensor connected to digital pin 12

// Define pin for LDR sensor
int ldrPin = 3; // Assuming analog input for LDR sensor

// Define pins for ultrasonic sensor
const int trigPin = 5;
const int echoPin = 2;
SoftwareSerial espSerial(0, 1);

String str;

// Define variables
long duration;
int distance;

void setup() {
  Serial.begin(115200);
  espSerial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(irSensorPin, INPUT); // Set IR sensor pin as input
  pinMode(trigPin, OUTPUT); // Set ultrasonic sensor trigger pin as output
  pinMode(echoPin, INPUT); // Set ultrasonic sensor echo pin as input

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }

  myservo.attach(6,600,2300);  // (pin, min, max)
}

void loop() {
  unsigned long currentMillis = millis(); // Get the current time

  // Read BMP085 sensor data
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure();
  float altitude = bmp.readAltitude();

  // Read gas sensor data
  gasSensorValue = analogRead(gasSensorPin);

  // Read IR sensor data
  int irSensorValue = digitalRead(irSensorPin);

  // Read LDR sensor data
  int ldrValue = digitalRead(ldrPin);

  // Clear the trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send a 10 microsecond pulse to trigger pin
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the duration of pulse from echo pin
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance in centimeters
  distance = duration * 0.034 / 2;

  // Control LED based on sensor readings
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
  delay(1000);

  // Blink LED with 1 second interval
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    digitalWrite(ledPin, !digitalRead(ledPin)); // Toggle LED state
  }

  // Control servo motor independently
  myservo.write(0);  // tell servo to go to a particular angle
  delay(1000);
  
  myservo.write(90);              
  delay(500); 
  
  myservo.write(135);              
  delay(500);
  
  myservo.write(180);              
  delay(1500);                     

  // Construct the string without labels
  str = String(temperature) + "," +
        String(pressure) + "," +
        String(altitude) + "," +
        String(distance) + "," +
        String(irSensorValue) + "," +
        String(ldrValue) + "," +
        String(gasSensorValue);

  // Send the string to ESP8266
  espSerial.println(str);

  // Print data to serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" Pa");
  Serial.print("Altitude: ");
  Serial.print(altitude);
  Serial.println(" meters");
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  Serial.print("IR Sensor Value: ");
  Serial.println(irSensorValue);
  Serial.print("LDR Sensor Value: ");
  Serial.println(ldrValue);
  Serial.print("Gas Sensor Value: ");
  Serial.println(gasSensorValue);

  delay(2000);
}#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <SoftwareSerial.h>
#include <Servo.h>

Adafruit_BMP085 bmp;
Servo myservo;  // create servo object to control a servo

int gasSensorPin = A0; // Assuming analog input for gas sensor
float gasSensorValue = 0.0;

int ledPin = 13; // Assuming the LED is connected to digital pin 13
unsigned long previousMillis = 0; // Store the previous millis value
const long interval = 1000; // Interval at which to blink the LED (milliseconds)

// Define pins for IR sensor
int irSensorPin = 12; // Assuming IR sensor connected to digital pin 12

// Define pin for LDR sensor
int ldrPin = 3; // Assuming analog input for LDR sensor

// Define pins for ultrasonic sensor
const int trigPin = 5;
const int echoPin = 2;
SoftwareSerial espSerial(0, 1);

String str;

// Define variables
long duration;
int distance;

void setup() {
  Serial.begin(115200);
  espSerial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(irSensorPin, INPUT); // Set IR sensor pin as input
  pinMode(trigPin, OUTPUT); // Set ultrasonic sensor trigger pin as output
  pinMode(echoPin, INPUT); // Set ultrasonic sensor echo pin as input

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }

  myservo.attach(6,600,2300);  // (pin, min, max)
}

void loop() {
  unsigned long currentMillis = millis(); // Get the current time

  // Read BMP085 sensor data
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure();
  float altitude = bmp.readAltitude();

  // Read gas sensor data
  gasSensorValue = analogRead(gasSensorPin);

  // Read IR sensor data
  int irSensorValue = digitalRead(irSensorPin);

  // Read LDR sensor data
  int ldrValue = digitalRead(ldrPin);

  // Clear the trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send a 10 microsecond pulse to trigger pin
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the duration of pulse from echo pin
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance in centimeters
  distance = duration * 0.034 / 2;

  // Control LED based on sensor readings
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
  delay(1000);

  // Blink LED with 1 second interval
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    digitalWrite(ledPin, !digitalRead(ledPin)); // Toggle LED state
  }

  // Control servo motor independently
  myservo.write(0);  // tell servo to go to a particular angle
  delay(1000);
  
  myservo.write(90);              
  delay(500); 
  
  myservo.write(135);              
  delay(500);
  
  myservo.write(180);              
  delay(1500);                     

  // Construct the string without labels
  str = String(temperature) + "," +
        String(pressure) + "," +
        String(altitude) + "," +
        String(distance) + "," +
        String(irSensorValue) + "," +
        String(ldrValue) + "," +
        String(gasSensorValue);

  // Send the string to ESP8266
  espSerial.println(str);

  // Print data to serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" Pa");
  Serial.print("Altitude: ");
  Serial.print(altitude);
  Serial.println(" meters");
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  Serial.print("IR Sensor Value: ");
  Serial.println(irSensorValue);
  Serial.print("LDR Sensor Value: ");
  Serial.println(ldrValue);
  Serial.print("Gas Sensor Value: ");
  Serial.println(gasSensorValue);

  delay(2000);
}
