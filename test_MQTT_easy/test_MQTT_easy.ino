/* test_MQTT_easy
 * 
 * Test para prueba de la librería PubSubClient.h, menos pesada que EspMQTTClient.h.
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// Parametros WIFI
const char* ssid = "RPi_PA";
const char* pass = "raspberry";
const char* mqtt_server = "192.168.4.1";

WiFiClient espClient;
PubSubClient MQTTclient(espClient);

#define PIN_LED 17



void setup() {
  Serial.begin(115200);

  SetupWifi();
  MQTTclient.setServer(mqtt_server, 1883);
  MQTTclient.setCallback(callback);

  pinMode(PIN_LED, OUTPUT);
}


void SetupWifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");  Serial.println(WiFi.localIP());
}


void MQTT_Reconnect() {
  // Loop until we're reconnected
  while (!MQTTclient.connected()) {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (MQTTclient.connect("ESP8266Client")) {
        Serial.println("connected");
        // Subscribe
        MQTTclient.subscribe("esp32/output");
      } else {  // Wait 5 seconds before retrying
        Serial.print("failed, rc="); Serial.print(MQTTclient.state()); Serial.println(" try again in 5 seconds");
        delay(5000);
      }
  }
}


void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(PIN_LED, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(PIN_LED, LOW);
    }
  }
}



void loop() {
  if (WiFi.status() == WL_CONNECTED) {
      if (!MQTTclient.connected()) MQTT_Reconnect();
      MQTTclient.loop();
  } 
  else {
      Serial.println ( "No conectado a WiFi" ); 
  }
}
