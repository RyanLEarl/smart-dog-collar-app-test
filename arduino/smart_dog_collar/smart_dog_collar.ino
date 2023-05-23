#define SMART_DOG_COLLAR_DEBUG
#undef SMART_DOG_COLLAR_DEBUG
#define SHOW_ML_OUTPUT
// #undef SHOW_ML_OUTPUT

#include <TensorFlowLite.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

// #include <Arduino_LSM9DS1.h> // Rev 1
#include <Arduino_BMI270_BMM150.h> // Rev 2
#include <ArduinoJson.h>
#include "./inc/sensors.h"
#include "./inc/smart_dog_collar_model_data.h"

#define SENSOR_COUNT 6
#define LABEL_COUNT 7
#define SEIZURE 4
#define READS_PER_SECOND 16
// TODO LIST
// 1: Collect data using ble sense to train new model on
// 2: Eventually develop a better neural network

namespace 
{
    const int VERSION = 0x00000001;

    /*************/
    /* Variables */
    /*************/
    constexpr int input_width = 80;
    constexpr int input_count = 6;
    constexpr int input_size = input_count * input_width;
    constexpr int label_count = 7;

    Sensors sensor;
    float input_buffer[input_size] = {1.0f}; // Gyroscope x, y, z followed by accelerometer x, y, z
    uint8_t buffer_start = 0;
    uint8_t output_buffer[READS_PER_SECOND] = {5};
    uint8_t output_buffer_start = 0;

    // Create an area of memory to use for input, output, and intermediate arrays.
    // The size of this will depend on the model you're using, and may need to be
    // determined by experimentation. But can be approximated by dividing the model size by 1024
    constexpr int kTensorArenaSize = 50 * 1025;
    uint8_t tensor_arena[kTensorArenaSize];
    tflite::ErrorReporter *error_reporter = nullptr;
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *model_input = nullptr;
    TfLiteTensor *model_output = nullptr;

    const char *labels[LABEL_COUNT] = { "car", "leisure", "play", "run_jog",
                                        "seizure", "sleep", "walk" };
}

void serializeResults(tflite::ErrorReporter*, int);

// Runs once on startup
void setup() 
{
    // Setup Serial
    Serial1.begin(9600);
    tflite::InitializeTarget();

    // Setup logging
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;
    #ifdef SMART_DOG_COLLAR_DEBUG
    error_reporter->Report("Attempting to setup");
    #endif

    // Setup struct
    sensor = Sensors();

    // Try to start up the IMU
    bool sensor_status = sensor.setupIMU(error_reporter);
    if(!sensor_status)
    {
        #ifdef SMART_DOG_COLLAR_DEBUG
        error_reporter->Report("Sensor failed to start");
        #endif
        return;
    }
    #ifdef SMART_DOG_COLLAR_DEBUG
    error_reporter->Report("Sensor setup successful");
    #endif

    // Map the model into a usable data structure.
    model = tflite::GetModel(sdc_model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) 
    {
        #ifdef SMART_DOG_COLLAR_DEBUG
        TF_LITE_REPORT_ERROR(error_reporter,
                            "Model provided is schema version %d not equal "
                            "to supported version %d.",
                            model->version(), TFLITE_SCHEMA_VERSION);
        #endif
        
        return;
    }

    // Pulls in all the operation implementations we need
    // Can use a static tflite::MicroMutableOpResolver<x> resolver; if low on memory
    // so we only include the operations we need
    static tflite::AllOpsResolver resolver;

    // Build an intepreter to run the model with
    static tflite::MicroInterpreter static_interpreter(model,
                                                        resolver,
                                                        tensor_arena,
                                                        kTensorArenaSize,
                                                        error_reporter);
    interpreter = &static_interpreter;

    // Runs through the model and allocates all necessary input, output and
    // intermediate tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if(allocate_status != kTfLiteOk)
    {
        #ifdef SMART_DOG_COLLAR_DEBUG
        error_reporter->Report("Memory allocations failed");
        #endif
        return;
    }

    // Obtain pointer to model's input and check model input parameters
    model_input = interpreter->input(0);
    if ((model_input->dims->size != 3) || (model_input->dims->data[0] != 1)
                                        || (model_input->dims->data[1] != input_width)
                                        || (model_input->dims->data[2] != input_count)) 
    {
        #ifdef SMART_DOG_COLLAR_DEBUG
        error_reporter->Report("Bad input tensor parameters in model");
        #endif
        return;
    }

    // Obtain pointer to model's output and check model output parameters
    model_output = interpreter->output(0);
    if ((model_output->dims->size != 2) || (model_output->dims->data[0] != 1) 
                                        || (model_output->dims->data[1] != label_count)) 
    {
        #ifdef SMART_DOG_COLLAR_DEBUG
        error_reporter->Report("Bad output tensor parameters in model");
        #endif
        return;
    }

    #ifdef SMART_DOG_COLLAR_DEBUG
    error_reporter->Report("Setup successful");
    #endif
}

