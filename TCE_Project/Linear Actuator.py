import mysql.connector
import paho.mqtt.client as mqtt

# Define callback functions
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # Subscribe to the topic
    client.subscribe("linear_actuator_voltage")

# Define callback functions
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    # Convert the payload to float
    try:
        voltage = float(msg.payload)
    except ValueError:
        print("Invalid data received:", msg.payload)
        return
    # Insert received data into the database
    insert_into_database(voltage)

    # Function to insert data into the database
def insert_into_database(data):
    mydb = mysql.connector.connect(
        host="localhost",
        user="root",
        password="",
        database="iot_firmware"
    )
    mycursor = mydb.cursor()
    sql = "INSERT INTO data(Voltage) VALUES(%s)"
    val = (data,)
    mycursor.execute(sql, val)
    mydb.commit()

    mycursor.close()
    mydb.close()
    print("Data inserted into database:", data)

# Create a MQTT client instance
client = mqtt.Client()

# Assign callback functions
client.on_connect = on_connect
client.on_message = on_message

# Connect to MQTT broker
client.connect("broker.emqx.io", 1883, 60)

# Start the loop
client.loop_forever()
