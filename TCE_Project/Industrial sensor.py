import mysql.connector
import paho.mqtt.client as mqttClient
from threading import Thread

class Mqtt:
    def __init__(self):
        self.db = mysql.connector.connect(
            host="localhost",
            user="root",
            password="",
            database="iot_firmware"
        )
        self.mqttclient = mqttClient.Client("2cfc106f-ac35-4dcf-9d6a-bd49ff4a615b")
        self.mqttclient.on_connect = self.on_connect
        self.mqttclient.on_message = self.on_message
        self.mqttclient.username_pw_set(username="", password="")
        mqttstatus = self.mqttclient.connect("broker.emqx.io", 1883, 60)
        self.mqttclient.subscribe("Industrial_IOT", 0)  # Change the topic here
        self.mqttclient.loop_forever()

    def upload(self, msg):
        try:
            mqtt_msg = msg.payload.decode("utf-8")
            print(mqtt_msg)
            
            print("Received message:", type(mqtt_msg))

            data_dict = {}

            for line in mqtt_msg.strip().split('\n'):
                key, value = line.split(': ')
                if key.endswith(' *C'):
                    key = key[:-3]  # remove the unit
                    value = float(value)  # convert temperature to float
                elif key.endswith(' Pa'):
                    key = key[:-3]  # remove the unit
                    value = float(value)  # convert pressure to float
                elif key.endswith(' meters'):
                    key = key[:-7]  # remove the unit
                    value = float(value)  # convert altitude to float
                elif key.endswith(' cm'):
                    key = key[:-3]  # remove the unit
                    value = float(value)  # convert distance to float
                elif key == 'IR Sensor Value' or key == 'LDR Sensor Value':
                    value = int(value)  # convert sensor values to integer
                elif key == 'Gas Sensor Value':
                    value = float(value)  # convert gas sensor value to float
                elif key == 'Servo Angle':
                    value = int(value)  # convert servo angle to integer
                data_dict[key] = value


            
            cleaned_data = {}

            for key, value in data_dict.items():
                # Removing unwanted characters and whitespace
                if(type(value)==int):
                    cleaned_data[key] = value
                elif(type(value)==float):
                    cleaned_data[key] = value
                else:
                    value = value.replace('*C', '').replace('Pa', '').replace('\r', '').replace('meters','').replace('cm','').strip()
                    cleaned_data[key] = value

            print("final output is")
            print(cleaned_data)
            cursor = self.db.cursor()
            sql = "INSERT INTO data (IR, LDR, Gas, Temperature, Pressure, Altitude, Distance) " \
                  "VALUES (%s, %s, %s, %s, %s, %s, %s)"
            val = (cleaned_data['IR Sensor Value'],cleaned_data['LDR Sensor Value'],cleaned_data['Gas Sensor Value'],cleaned_data['emperature'],cleaned_data['Pressure'],cleaned_data['Altitude'],cleaned_data['Distance'])
            cursor.execute(sql, val)
            self.db.commit()

           
            print("Data Inserted!")
        except Exception as e:
            print("Error:", e)

    def on_connect(self, mqttclient, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT broker!")
        else:
            print("Connection to MQTT broker failed")

    def on_message(self, mqttclient, userdata, msg):
        Thread(target=self.upload, args=(msg,)).start()

if __name__ == "__main__":
    Mqtt()
