#ifndef OUTPUT_HANDLER_H
#define OUTPUT_HANDLER_H

// #include <ArduinoBearSSL.h>
// #include <ArduinoECCX08.h>
// #include <ArduinoMqttClient.h>
// #include <WiFiNINA.h>
// #include <Arduino_LSM6DS3.h>
#include <Arduino_BMI270_BMM150.h> // Rev 2
// #include "arduino_secrets.h"

#include "tensorflow/lite/micro/micro_error_reporter.h"

#define SENSOR_COUNT 6
#define LABEL_COUNT 7
#define SEIZURE 4

class Output_Handler{
/*************/
/* Variables */
/*************/
private:
    const char *labels[LABEL_COUNT] = { "car", "leisure", "play", "run_jog",
                                        "seizure", "sleep", "walk" };
/***********/
/* Methods */
/***********/
public:
    void handleOutput(tflite::ErrorReporter*, int, float*);
};

#endif