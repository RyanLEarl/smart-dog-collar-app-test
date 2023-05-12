#include "./inc/secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

/**
 * Connects to local WiFi network as identified in secrets.h, and then
 * connects to AWS IoT with the URL defined in secrets.h.
 */
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

/**
 * Receives a JSON document from the Arduino Nano 33 BLE Sense and
 * forwards it to AWS IoT Core.
 */
void publishMessage()
{
  StaticJsonDocument<256> doc;
  /*doc["time"] = millis();
  doc["sensor_a0"] = analogRead(0);
  char jsonBuffer[512];*/

  boolean message_ready = false;
  String message = "";
  while (!message_ready) { // blocking but that's ok
    if (Serial.available()) {
      message = Serial.readString();
      message_ready = true;
    }
  }
  // Attempt to deserialize the JSON-formatted message to verify that it's valid JSON
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  } else {
    // forward the message to AWS IoT Core
    client.publish(AWS_IOT_PUBLISH_TOPIC, message);
  }
}

/**
 * Handles messages sent by AWS IoT
 * This function is not normally used, so it is left unimplemeted.
 */
void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

//  StaticJsonDocument<200> doc;
//  deserializeJson(doc, payload);
//  const char* message = doc["message"];
}

void setup() {
  Serial.begin(9600);
  connectAWS();
}

void loop() {
  publishMessage();
  client.loop();
  delay(1000);
}