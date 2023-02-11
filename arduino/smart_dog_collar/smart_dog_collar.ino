#include <ArduinoBLE.h>
// #include <Arduino_LSM9DS1.h> // Rev 1
#include <Arduino_BMI270_BMM150.h> // Rev 2
#include <TensorFlowLite.h>

#include <cmath>

#include "./inc/smart_dog_collar_model_data.h"
#include "./inc/sensors.h"
#include "./inc/output_handler.h"

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#define DATA_LENGTH 400
#define ACCELERATION_COUNT 3
#define ACCELERATION_DATA_LENGTH (DATA_LENGTH * 3)
#define GYROSCOPE_COUNT 3
#define GYROSCOPE_DATA_LENGTH (DATA_LENGTH * 3)
#define INPUT_COUNT 6
#define TARGET_HZ 17 // Take 17 samples per second
#define NORMALIZATION 1000

#define SMART_DOG_COLLAR_DEBUG
// #undef SMART_DOG_COLLAR_DEBUG
// TODO LIST
// 1: Double check data FIFO
// 2: tensor input/output checks
// 3: Figure out if the current ml model input is correct
// 4: output handler (AWS stuff)

namespace 
{
  const int VERSION = 0x00000001;

  /*************/
  /* Variables */
  /*************/
  constexpr int input_count = 6;
  constexpr int label_count = 7;

  // Create an area of memory to use for input, output, and intermediate arrays.
  // The size of this will depend on the model you're using, and may need to be
  // determined by experimentation.
  constexpr int kTensorArenaSize = 90 * 1024;
  uint8_t tensor_arena[kTensorArenaSize];

  tflite::ErrorReporter *error_reporter = nullptr;
  const tflite::Model *model = nullptr;
  tflite::MicroInterpreter *interpreter = nullptr;
  TfLiteTensor *model_input = nullptr;
  TfLiteTensor *model_output = nullptr;
  Sensors sensor;
  Output_Handler output_handler;
}

void setup() {
  // Setup Serial
  tflite::InitializeTarget();

  // Setup logging
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;
  error_reporter->Report("Started");

  // Setup structs
  sensor = Sensors();
  output_handler = Output_Handler();

  // Try to start up the IMU
  bool sensor_status = sensor.setupIMU(error_reporter);
  if(!sensor_status)
  {
    error_reporter->Report("Sensor failed to start");
    return;
  }

  // Map the model into a usable data structure.
  model = tflite::GetModel(sdc_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) 
  {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);

    return;
  }

  // Pulls in all the operation implementations we need
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
    error_reporter->Report("Memory allocations failed");
    return;
  }

  // TODO 2: Check if setup correctly
  // Obtain pointer to model's input and check model input parameters
  model_input = interpreter->input(0);
  // if ((model_input->dims->size != input_count) || (model_input->dims->data[0] != 1)) 
  // {
  //   error_reporter->Report("Bad input tensor parameters in model");
  //   return;
  // }

  // TODO 3 Random testing for input size
  int input_length = model_input->bytes / sizeof(float);
  error_reporter->Report("Input length: %d", input_length);

  // TODO 2: Check if setup correctly
  // Obtain pointer to model's output and check model output parameters
  model_output = interpreter->output(0);
  // if ((model_output->dims->size != label_count) || (model_output->dims->data[0] != 1) || (model_output->dims->data[1] != label_count)) 
  // {
  //   error_reporter->Report("Bad output tensor parameters in model");
  //   return;
  // }
}

void loop() {
  // Check if data is avaliable
  const bool data_available = IMU.accelerationAvailable() || IMU.gyroscopeAvailable();
  if (!data_available) 
  {
    return;
  }

  // Read data from sensors
  sensor.readAccelerometerAndGyroscope(error_reporter, model_input->data.f);

  // Invokes the interpreter
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) 
  {
    error_reporter->Report("Invoke failed");
    return;
  }

  // Read the results of the ml model
  int8_t max_score = 0;
  int max_index = 0;
  for (int i = 0; i < label_count; i++) 
  {
    const int8_t score = model_output->data.f[i];
    if ((i == 0) || (score > max_score)) 
    {
      max_score = score;
      max_index = i;
    }
  }

  // Handle the results of the ml model
  output_handler.handleOutput(error_reporter, max_index);
}
