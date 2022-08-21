//convert MQTT byte* payload to float https://gist.github.com/jvalteren/40ab1d6b5cde2ffb0201

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ezTime.h>
#include "arduino_secrets.h"
const char* ssid     = SECRET_SSID;
const char* password = SECRET_PASS;
const char* mqttuser = SECRET_MQTTUSER;
const char* mqttpass = SECRET_MQTTPASS;

const char* mqtt_server = "[mqtt server]";

#define fanPin 5 //D1
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
float DMaskOffP = 0.0;

Timezone GB;

void setup() {

  pinMode(fanPin, OUTPUT);
  analogWriteRange(100);
  analogWriteFreq(10000);
  Serial.begin(115200);
  delay(100); 
  startWifi();
  syncDate();
  client.setServer(mqtt_server, 1884);
  client.setCallback(callback);

}

void loop() {

  delay(5000);
  sendMQTT();
  
}


void startWifi() {
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
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
void reconnect() {
  while (!client.connected()) {    // while not (!) connected....
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqttuser, mqttpass)) {
      Serial.println("connected");
      client.subscribe("[mqtt topic]");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void sendMQTT() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void syncDate() {
  waitForSync();
  Serial.println("UTC: " + UTC.dateTime());
  GB.setLocation("Europe/London");
  Serial.println("London time: " + GB.dateTime());
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    float f = 0.0;
    int x = 0;
    int index = -1;
    for (int i = 0; i < length; i++) {

      if (payload[i] >= 48 && payload[i] <= 57) {

        x = x * 10 + (int)(payload[i] - 48);
      } else if (payload[i] == 46) {

        f += x;
        x = 0;
        index = i;
      }
    }
    if (index != -1) {

      int p = 1;
      for (int i = 0; i < length - index - 1; i++) {
        p = p * 10;
      }
      f += (x / (float)p);
    }
    DMaskOffP = f;
    Serial.println(DMaskOffP);
    Serial.println(DMaskOffP);
//    if (minute() == 0 || minute() == 15 || minute() == 30 || minute() == 45) { //if you want to have periodic update
  if (DMaskOffP > [define a high risk level]) {
        analogWrite(fanPin, [define fan speed here]);
      delay(10000);
      Serial.println("DMaskOffP>[define a high risk level]");
    } else if (DMaskOffP > [define a moderate risk level]) {
      analogWrite(fanPin, [define fan speed here]);
      delay(10000);
      Serial.println("[define a high risk level]>DMaskOffP>[define a moderate risk level]");
    } else
    {
      analogWrite(fanPin, [define fan speed here]);
      delay(10000);
      Serial.println("DMaskOffP<[define a moderate risk level]");
    }
  } 
//}
