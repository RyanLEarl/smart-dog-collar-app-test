#include "../inc/sensors.h"

#define SMART_DOG_COLLAR_DEBUG
// #undef SMART_DOG_COLLAR_DEBUG

// Sets up the IMU and finds the sample rate
bool Sensors::setupIMU(tflite::ErrorReporter *error_reporter) 
{
    // Try to start up the IMU
    if (!IMU.begin()) 
    {
        error_reporter->Report("Failed to initialize IMU");
        return 0;
    }

    // Find the sample rate of our sensors
    acceleration_sample_rate = IMU.accelerationSampleRate();
    gyroscope_sample_rate = IMU.gyroscopeSampleRate();

    // Set how often to accept samples
    acceleration_sample_every_n = static_cast<int>(roundf(acceleration_sample_rate / TARGET_HZ));
    gyroscope_sample_every_n = static_cast<int>(roundf(gyroscope_sample_rate / TARGET_HZ));

    #ifdef SMART_DOG_COLLAR_DEBUG
    float rate_frac;
    float rate_int;
    rate_frac = modf(acceleration_sample_rate, &rate_int);
    TF_LITE_REPORT_ERROR(error_reporter, "Acceleration sample rate %d.%d Hz",
                        static_cast<int32_t>(rate_int),
                        static_cast<int32_t>(rate_frac * 100));
    rate_frac = modf(gyroscope_sample_rate, &rate_int);
    TF_LITE_REPORT_ERROR(error_reporter, "Gyroscope sample rate %d.%d Hz",
                        static_cast<int32_t>(rate_int),
                        static_cast<int32_t>(rate_frac * 100));
    #endif

    return 1;
}

// Reads in samples from the accelerometer and gyroscope if data is available
// and stores it in a FIFO buffer
// NOTE: readGyroscope and readAccelerometer don't return anything in the provided
// library, so if they get updated add a ! before the if check
// Approximately 11 per second?
void Sensors::readAccelerometerAndGyroscope(tflite::ErrorReporter *error_reporter, float *input) 
{ 
    int error_status = 0;

    // Try to get data from the gyroscope
    if(IMU.gyroscopeAvailable()){
        // Read data if it is time to read gyroscope
        if(true)
        // if(gyroscope_skip_counter >= gyroscope_sample_every_n)
        {
            const int gyroscope_index = (gyroscope_data_index % GYROSCOPE_DATA_LENGTH);
            gyroscope_data_index += 3;
            float* current_gyroscope_data = &gyroscope_data[gyroscope_index];

            // Read gyroscope sample
            if(IMU.readGyroscope(current_gyroscope_data[0], current_gyroscope_data[1], current_gyroscope_data[2]))
            // if(!IMU.readGyroscope(current_gyroscope_data[0], current_gyroscope_data[1], current_gyroscope_data[2]))
            {
                error_reporter->Report("Failed to read gyroscope data");
                error_status = 1;
            }
            else
            {
                // Give the normalized data to the ml model
                for(int i = 0; i < GYROSCOPE_COUNT; i++)
                {
                    current_gyroscope_data[i] /= NORMALIZATION;
                    input[i] = current_gyroscope_data[i];
                }
                gyroscope_skip_counter = 1;

                // Check what is being read from the IMU
                #ifdef SMART_DOG_COLLAR_DEBUG
                error_reporter->Report("Data from gyroscope");
                Serial.print(current_gyroscope_data[0]);
                Serial.print('\t');
                Serial.print(current_gyroscope_data[1]);
                Serial.print('\t');
                Serial.println(current_gyroscope_data[2]);
                #endif
            }
        }

        gyroscope_skip_counter++;
    }

    // Try to read data from the accelerometer
    if(IMU.accelerationAvailable())
    {
        // Read data if it is time to read accelerometer
        if(true)
        // if(acceleration_skip_counter >= acceleration_sample_every_n)
        {
            const int acceleration_index = (acceleration_data_index % ACCELERATION_DATA_LENGTH);
            acceleration_data_index += 3;
            float* current_acceleration_data = &acceleration_data[acceleration_index];

            // Read acceleration sample
            if (IMU.readAcceleration(current_acceleration_data[0],
                                      current_acceleration_data[1],
                                      current_acceleration_data[2])) 
            {
                error_reporter->Report("Failed to read acceleration data");
                error_status = 1;
            }
            else
            {
                // Normalize accelerometer data
                current_acceleration_data[0] += ACCELERATION_X_NORM;
                current_acceleration_data[1] += ACCELERATION_Y_NORM;
                current_acceleration_data[2] += ACCELERATION_Z_NORM;

                // Give normalized data to ml model 
                for(int i = 0; i < ACCELERATION_COUNT; i++)
                {
                    current_acceleration_data[i] /= NORMALIZATION;
                    input[i + GYROSCOPE_COUNT] = current_acceleration_data[i];
                }
                acceleration_skip_counter = 1;

                // Check what is being read from the IMU
                #ifdef SMART_DOG_COLLAR_DEBUG
                error_reporter->Report("Data from accelerometer");
                Serial.print(current_acceleration_data[0]);
                Serial.print('\t');
                Serial.print(current_acceleration_data[1]);
                Serial.print('\t');
                Serial.println(current_acceleration_data[2]);
                #endif
            }
        }

        acceleration_skip_counter++;
    }
}
