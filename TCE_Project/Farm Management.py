import mysql.connector
import paho.mqtt.client as mqttClient
from threading import Thread
import json

class Mqtt:
    def __init__(self):
        self.json_data = {}
        self.db = mysql.connector.connect(
            host="localhost",
            user="root",
            password="",
            db="iot_firmware"
        )
        mqttclient = mqttClient.Client("2cfc106f-ac35-4dcf-9d6a-bd49ff4a615b")
        mqttclient.on_connect = self.on_connect
        mqttclient.on_message = self.on_message
        mqttclient.username_pw_set(username="", password="")
        mqttstatus = mqttclient.connect("broker.emqx.io", 1883, 60)
        mqttclient.subscribe("Farm_Management", 0)
        mqttclient.loop_forever()

    def upload(self, msg):
        mqtt_msg = str(msg.payload).replace("b'", "").replace("}", "").replace("{", "").replace(" ", "") \
                   .replace("\n", "").replace("[", "").replace("]", "")
        print(mqtt_msg)

        parsed_data = list(map(lambda x: x.strip(), mqtt_msg.split(",")))
        print("Parsed Data:", parsed_data)

        Rain, SoilMoisture, WaterFloat, Temp, Humidity, WaterFlow = parsed_data

        mycursor = self.db.cursor()
        sql = "INSERT INTO data (Rain_drop, Soil_moisture, Water_level, Temperature, Humidity, Water_Flow)" \
              " VALUES (%s, %s, %s, %s, %s, %s)"
        mycursor.execute(sql, (Rain, SoilMoisture, WaterFloat, Temp, Humidity, WaterFlow))
        self.db.commit()

        print("Data Inserted!")

    def on_connect(self, mqttclient, userdata, flags, rc):
        if rc == 0:
            print("connected!")
        else:
            print("Connection failed")

    def on_message(self, mqttclient, userdata, msg):
        Thread(target=self.upload, args=(msg,)).start()

if __name__ == "__main__":
    Mqtt()

