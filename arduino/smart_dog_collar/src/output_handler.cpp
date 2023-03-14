// #define SMART_DOG_COLLAR_DEBUG
// // #undef SMART_DOG_COLLAR_DEBUG
// #define SMART_DOG_COLLAR_AWS_DEBUG
// // #undef SMART_DOG_COLLAR_AWS_DEBUG
// #define BLE_SENSE_BOARD
// #undef BLE_SENSE_BOARD

// #include "../inc/output_handler.h"

// // void OutputHandler::setupOutputHandler()
// void OutputHandler::setupOutputHandler()
// {
//     while (!Serial);
//     stringXGyro = String("{\"body\":{\"xRotationalAxis\":\"");
//     stringYGyro = String("\", \"yRotationalAxis\":\""); 
//     stringZGyro = String("\", \"zRotationalAxis\":\""); 
//     stringXAccel = String("{\"body\":{\"xAcceleration\":\"");
//     stringYAccel = String("\", \"yAcceleration\":\""); 
//     stringZAccel = String("\", \"zAcceleration\":\""); 
//     stringOne = String();
//     stringTwo = String();
//     stringThree = String();
//     stringFour = String();
//     stringFive = String();
//     stringSix = String();
//     stringSeven = String("\"}}");
//     stringOutput = String();
    
//     // Check is ECCX08 is ready
//     if (!ECCX08.begin()) 
//     {
//         Serial.println("No ECCX08 present!");
//         while (1);
//     }

//     // Set a callback to get the current time
//     // used to validate the servers certificate
//     ArduinoBearSSL.onGetTime(getTime);

//     // Set the ECCX08 slot to use for the private key
//     // and the accompanying public certificate for it
//     sslClient.setEccSlot(0, certificate);

//     // Optional, set the client id used for MQTT,
//     // each device that is connected to the broker
//     // must have a unique client id. The MQTTClient will generate
//     // a client id for you based on the millis() value if not set
//     //
//     mqttClient.setId("iotconsole-8bfa76b7-02fb-4391-b5bb-46cc4a524e23");

//     // Set the message callback, this function is
//     // called when the MQTTClient receives a message
//     mqttClient.onMessage(onMessageReceived);
// }

// #ifdef BLE_SENSE_BOARD
// void OutputHandler::handleOutput(tflite::ErrorReporter* error_reporter, int activity, float *sensor_data)
// { 
//     // Connect to wifi if not yet connected
//     if (WiFi.status() != WL_CONNECTED) 
//     {
//         connectWiFi();
//     }

//     // Make sure MQTT is connected to
//     if (!mqttClient.connected()) 
//     {
//         // MQTT client is disconnected, connect
//         connectMQTT();
//     }

//     // poll for new MQTT messages and send keep alive
//     mqttClient.poll();

//     // Handle seizure
//     if(activity == SEIZURE)
//     {
//         // Send push notification
//     }

//     #ifdef SMART_DOG_COLLAR_AWS_DEBUG
//     float gyroX = sensor_data[0];
//     if(false)
//     // if (gyroX > gyroscopeOrientation + sensorSensitivity || gyroX < gyroscopeOrientation - sensorSensitivity) 
//     {
//         gyroscopeOrientation = sensor_data[0];
//         stringOne = stringXGyro + sensor_data[0];
//         stringTwo = stringYGyro + sensor_data[1];
//         stringThree = stringZGyro + sensor_data[2];
//         stringFour = stringXAccel + sensor_data[3];
//         stringFive = stringYAccel + sensor_data[4];
//         stringSix = stringZAccel + sensor_data[5];
//         stringOutput = stringOne + stringTwo + stringThree + stringFour + stringFive + stringSix + stringSeven;
//         Serial.println(gyroscopeOrientation);
//         publishMessage();
//     }
//     #endif

//     #ifdef SMART_DOG_COLLAR_DEBUG
//     // Check what the sensors were
//     for(int i = 0; i < SENSOR_COUNT; i++)
//     {
//         Serial.println(sensor_data[i]);
//     }

//     // Check what was the result of the model
//     error_reporter->Report(labels[activity]);
//     #endif
// }
// #else
// void OutputHandler::handleOutput(int activity, float *sensor_data)
// // void OutputHandler::handleOutput(int activity, float *sensor_data)
// { 
//     // Connect to wifi if not yet connected
//     if (WiFi.status() != WL_CONNECTED) 
//     {
//         connectWiFi();
//     }

