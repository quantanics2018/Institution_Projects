import minimalmodbus
import time
import paho.mqtt.client as mqtt

# Configuration parameters
baudrate = 19200
bytesize = 8
parity = 'N'
stopbits = 1
timeout = 1
port = '/dev/ttyUSB0'
slave_address = 1  # Slave address

# MQTT Broker configuration
mqtt_broker = 'broker.emqx.io'
mqtt_port = 1883
mqtt_topic = 'data'

# Create a Modbus instrument instance
instrument = minimalmodbus.Instrument(port, slave_address)

# Setup serial connection parameters
instrument.serial.baudrate = baudrate
instrument.serial.bytesize = bytesize
instrument.serial.parity = parity
instrument.serial.stopbits = stopbits
instrument.serial.timeout = timeout

# Read Modbus register
register_address = 0  # Register address to read

# MQTT client setup
client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

def on_disconnect(client, userdata, rc):
    if rc != 0:
        print("Unexpected disconnection.")

client.on_connect = on_connect
client.on_disconnect = on_disconnect

# Connect to MQTT broker
client.connect(mqtt_broker, mqtt_port, 60)
client.loop_start()

try:
    while True:
        # Read the single register
        register_value = instrument.read_register(register_address, functioncode=4)
        register_value = float(register_value) / 1000
        
        # Print the register value for debugging
        print("Register value:", register_value)
        
        # Send only the value to MQTT broker
        client.publish(mqtt_topic, str(register_value))
        print("Value:", register_value)
        
        # If register value is above 16, set relay 1 to high
        if register_value > 16:
            instrument.write_bit(register_address, 1, functioncode=5)
            print("Relay 1 set to high.")
        else:
            # Ensure relay 1 is set to low if condition is not met
            instrument.write_bit(register_address, 0, functioncode=5)
            print("Relay 1 set to low.")
        
        # Wait for a while before reading again
        time.sleep(1)

except Exception as e:
    print("Error:", e)
