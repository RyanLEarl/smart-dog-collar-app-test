// #ifndef OUTPUT_HANDLER_H
// #define OUTPUT_HANDLER_H
// #define BLE_SENSE_BOARD
// #undef BLE_SENSE_BOARD

// #include <ArduinoBearSSL.h>
// #include <ArduinoECCX08.h>
// #include <ArduinoMqttClient.h>
// // #include <SPI.h>
// #include <WiFiNINA.h>
// #include "arduino_secrets.h"

// #ifdef BLE_SENSE_BOARD
// #include "tensorflow/lite/micro/micro_error_reporter.h"
// // #include <Arduino_LSM9DS1.h> // Rev 1
// #include <Arduino_BMI270_BMM150.h> // Rev 2
// #else
// #include <Arduino_LSM6DS3.h> // IoT
// #endif

// #define SENSOR_COUNT 6
// #define LABEL_COUNT 7
// #define SEIZURE 4

// /*************/
// /*  Globals  */
// /*************/
// /////// Enter your sensitive data in arduino_secrets.h
// // const char ssid[]        = SECRET_SSID;
// // const char pass[]        = SECRET_PASS;
// // const char broker[]      = SECRET_BROKER;
// // const char* sdc_certificate  = SECRET_CERTIFICATE;

// // WiFiClient    sdc_wifiClient;            // Used for the TCP socket connection
// // BearSSLClient sdc_sslClient(sdc_wifiClient); // Used for SSL/TLS connection, integrates with ECC508
// // MqttClient    sdc_mqttClient(sdc_sslClient);
// // String stringXGyro, stringYGyro, stringZGyro, stringXAccel, stringYAccel, stringZAccel, stringOne, stringTwo, stringThree, stringFour, stringFive, stringSix, stringSeven, stringOutput;

// class OutputHandler{
// /*************/
// /* Variables */
// /*************/
// private:
//     char ssid[]        = SECRET_SSID;
//     char pass[]        = SECRET_PASS;
//     char broker[]      = SECRET_BROKER;
//     char* sdc_certificate  = SECRET_CERTIFICATE;

//     WiFiClient    wifiClient;            // Used for the TCP socket connection
//     BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
//     MqttClient    mqttClient(sslClient);
//     String stringXGyro, stringYGyro, stringZGyro, stringXAccel, stringYAccel, stringZAccel, stringOne, stringTwo, stringThree, stringFour, stringFive, stringSix, stringSeven, stringOutput;

//     const char *labels[LABEL_COUNT] = { "car", "leisure", "play", "run_jog",
//                                         "seizure", "sleep", "walk" };
//     int orientations[3];

//     int gyroscopeOrientation;
//     int sensorSensitivity = 100;

// /***********/
// /* Methods */
// /***********/
// public:
//     void setupOutputHandler();
//     #ifdef BLE_SENSE_BOARD
//     void handleOutput(tflite::ErrorReporter*, int, float*);
//     #else
//     void handleOutput(int, float*);
//     #endif
//     void publishMessage();
//     void connectWiFi();
//     void connectMQTT();
//     void onMessageReceived(int);
//     unsigned long getTime();
// };

// // void setupOutputHandler();
// // void publishMessage();
// // void handleOutput(int, float*);

// // void connectWiFi();
// // void connectMQTT();
// // void onMessageReceived(int);
// // unsigned long getTime();

// #endif