#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h> // Include the PubSubClient library

const char* ssid = "Quantanics"; // Your WiFi SSID
const char* password = "Qu@nt@nics18"; // Your WiFi password

#define MOTOR_PIN_ENA D1 // Enable pin for motor driver (PWM pin)
#define MOTOR_PIN_IN1 D2 // Input pin 1 for motor driver
#define MOTOR_PIN_IN2 D3 // Input pin 2 for motor driver

int motorSpeed = 0; // Global variable to store received motor speed
int distance = 0; // Global variable to store received distance
bool forwardFlag = false; // Flag for forward direction
bool reverseFlag = false; // Flag for reverse direction
bool resetFlag = false; // Flag for reset

ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// MQTT parameters
const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char* mqttClientId = "LA_tce/Dashboard";
const char* mqttTopic = "linear_actuator_voltage";

// HTML content as a string variable
const char* htmlContent = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Linear Actuator</title>
    <style>
        .container {
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        .slider {
            position: relative;
            width: 100%;
            max-width: 240px;
            margin: 0 auto;
        }
        .slider__range {
            -webkit-appearance: none;
            appearance: none;
            width: 100%;
            height: 10px;
            background: #ddd;
            outline: none;
            border-radius: 5px;
            box-shadow: 0 0 2px rgba(0, 0, 0, 0.2);
        }
        .slider__range::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 20px;
            height: 20px;
            background: #4caf50;
            border-radius: 50%;
            cursor: pointer;
            box-shadow: 0 0 2px rgba(0, 0, 0, 0.2);
            transition: transform 0.2s ease-out;
        }
        .slider__range:hover::-webkit-slider-thumb {
            transform: translateY(-5px);
        }
        .slider__range_distance {
            -webkit-appearance: none;
            appearance: none;
            width: 100%;
            height: 10px;
            background: #ddd; /* Light Gray */
            outline: none;
            border-radius: 5px;
            box-shadow: 0 0 2px rgba(0, 0, 0, 0.2);
        }
        .slider__range_distance::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 20px;
            height: 20px;
            background: #333; /* Dark Gray */
            border-radius: 50%;
            cursor: pointer;
            box-shadow: 0 0 2px rgba(0, 0, 0, 0.2);
            transition: transform 0.2s ease-out;
        }
        .slider__range_distance:hover::-webkit-slider-thumb {
            transform: translateY(-5px);
        }
        .button {
            display: inline-block;
            padding: 10px 20px;
            margin-top: 20px;
            font-size: 16px;
            text-align: center;
            cursor: pointer;
            background-color: #4caf50;
            color: white;
            border: none;
            border-radius: 5px;
            transition: background-color 0.3s;
        }
        .button:hover {
            background-color: #45a049;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="slider">
            <input type="range" min="0" max="255" value="0" class="slider__range" id="slider">
        </div>
        <button class="button" id="forwardButton">Forward</button>
        <button class="button" id="reverseButton">Reverse</button>
        <div class="slider" style="margin-top: 20px;">
            <input type="range" min="0" max="100" value="0" class="slider__range_distance" id="distanceSlider">
        </div>
        <span id="distanceValue">Distance: 0</span> <!-- Span element to display distance value -->
        <button class="button" id="resetButton">Reset</button>
    </div>
<script>
    const slider = document.getElementById('slider');
    const distanceSlider = document.getElementById('distanceSlider');
    const distanceValueSpan = document.getElementById('distanceValue'); // Reference to the span element
    const forwardButton = document.getElementById('forwardButton');
    const reverseButton = document.getElementById('reverseButton');
    const resetButton = document.getElementById('resetButton');

    // Event listeners for slider inputs
    slider.addEventListener('input', () => {
        // Update distance value span text content
        distanceValueSpan.textContent = 'Distance: ' + distanceSlider.value;
        console.log('Slider value changed:', slider.value);
        console.log('Distance slider value:', distanceSlider.value); // Print distance value to console
        if (forwardButton.classList.contains('active')) {
            sendSliderValue(slider.value, distanceSlider.value, true, false);
        } else if (reverseButton.classList.contains('active')) {
            sendSliderValue(slider.value, distanceSlider.value, false, true);
        }
    });

      distanceSlider.addEventListener('input', () => {
    // Update distance value span text content
    distanceValueSpan.textContent = 'Distance: ' + distanceSlider.value;
    console.log('Distance slider value changed:', distanceSlider.value); // Print distance value to console
    
    // Check if both forward and reverse flags are off
    if (!forwardButton.classList.contains('active') && !reverseButton.classList.contains('active')) {
        // Send the slider values to the server only when both flags are off
        sendSliderValue(slider.value, distanceSlider.value, false, false);
    }
    });

    forwardButton.addEventListener('click', () => {
        console.log('Forward button clicked');
        forwardButton.classList.add('active');
        reverseButton.classList.remove('active');
        slider.value = 0;
        distanceSlider.value = 0;
        sendSliderValue(slider.value, distanceSlider.value, true, false);
    });

    reverseButton.addEventListener('click', () => {
        console.log('Reverse button clicked');
        reverseButton.classList.add('active');
        forwardButton.classList.remove('active');
        slider.value = 0;
        distanceSlider.value = 0;
        sendSliderValue(slider.value, distanceSlider.value, false, true);
    });

    resetButton.addEventListener('click', () => {
        console.log('Reset button clicked');
        slider.value = 0;
        distanceSlider.value = 0;
        sendSliderValue(0, 0, false, false);
        forwardButton.classList.remove('active'); // Remove active class from forward button
        reverseButton.classList.remove('active'); // Remove active class from reverse button
        forwardFlag = false; // Set forward flag to false
        reverseFlag = false; // Set reverse flag to false
        // Send request to reset distance value
        fetch('/set?reset=true&distance=0')
            .then(response => {
                if (!response.ok) {
                    console.error('Error resetting distance value');
                }
            })
            .catch(error => {
                console.error('Error resetting distance value:', error);
            });
    });

    function sendSliderValue(value, distanceValue, forwardFlag, reverseFlag) {
    fetch('/set', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: 'value=' + value + '&distance=' + distanceValue + '&forwardFlag=' + forwardFlag + '&reverseFlag=' + reverseFlag,
    })
    .then(response => {
        if (!response.ok) {
            console.error('Error sending slider value to server');
        }
    })
    .catch(error => {
        console.error('Error sending slider value to server:', error);
    });
}
</script>

