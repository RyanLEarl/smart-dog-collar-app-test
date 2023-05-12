#define SMART_DOG_COLLAR_DEBUG
#undef SMART_DOG_COLLAR_DEBUG
#define BLE_SENSE_BOARD
// #undef BLE_SENSE_BOARD

#include "../inc/sensors.h"

#ifdef BLE_SENSE_BOARD
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
    sample_every_n = max(acceleration_sample_every_n, gyroscope_sample_every_n);

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
bool Sensors::readAccelerometerAndGyroscope(tflite::ErrorReporter *error_reporter, float *input)
{ 
    // Try to get data from the gyroscope
    if(IMU.gyroscopeAvailable())
    {
        const int gyroscope_index = (gyroscope_data_index % GYROSCOPE_DATA_LENGTH);
        gyroscope_data_index += 3;
        float* current_gyroscope_data = &gyroscope_data[gyroscope_index];

        // Read gyroscope sample
        if(IMU.readGyroscope(current_gyroscope_data[0], current_gyroscope_data[1], current_gyroscope_data[2]))
        {
            // Give the normalized data to the ml model
            for(int i = 0; i < GYROSCOPE_COUNT; i++)
            {
                current_gyroscope_data[i] /= G_NORMALIZATION;
                input[i] = current_gyroscope_data[i];
            }

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
        else
        {
            error_reporter->Report("Failed to read gyroscope data");
            return false;
        }
    }
    else
    {
        error_reporter->Report("No gyroscope data found");
        return false;
    }

    // Try to read data from the accelerometer
    if(IMU.accelerationAvailable())
    {
        const int acceleration_index = (acceleration_data_index % ACCELERATION_DATA_LENGTH);
        acceleration_data_index += 3;
        float* current_acceleration_data = &acceleration_data[acceleration_index];

        // Read acceleration sample
        if (IMU.readAcceleration(current_acceleration_data[0], current_acceleration_data[1], current_acceleration_data[2]))
        {
            // Give normalized data to ml model 
            for(int i = 0; i < ACCELERATION_COUNT; i++)
            {
                current_acceleration_data[i] /= A_NORMALIZATION;
                input[i + GYROSCOPE_COUNT] = current_acceleration_data[i];
            }

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
        else
        {
            error_reporter->Report("Failed to read acceleration data");
            return false;
        }
    }
    else
    {
        error_reporter->Report("No accelerometer data found");
        return false;
    }

    skip_counter = 1;
    return true;
}
#else
// Sets up the IMU and finds the sample rate
bool Sensors::setupIMU() 
{
    // Try to start up the IMU
    if (!IMU.begin()) 
    {
        Serial.println("Failed to initialize IMU");
        return 0;
    }

    // Find the sample rate of our sensors
    acceleration_sample_rate = IMU.accelerationSampleRate();
    gyroscope_sample_rate = IMU.gyroscopeSampleRate();

    // Set how often to accept samples
    acceleration_sample_every_n = static_cast<int>(roundf(acceleration_sample_rate / TARGET_HZ));
    gyroscope_sample_every_n = static_cast<int>(roundf(gyroscope_sample_rate / TARGET_HZ));
    sample_every_n = max(acceleration_sample_every_n, gyroscope_sample_every_n);

    return 1;
}

// Reads in samples from the accelerometer and gyroscope if data is available
// and stores it in a FIFO buffer
// NOTE: readGyroscope and readAccelerometer don't return anything in the provided
// library, so if they get updated add a ! before the if check
// We want it to read pproximately 11 per second
bool Sensors::readAccelerometerAndGyroscope(float *input)
{
    // Try to get data from the gyroscope
    if(IMU.gyroscopeAvailable())
    {
        const int gyroscope_index = (gyroscope_data_index % GYROSCOPE_DATA_LENGTH);
        gyroscope_data_index += 3;
        float* current_gyroscope_data = &gyroscope_data[gyroscope_index];

        // Read gyroscope sample
        if(IMU.readGyroscope(current_gyroscope_data[0], current_gyroscope_data[1], current_gyroscope_data[2]))
        {
            // Give the normalized data to the ml model
            for(int i = 0; i < GYROSCOPE_COUNT; i++)
            {
                current_gyroscope_data[i] /= G_NORMALIZATION;
                input[i] = current_gyroscope_data[i];
            }

            // Check what is being read from the IMU
            #ifdef SMART_DOG_COLLAR_DEBUG
            Serial.println("Data from gyroscope");
            Serial.print(current_gyroscope_data[0]);
            Serial.print('\t');
            Serial.print(current_gyroscope_data[1]);
            Serial.print('\t');
            Serial.println(current_gyroscope_data[2]);
            #endif
        }
        else
        {
            Serial.println("Failed to read gyroscope data");
            return false;
        }
    }
    else
    {
        Serial.println("No gyroscope data found");
        return false;
    }

    // Try to read data from the accelerometer
    if(IMU.accelerationAvailable())
    {
        const int acceleration_index = (acceleration_data_index % ACCELERATION_DATA_LENGTH);
        acceleration_data_index += 3;
        float* current_acceleration_data = &acceleration_data[acceleration_index];

        // Read acceleration sample
        if (IMU.readAcceleration(current_acceleration_data[0], current_acceleration_data[1], current_acceleration_data[2]))
        {
            // Give normalized data to ml model 
            for(int i = 0; i < ACCELERATION_COUNT; i++)
            {
                current_acceleration_data[i] /= A_NORMALIZATION;
                input[i + GYROSCOPE_COUNT] = current_acceleration_data[i];
            }

            // Check what is being read from the IMU
            #ifdef SMART_DOG_COLLAR_DEBUG
            Serial.println("Data from accelerometer");
            Serial.print(current_acceleration_data[0]);
            Serial.print('\t');
            Serial.print(current_acceleration_data[1]);
            Serial.print('\t');
            Serial.println(current_acceleration_data[2]);
            #endif
        }
        else
        {
            Serial.println("Failed to read acceleration data");
            return false;
        }
    }
    else
    {
        Serial.println("No accelerometer data found");
        return false;
    }

    skip_counter++;
    return true;
}
#endif
