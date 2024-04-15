import mysql.connector
import paho.mqtt.client as mqttClient
from threading import Thread


class Mqtt:
   def __init__(self):
       self.json_data = {}
       self.setup_mqtt_client()
  


   def setup_mqtt_client(self):
       mqttclient = mqttClient.Client("47d4f601-5d9c-4f80-8a94-6ca455c253f0")
       mqttclient.on_connect = self.on_connect
       mqttclient.on_message = self.on_message
       mqttclient.username_pw_set(username="", password="")
       mqttstatus = mqttclient.connect("broker.emqx.io", 1883, 60)
       mqttclient.subscribe("OTA_update", 0)
       mqttclient.loop_forever()


   def upload(self, msg):
       mqtt_msg = str(msg.payload).replace("b'", "").replace("'", "").replace(" ", "").replace("\\n", "").replace("\n", '')
       print(msg.payload)
       print(mqtt_msg)
       mydb = mysql.connector.connect(
          host="localhost",
          user="root",
          password="",
          database="iot_firmware"
       )
       mycursor = mydb.cursor()
       sql = "INSERT INTO data(distance) VALUES(%s)"
       val = (mqtt_msg,)
       mycursor.execute(sql,val)
       mydb.commit()

       mycursor.close()
       mydb.close()
       print("output message")
       print(mqtt_msg)
  
   
     


   def on_connect(self, mqttclient, userdata, flags, rc):
       if rc == 0:
           print("connected!")
       else:
           print("Connection failed")


   def on_message(self, mqttclient, userdata, msg):
       Thread(target=self.upload, args=(msg,)).start()


if __name__ == '__main__':
   Mqtt()