// Runs forever once setup is done
void loop() 
{
    // Add a timer
    const unsigned long timer = 10000UL / READS_PER_SECOND; // 17 reads per second
    static unsigned long lastSampleTime = 0 - timer;
    unsigned long now = millis();

    // See if it is time to take a sample
    if(!((now - lastSampleTime) >= timer))
    {
        return;
    }

    // Make sure the buffer_start is valid
    if(buffer_start >= input_width || buffer_start < 0)
    {
        #ifdef SMART_DOG_COLLAR_DEBUG
        error_reporter->Report("Invalid buffer start value");
        #endif
        buffer_start %= input_width;
        return;
    }

    // Read data from sensors
    if(!sensor.readAccelerometerAndGyroscope(error_reporter, &input_buffer[buffer_start * input_count]))
    {
        // Wasn't able to read data from the sensors so don't do anything else
        return;
    }
    lastSampleTime += timer;

    // Insert data into the model
    for(int i = 0; i < input_count; i++)
    {
        // Insert Gyroscope followed by Accelerometer data starting with the offset + 1 to do FIFO
        model_input->data.f[i] = input_buffer[(((buffer_start + 1) * input_count) + i) % input_size];
    }

    // Invokes the interpreter
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) 
    {
        #ifdef SMART_DOG_COLLAR_DEBUG
        error_reporter->Report("Invoke failed");
        #endif
        return;
    }

    // Read the results of the ml model
    float max_score_float = 0;
    uint8_t max_index = 0;
    for(int i = 0; i < label_count; i++)
    {
        const float float_score = model_output->data.f[i];
    if(float_score > max_score_float)
    {
        max_score_float = float_score;
        max_index = i;
    }
    }

    // Insert result into the output buffer
    output_buffer[output_buffer_start] = max_index;
    output_buffer_start++;
    output_buffer_start %= READS_PER_SECOND;

    // Handle the results of the ml model
    if(output_buffer_start == 0)
    {
        serializeResults(error_reporter, max_index);
    }

    buffer_start++;
    buffer_start %= input_width;
}

// Puts ML results as a serialized json message that will be used later on with WiFi
void serializeResults(tflite::ErrorReporter*, int)
{
    uint8_t event = 0;
    uint8_t event_count = 0;

    // Find label with the highest frequency
    for(uint8_t i = 0; i < (READS_PER_SECOND - 1); i++)
    {
        uint8_t new_event_count = 0;

        // Count current item
        for(uint8_t j = i + 1; j < READS_PER_SECOND; j++)
        {
            if(output_buffer[i] == output_buffer[j])
            {
                new_event_count++;
            }
        }

        // Update event if necessary
        if(new_event_count > event_count)
        {
            event = output_buffer[i];
            event_count = new_event_count;
        }

        // Stop if impossible for any future item
        if(event_count > (READS_PER_SECOND - i - 1))
        {
            break;
        }
    }

    // Make seizure visible in output
    if(event == SEIZURE)
    {
        #ifdef SMART_DOG_COLLAR_DEBUG
        error_reporter->Report("Seizure detected");
        error_reporter->Report("Seizure detected");
        error_reporter->Report("Seizure detected");
        error_reporter->Report("Seizure detected");
        #endif
    }

    // Turn Results into Json
    DynamicJsonDocument doc(256);
    doc["type"] = "request";
    doc["status"] = labels[event];

    // Serialize Json
    serializeJson(doc, Serial1);

    #ifdef SHOW_ML_OUTPUT
    // Check what was the result of the model
    String jsonString;
    deserializeJson(doc, jsonString);
    Serial.println(jsonString);
    #endif
}
