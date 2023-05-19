#ifndef SENSORS_H
#define SENSORS_H

#include <TensorFlowLite.h>
#include "tensorflow/lite/micro/micro_error_reporter.h"
// #include <Arduino_LSM9DS1.h> // Rev 1
#include <Arduino_BMI270_BMM150.h> // Rev 2

#define DATA_LENGTH 80
#define ACCELERATION_COUNT 3
#define ACCELERATION_DATA_LENGTH (DATA_LENGTH * 3)
#define GYROSCOPE_COUNT 3
#define GYROSCOPE_DATA_LENGTH (DATA_LENGTH * 3)
#define INPUT_COUNT 6
#define TARGET_HZ 16
#define A_NORMALIZATION 0.40
#define G_NORMALIZATION 100

class Sensors{
/*************/
/* Variables */
/*************/
private:
    // A buffer holding the last data_length sets of 3-channel values from the accelerometer.
    // const int acceleration_data_length = DATA_LENGTH * 3;
    float acceleration_data[ACCELERATION_DATA_LENGTH] = {};
    int acceleration_data_index = 0;  // The next free entry in the data array.
    float acceleration_sample_rate = 0.0f;

    // A buffer holding the last data_length sets of 3-channel values from the gyroscope.
    // const int gyroscope_data_length = data_length * 3;
    float gyroscope_data[GYROSCOPE_DATA_LENGTH] = {};
    int gyroscope_data_index = 0;  // The next free entry in the data array.
    float gyroscope_sample_rate = 0.0f;

    // Variables for sample speed
    int acceleration_sample_every_n;
    int acceleration_skip_counter = 1;
    int gyroscope_sample_every_n;
    int gyroscope_skip_counter = 1;

    int sample_every_n;
    int skip_counter = 1;

/***********/
/* Methods */
/***********/
public:
    bool setupIMU(tflite::ErrorReporter*);
    bool readAccelerometerAndGyroscope(tflite::ErrorReporter*, float*);
};
#endif