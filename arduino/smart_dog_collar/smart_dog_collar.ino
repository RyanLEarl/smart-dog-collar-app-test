#define SMART_DOG_COLLAR_DEBUG
// #undef SMART_DOG_COLLAR_DEBUG
#define BLE_SENSE_BOARD
// #undef BLE_SENSE_BOARD
#ifdef BLE_SENSE_BOARD
#define BLE_SENSE_NO_WIFI
// #undef BLE_SENSE_NO_WIFI
#endif

#include "./inc/sensors.h"
// #include "./inc/output_handler_temp.h"

#ifdef BLE_SENSE_BOARD
#include <TensorFlowLite.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "./inc/smart_dog_collar_model_data.h"
// #include <Arduino_LSM9DS1.h> // Rev 1
#include <Arduino_BMI270_BMM150.h> // Rev 2
#ifndef BLE_SENSE_NO_WIFI
#include <WiFiNINA.h>
#endif
#else
#include <Arduino_LSM6DS3.h> // IoT
#include <WiFiNINA.h>
#endif

#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include "./inc/arduino_secrets.h"

#define SENSOR_COUNT 6
#define LABEL_COUNT 7
#define SEIZURE 4
#define READS_PER_SECOND 16
// TODO LIST
// 1: output handler (AWS push notification stuff)
// 2: Collect data using ble sense to train new model on
// 3: Eventually develop a better neural network

const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_PASS;
const char broker[]      = SECRET_BROKER;
const char* certificate  = SECRET_CERTIFICATE;

#ifndef BLE_SENSE_NO_WIFI
WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);
#endif

// Strings for outputting data to AWS
String stringXGyro, stringYGyro, stringZGyro, 
       stringXAccel, stringYAccel, stringZAccel, 
       stringOne, stringTwo, stringThree, stringFour, 
       stringFive, stringSix, stringSeven, stringOutput;

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
  // OutputHandler output_handler;
  float input_buffer[input_size] = {1.0f}; // Gyroscope x, y, z followed by accelerometer x, y, z
  uint8_t buffer_start = 0;
  uint8_t output_buffer[READS_PER_SECOND] = {5};
  uint8_t output_buffer_start = 0;

  // Create an area of memory to use for input, output, and intermediate arrays.
  // The size of this will depend on the model you're using, and may need to be
  // determined by experimentation. But can be approximated by dividing the model size by 1024
  #ifdef BLE_SENSE_BOARD
  constexpr int kTensorArenaSize = 50 * 1025;
  uint8_t tensor_arena[kTensorArenaSize];
  tflite::ErrorReporter *error_reporter = nullptr;
  const tflite::Model *model = nullptr;
  tflite::MicroInterpreter *interpreter = nullptr;
  TfLiteTensor *model_input = nullptr;
  TfLiteTensor *model_output = nullptr;
  #endif

  const char *labels[LABEL_COUNT] = { "car", "leisure", "play", "run_jog",
                                      "seizure", "sleep", "walk" };
  int gyroscopeOrientation;
  int sensorSensitivity = 100;
}

void setupOutputHandler();
#ifdef BLE_SENSE_BOARD
void handleOutput(tflite::ErrorReporter*, int, float*);
#else
void handleOutput(int, float*);
#endif
void publishMessage();
void connectWiFi();
unsigned long getTime();
void connectMQTT();
void onMessageReceived(int);

// Runs once on startup
void setup() 
{
  #ifdef BLE_SENSE_BOARD
  // Setup Serial
  tflite::InitializeTarget();

  // Setup logging
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;
  #else
  Serial.begin(9600);
  #endif

  error_reporter->Report("Attempting to setup");

  // Setup structs
  sensor = Sensors();
  // output_handler = OutputHandler();
  #ifndef BLE_SENSE_BOARD
  setupOutputHandler();
  #endif

  #ifdef BLE_SENSE_BOARD
  // Try to start up the IMU
  bool sensor_status = sensor.setupIMU(error_reporter);
  if(!sensor_status)
  {
    error_reporter->Report("Sensor failed to start");
    return;
  }
  error_reporter->Report("Sensor setup successful");

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
    error_reporter->Report("Memory allocations failed");
    return;
  }

  // Obtain pointer to model's input and check model input parameters
  model_input = interpreter->input(0);
  if ((model_input->dims->size != 3) || (model_input->dims->data[0] != 1)
                                     || (model_input->dims->data[1] != input_width)
                                     || (model_input->dims->data[2] != input_count)) 
  {
    error_reporter->Report("Bad input tensor parameters in model");
    return;
  }

  // Obtain pointer to model's output and check model output parameters
  model_output = interpreter->output(0);
  if ((model_output->dims->size != 2) || (model_output->dims->data[0] != 1) 
                                      || (model_output->dims->data[1] != label_count)) 
  {
    error_reporter->Report("Bad output tensor parameters in model");
    return;
  }

  error_reporter->Report("Setup successful");
  #else // iot board
  bool sensor_status = sensor.setupIMU();
  if(!sensor_status)
  {
    Serial.println("Sensor failed to start");
    return;
  }

  Serial.println("Setup successful");
  #endif
}

