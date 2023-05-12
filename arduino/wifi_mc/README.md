Programming and using the ESP32 WiFi Microcontroller

1. Clone the "arduino" directory which is the parent of this file's parent (wifi_mc).
2. Install the Arduino IDE if you haven't already.
3. Open up the wifi_mc.ino file in your Arduino IDE.
4. You may need to add the ESP32 board definition to the boards manager.
  a. Open the Preferences window.
  b. For Additional Board Manager URLs, add https://dl.espressif.com/dl/package_esp32_index.json.
  c. Choose Tools, Board, Boards Manager.
  d. Search esp32 and install the latest version.
4. You may need to install libraries using the library manager. The libraries needed are found under "arduino/lib".
5. Fill out the "secrets_template.h" file in "wifi_mc/lib" with the necessary credentials. Refer to this link https://aws.amazon.com/blogs/compute/building-an-aws-iot-core-device-using-aws-serverless-and-an-esp32/ for how to create a "thing" in AWS IoT Core and generate the certificates. Rename the file to "secrets.h".
6. Connect the ESP32 to your device.
7. Click on "Upload" to upload code to the arduino (Might take a few minutes).
8. The ESP should automatically connect to the specified WiFi network and then connect to AWS IoT Core.

Refer to this link https://aws.amazon.com/blogs/compute/building-an-aws-iot-core-device-using-aws-serverless-and-an-esp32/ for more detailed instructions.