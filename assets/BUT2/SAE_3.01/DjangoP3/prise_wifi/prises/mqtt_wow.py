import paho.mqtt.client as mqtt
import threading

mqtt_broker = '192.168.137.228'
mqtt_port = 1883
mqtt_user = 'pierre'
mqtt_pass = 'ECNP'

led_state = '0'  # Valeur partagée (à synchroniser)

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("led/state")

def on_message(client, userdata, msg):
    global led_state
    payload = msg.payload.decode()
    print(f"Received message on {msg.topic}: {payload}")
    if msg.topic == "led/state":
        led_state = payload

client = mqtt.Client()
client.username_pw_set(mqtt_user, mqtt_pass)
client.on_connect = on_connect
client.on_message = on_message

def mqtt_loop():
    client.connect(mqtt_broker, mqtt_port, 60)
    client.loop_forever()

mqtt_thread = threading.Thread(target=mqtt_loop)
mqtt_thread.daemon = True
mqtt_thread.start()