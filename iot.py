import paho.mqtt.client as mqtt
import requests
import json

mqtt_broker = "192.168.0.74"
mqtt_port = 1883
mqtt_topic = "dht"

http_server = "http://192.168.0.74:3000/data"

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")
        client.subscribe(mqtt_topic)
    else:
        print(f"Connection failed with result code {rc}")

def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    print(f"Message on topic {msg.topic}: {payload}")
    
    try:
        # Load the JSON string into a Python dictionary
        data = json.loads(payload)

        # Reformat the data dictionary
        reformatted_data = {
            "humidity": data.get("humidity"),
            "temperature": data.get("temperature"),
            "ip": data.get("ip"),
            "id": data.get("id")
        }

        print("Reformatted data:", reformatted_data)

        response = requests.post(http_server, json=reformatted_data)
        print(f"HTTP Response: {response.status_code}")

        if response.status_code == 200:
            print("Data sent successfully")
    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")
    except Exception as e:
        print(f"Error sending data to the server: {str(e)}")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

try:
    client.connect(mqtt_broker, mqtt_port)
except Exception as e:
    print(f"Error connecting to MQTT broker: {str(e)}")

client.loop_forever()
