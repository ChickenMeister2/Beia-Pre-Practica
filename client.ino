#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DHTPIN 2
#define DHTTYPE DHT11

// WiFi credentials
const char* ssid = "hidden";
const char* password = "hidden";

// MQTT Server settings
const char* mqtt_server = "mqtt.beia-telemetrie.ro";
const char* mqtt_topic = "/training/device/cosmin-dima/data";

WiFiClient espClient;
PubSubClient client(espClient);
DHT_Unified dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  dht.begin();
}

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client")) {
      Serial.println("Connected to MQTT Broker!");
    } else {
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  sensors_event_t event;

  // Create a JSON object for storing temperature and humidity
  StaticJsonDocument<200> doc;

  dht.temperature().getEvent(&event);
  if (!isnan(event.temperature)) {
    doc["t"] = event.temperature;
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
  }

  dht.humidity().getEvent(&event);
  if (!isnan(event.relative_humidity)) {
    doc["h"] = event.relative_humidity;
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
  }

  // Generate the JSON string
  char jsonBuffer[200];
  serializeJson(doc, jsonBuffer);
  client.publish(mqtt_topic, jsonBuffer);

  delay(2000); // Delay between measurements
}