// Runs forever once setup is done
void loop() 
{
  // Add a timer
  const unsigned long timer = 1000UL / READS_PER_SECOND; // 17 reads per second
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
    error_reporter->Report("Invalid buffer start value");
    buffer_start %= input_width;
    return;
  }

  // Read data from sensors
  #ifdef BLE_SENSE_BOARD
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
    error_reporter->Report("Invoke failed");
    return;
  }

  // Read the results of the ml model
  float max_score_float = 0;
  uint8_t max_index = 0;
  for(int i = 0; i < label_count; i++)
  {
    const float float_score = model_output->data.f[i];
    // error_reporter->Report("Float Score of %d: %f", i, float_score);
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
  // output_handler.handleOutput(error_reporter, max_index, input_buffer);
  if(output_buffer_start == 0)
  {
      handleOutput(error_reporter, max_index, input_buffer);
  }
  #else // iot board
  uint8_t max_index = 0;
  if(!sensor.readAccelerometerAndGyroscope(input_buffer&input_buffer[buffer_start * input_count]))
  {
    return;
  }
  #endif

  buffer_start++;
  buffer_start %= input_width;
}

#ifndef BLE_SENSE_BOARD
void setupOutputHandler()
{
    while (!Serial);
    stringXGyro = String("{\"body\":{\"xRotationalAxis\":\"");
    stringYGyro = String("\", \"yRotationalAxis\":\""); 
    stringZGyro = String("\", \"zRotationalAxis\":\""); 
    stringXAccel = String("{\"body\":{\"xAcceleration\":\"");
    stringYAccel = String("\", \"yAcceleration\":\""); 
    stringZAccel = String("\", \"zAcceleration\":\""); 
    stringOne = String();
    stringTwo = String();
    stringThree = String();
    stringFour = String();
    stringFive = String();
    stringSix = String();
    stringSeven = String("\"}}");
    stringOutput = String();
    
    // Check is ECCX08 is ready
    if (!ECCX08.begin()) 
    {
        Serial.println("No ECCX08 present!");
        while (1);
    }

    // Set a callback to get the current time
    // used to validate the servers certificate
    ArduinoBearSSL.onGetTime(getTime);

    // Set the ECCX08 slot to use for the private key
    // and the accompanying public certificate for it
    sslClient.setEccSlot(0, certificate);

    // Optional, set the client id used for MQTT,
    // each device that is connected to the broker
    // must have a unique client id. The MQTTClient will generate
    // a client id for you based on the millis() value if not set
    //
    mqttClient.setId("iotconsole-8bfa76b7-02fb-4391-b5bb-46cc4a524e23");

    // Set the message callback, this function is
    // called when the MQTTClient receives a message
    mqttClient.onMessage(onMessageReceived);
}
#endif

