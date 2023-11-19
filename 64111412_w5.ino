#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

const char *ssid = "IR_Lab";
const char *password = "ccsadmin";
const char *mqtt_server = "192.168.0.74";
const char *mqtt_topic = "dht";
const int ledPin = D6; 

float temp;
float hum;
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht11(D4, DHT11);




void setup_wifi() {
    delay(10);
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
        if (client.connect("ESP8266Client")) {
            Serial.println("connected");
            client.subscribe("led");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void callback(char *topic, byte *payload, unsigned int length) {
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    if (String(topic) == "led") {
        if (message == "on") {
            digitalWrite(ledPin, HIGH);
        } else if (message == "off") {
            digitalWrite(ledPin, LOW);
        }
    }
}

void setup() {
    pinMode(ledPin, OUTPUT);
    Serial.begin(115200);
    dht11.begin();
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    
}

void loop() {
    if (!client.connected()) {
        reconnect();
        digitalWrite(ledPin, LOW);
    }
    client.loop();

    float humidity = dht11.readHumidity();
    float temperature = dht11.readTemperature();

    if (!isnan(humidity) && !isnan(temperature)) {
        temp = temperature;
        hum = humidity;
        IPAddress localIP = WiFi.localIP();
        StaticJsonDocument<200> jsonDocument;
        jsonDocument["humidity"] = hum;
        jsonDocument["temperature"] = temp;
        jsonDocument["ip"] = localIP.toString();
        String jsonString;
        serializeJson(jsonDocument, jsonString);
        client.publish(mqtt_topic, jsonString.c_str());
        
    }

    delay(5000);
}