//     // Make sure MQTT is connected to
//     if (!mqttClient.connected()) 
//     {
//         // MQTT client is disconnected, connect
//         connectMQTT();
//     }

//     // poll for new MQTT messages and send keep alive
//     mqttClient.poll();

//     // Handle seizure
//     if(activity == SEIZURE)
//     {
//         // Send push notification
//     }

//     #ifdef SMART_DOG_COLLAR_AWS_DEBUG
//     float gyroX = sensor_data[0];
//     if(false)
//     // if (gyroX > gyroscopeOrientation + sensorSensitivity || gyroX < gyroscopeOrientation - sensorSensitivity) 
//     {
//         gyroscopeOrientation = sensor_data[0];
//         stringOne = stringXGyro + sensor_data[0];
//         stringTwo = stringYGyro + sensor_data[1];
//         stringThree = stringZGyro + sensor_data[2];
//         stringFour = stringXAccel + sensor_data[3];
//         stringFive = stringYAccel + sensor_data[4];
//         stringSix = stringZAccel + sensor_data[5];
//         stringOutput = stringOne + stringTwo + stringThree + stringFour + stringFive + stringSix + stringSeven;
//         Serial.println(gyroscopeOrientation);
//         publishMessage();
//     }
//     #endif

//     #ifdef SMART_DOG_COLLAR_DEBUG
//     // Check what the sensors were
//     for(int i = 0; i < SENSOR_COUNT; i++)
//     {
//         Serial.println(sensor_data[i]);
//     }

//     // Check what was the result of the model
//     Serial.println(labels[activity]);
//     #endif
// }
// #endif

// void OutputHandler::publishMessage() 
// {
//     // char json4[] = 
//     // char json3[] = std::string("\", \"z-rotational-axis\":\"") + z + "\"}}";
//     // char json2[] = std::string("\", \"y-rotational-axis\":\"") + y + 
//     // char json[] = std::string("{\"body\":{\"x-rotational-axis\":\"") + gyroscopeOrientation + "\", \"y-rotational-axis\":\"" + y + "\", \"z-rotational-axis\":\"" + z + "\"}}";
//     // Serial.println(json);
//     // Serial.println("Publishing message");

//     // send message, the Print interface can be used to set the message contents
//     mqttClient.beginMessage("smart-dog-collar");
//     // mqttClient.print("I have been moved: ");
//     mqttClient.print(stringOutput);
//     mqttClient.endMessage();
// }

// void OutputHandler::connectWiFi() 
// {
//     Serial.print("Attempting to connect to SSID: ");
//     Serial.print(ssid);
//     Serial.print(" ");

//     while (WiFi.begin(ssid, pass) != WL_CONNECTED) 
//     {
//         // failed, retry
//         Serial.print(".");
//         delay(5000);
//     }
//     Serial.println();

//     Serial.println("You're connected to the network");
//     Serial.println();
// }

// void OutputHandler::connectMQTT() 
// {
//     Serial.print("Attempting to MQTT broker: ");
//     Serial.print(broker);
//     Serial.println(" ");

//     while (!mqttClient.connect(broker, 8883)) 
//     {
//         // failed, retry
//         Serial.print(".");
//         delay(5000);
//     }
//     Serial.println();

//     Serial.println("You're connected to the MQTT broker");
//     Serial.println();

//     // subscribe to a topic
//     mqttClient.subscribe("$aws/things/Ryan-smart-dog-collar-nano-33-iot/shadow/update");
// }

// void OutputHandler::onMessageReceived(int messageSize) 
// {
//     // we received a message, print out the topic and contents
//     Serial.print("Received a message with topic '");
//     // Serial.print(mqttClient.messageTopic());
//     Serial.print("', length ");
//     Serial.print(messageSize);
//     Serial.println(" bytes:");

//     // use the Stream interface to print the contents
//     while (mqttClient.available()) 
//     {
//         Serial.print((char)mqttClient.read());
//     }
//     Serial.println();

//     Serial.println();
// }

// unsigned long OutputHandler::getTime() 
// {
//     // get the current time from the WiFi module  
//     return WiFi.getTime();
// }