#ifdef BLE_SENSE_BOARD 
#ifdef BLE_SENSE_NO_WIFI
void handleOutput(tflite::ErrorReporter* error_reporter, int activity, float *sensor_data)
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

    // Handle seizure
    if(event == SEIZURE)
    {
        // Send push notification
        error_reporter->Report("Seizure detected");
        error_reporter->Report("Seizure detected");
        error_reporter->Report("Seizure detected");
        error_reporter->Report("Seizure detected");
        #ifdef SMART_DOG_COLLAR_DEBUG
        delay(5000);
        #endif
    }

    #ifdef SMART_DOG_COLLAR_DEBUG
    // Check what the sensors were
    for(int i = 0; i < SENSOR_COUNT; i++)
    {
        // Serial.println(sensor_data[i]);
    }

    // Check what was the result of the model
    error_reporter->Report(labels[event]);
    #endif
}
#else
void handleOutput(tflite::ErrorReporter* error_reporter, int activity, float *sensor_data)
{ 
    // Connect to wifi if not yet connected
    if (WiFi.status() != WL_CONNECTED) 
    {
        connectWiFi();
    }

    // Make sure MQTT is connected to
    if (!mqttClient.connected()) 
    {
        // MQTT client is disconnected, connect
        connectMQTT();
    }

    // poll for new MQTT messages and send keep alive
    mqttClient.poll();

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

    // Handle seizure
    if(event == SEIZURE)
    {
        // Send push notification
        #ifdef SMART_DOG_COLLAR_DEBUG
        delay(5000);
        #endif
    }

    #ifdef SMART_DOG_COLLAR_AWS_DEBUG
    float gyroX = sensor_data[0];
    if(false)
    // if (gyroX > gyroscopeOrientation + sensorSensitivity || gyroX < gyroscopeOrientation - sensorSensitivity) 
    {
        gyroscopeOrientation = sensor_data[0];
        stringOne = stringXGyro + sensor_data[0];
        stringTwo = stringYGyro + sensor_data[1];
        stringThree = stringZGyro + sensor_data[2];
        stringFour = stringXAccel + sensor_data[3];
        stringFive = stringYAccel + sensor_data[4];
        stringSix = stringZAccel + sensor_data[5];
        stringOutput = stringOne + stringTwo + stringThree + stringFour + stringFive + stringSix + stringSeven;
        Serial.println(gyroscopeOrientation);
        publishMessage();
    }
    #endif

    #ifdef SMART_DOG_COLLAR_DEBUG
    // Check what the sensors were
    for(int i = 0; i < SENSOR_COUNT; i++)
    {
        // Serial.println(sensor_data[i]);
    }

    // Check what was the result of the model
    error_reporter->Report(labels[event]);
    #endif
}
#endif
#else
void handleOutput(int activity, float *sensor_data)
// void handleOutput(int activity, float *sensor_data)
{ 
    // Connect to wifi if not yet connected
    if (WiFi.status() != WL_CONNECTED) 
    {
        connectWiFi();
    }

    // Make sure MQTT is connected to
    if (!mqttClient.connected()) 
    {
        // MQTT client is disconnected, connect
        connectMQTT();
    }

    // poll for new MQTT messages and send keep alive
    mqttClient.poll();

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

    // Handle seizure
    if(event == SEIZURE)
    {
        // Send push notification
        #ifdef SMART_DOG_COLLAR_DEBUG
        delay(5000);
        #endif
    }

    #ifdef SMART_DOG_COLLAR_AWS_DEBUG
    float gyroX = sensor_data[0];
    if(false)
    // if (gyroX > gyroscopeOrientation + sensorSensitivity || gyroX < gyroscopeOrientation - sensorSensitivity) 
    {
        gyroscopeOrientation = sensor_data[0];
        stringOne = stringXGyro + sensor_data[0];
        stringTwo = stringYGyro + sensor_data[1];
        stringThree = stringZGyro + sensor_data[2];
        stringFour = stringXAccel + sensor_data[3];
        stringFive = stringYAccel + sensor_data[4];
        stringSix = stringZAccel + sensor_data[5];
        stringOutput = stringOne + stringTwo + stringThree + stringFour + stringFive + stringSix + stringSeven;
        Serial.println(gyroscopeOrientation);
        publishMessage();
    }
    #endif

    #ifdef SMART_DOG_COLLAR_DEBUG
    // Check what the sensors were
    for(int i = 0; i < SENSOR_COUNT; i++)
    {
        Serial.println(sensor_data[i]);
    }

    // Check what was the result of the model
    Serial.println(labels[event]);
    #endif
}

void publishMessage() 
{
    // char json4[] = 
    // char json3[] = std::string("\", \"z-rotational-axis\":\"") + z + "\"}}";
    // char json2[] = std::string("\", \"y-rotational-axis\":\"") + y + 
    // char json[] = std::string("{\"body\":{\"x-rotational-axis\":\"") + gyroscopeOrientation + "\", \"y-rotational-axis\":\"" + y + "\", \"z-rotational-axis\":\"" + z + "\"}}";
    // Serial.println(json);
    // Serial.println("Publishing message");

    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage("smart-dog-collar");
    // mqttClient.print("I have been moved: ");
    mqttClient.print(stringOutput);
    mqttClient.endMessage();
}
#endif

#ifndef BLE_SENSE_BOARD
void connectWiFi() 
{
    Serial.print("Attempting to connect to SSID: ");
    Serial.print(ssid);
    Serial.print(" ");

    while (WiFi.begin(ssid, pass) != WL_CONNECTED) 
    {
        // failed, retry
        Serial.print(".");
        delay(5000);
    }
    Serial.println();

    Serial.println("You're connected to the network");
    Serial.println();
}

unsigned long getTime() 
{
    // get the current time from the WiFi module  
    return WiFi.getTime();
}

void connectMQTT() 
{
    Serial.print("Attempting to MQTT broker: ");
    Serial.print(broker);
    Serial.println(" ");

    while (!mqttClient.connect(broker, 8883)) 
    {
        // failed, retry
        Serial.print(".");
        delay(5000);
    }
    Serial.println();

    Serial.println("You're connected to the MQTT broker");
    Serial.println();

    // subscribe to a topic
    mqttClient.subscribe("$aws/things/Ryan-smart-dog-collar-nano-33-iot/shadow/update");
}

void onMessageReceived(int messageSize) 
{
    // we received a message, print out the topic and contents
    Serial.print("Received a message with topic '");
    // Serial.print(mqttClient.messageTopic());
    Serial.print("', length ");
    Serial.print(messageSize);
    Serial.println(" bytes:");

    // use the Stream interface to print the contents
    while (mqttClient.available()) 
    {
        Serial.print((char)mqttClient.read());
    }
    Serial.println();

    Serial.println();
}
#endif