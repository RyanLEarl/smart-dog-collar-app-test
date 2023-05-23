#include "./inc/secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include "WiFi.h"

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "smart-dog-collar"
#define AWS_IOT_SUBSCRIBE_TOPIC "smart-dog-collar"

HardwareSerial SerialPort(2); // UART2

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

StaticJsonDocument<256> in_doc;
StaticJsonDocument<256> out_doc;

/**
 * Connects to local WiFi network as identified in secrets.h, and then
 * connects to AWS IoT with the URL defined in secrets.h.
 */
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  // Connect to WiFi network
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

  Serial.println("Connecting to AWS IOT");

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
  String message = "";
  if (SerialPort.available()) {
    message = SerialPort.readString();
  } else {
	  return;
  }
  
  in_doc.clear();
  out_doc.clear();
  
  // Attempt to deserialize the JSON-formatted message to verify that it's valid JSON
  DeserializationError error = deserializeJson(in_doc, message);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  } else {
    out_doc["type"] = in_doc["type"];
    //out_doc["timestamp"] = millis();
    out_doc["status"] = in_doc["status"];

    // in_doc["type"] = "type";
    // in_doc["status"] = "test2";
    // out_doc["type"] = in_doc["type"];
    // out_doc["status"] = in_doc["status"];
    
    String payload;
    serializeJson(out_doc, payload);
    // Serial.println("Sending:");
    // Serial.println(payload);

    // forward the message to AWS IoT Core
    client.publish(AWS_IOT_PUBLISH_TOPIC, payload.c_str());
    delay(1000);
  }
}

/**
 * Handles messages sent by AWS IoT
 * This function is not currently used, so it is left unimplemeted.
 */
void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

//  StaticJsonDocument<200> doc;
//  deserializeJson(doc, message);
//  const char* message = doc["message"];
}

/**
 * Starts by connecting to Aws through the local WiFi network.
 */
void setup() {
  Serial.begin(9600);
  SerialPort.begin(9600, SERIAL_8N1, 16, 17); // RX = 16, TX = 17
  connectAWS();
}

/**
 * Continuously reads incoming messages from the Arduino on the serial port.
 * Forwards these messages to AWS.
 */
void loop() {
  publishMessage();
  client.loop();
  delay(10);
  //delay(1000);
}