</body>
</html>
)";

void handleRoot() {
    server.send(200, "text/html", htmlContent);
}

void handleInt() {
    if (server.hasArg("value") && server.hasArg("forwardFlag") && server.hasArg("reverseFlag")&& server.hasArg("distance")) {
        String value = server.arg("value");
        motorSpeed = value.toInt(); // Store received motor speed in the global variable

        String forwardFlagStr = server.arg("forwardFlag");
        forwardFlag = forwardFlagStr.equals("true");

        String reverseFlagStr = server.arg("reverseFlag");
        reverseFlag = reverseFlagStr.equals("true");

        String distanceValue = server.arg("distance");
                distance = distanceValue.toInt(); // Store received distance in the global variable

        Serial.println("Motor speed value set to: " + String(motorSpeed));
        server.send(200, "text/plain", "Motor speed value set to: " + String(motorSpeed));
        
        // Control the motor based on the received value
        if (forwardFlag) {
            // Rotate the motor forward
            digitalWrite(MOTOR_PIN_IN1, HIGH);
            digitalWrite(MOTOR_PIN_IN2, LOW);
            analogWrite(MOTOR_PIN_ENA, motorSpeed); // Set the motor speed
            Serial.println("Motor Forward");
            Serial.print("Motor speed: ");
            Serial.println(motorSpeed);
        } else if (reverseFlag) {
            // Rotate the motor in reverse
            digitalWrite(MOTOR_PIN_IN1, LOW);
            digitalWrite(MOTOR_PIN_IN2, HIGH);
            analogWrite(MOTOR_PIN_ENA, motorSpeed); // Set the motor speed
            Serial.println("Motor Reverse");
            Serial.print("Motor speed: ");
            Serial.println(motorSpeed);
        } 
         else if (!forwardFlag &&!reverseFlag && distance>0){
           Serial.println(distance);
            // Rotate the motor forward
            digitalWrite(MOTOR_PIN_IN1, HIGH);
            digitalWrite(MOTOR_PIN_IN2, LOW);
            motorSpeed =255;
            analogWrite(MOTOR_PIN_ENA, motorSpeed); // Set the motor speed
            int time = distance/0.5 *1000;
            delay(time);
            digitalWrite(MOTOR_PIN_IN1, LOW);
            digitalWrite(MOTOR_PIN_IN2, LOW);
            Serial.println("Motor stopped");
            
          }
         
        // Publish voltage value to MQTT topic
        float voltage = map(motorSpeed, 0, 255, 0, 2400) / 100.0;
        Serial.println("Publishing voltage value: " + String(voltage));
        if (mqttClient.connected()) {
            mqttClient.publish(mqttTopic, String(voltage).c_str());
        }
    }
}


void setup() {
    Serial.begin(115200);
    delay(1000); // Delay for serial connection to initialize
    Serial.println("Motor Control");
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi...");

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    Serial.println("WiFi connected");
    Serial.println("IP Address: ");
    Serial.println(WiFi.localIP());

    pinMode(MOTOR_PIN_ENA, OUTPUT);
    pinMode(MOTOR_PIN_IN1, OUTPUT);
    pinMode(MOTOR_PIN_IN2, OUTPUT);

    server.on("/", handleRoot);
    server.on("/set", handleInt);

    server.begin();
    Serial.println("HTTP server started");

    mqttClient.setServer(mqttServer, mqttPort); // Set MQTT server and port
    mqttClient.connect(mqttClientId); // Connect to MQTT broker
}

   void loop() {
    server.handleClient();
    if (!mqttClient.connected()) {
        mqttClient.connect(mqttClientId);
    }
    mqttClient.loop();
}
