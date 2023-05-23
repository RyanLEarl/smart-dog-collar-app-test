# Smart Dog Collar Arduino Code

## Running the Machine Learning

1. At minimum clone the "arduino" directory which is the parent of this file's parent (smart_dog_collar)
2. Install the Arduino IDE if you haven't already
3. Open up the smart_dog_collar.ino file in your Arduino IDE
4. Connect the Arduino Nano 33 BLE Sense Rev 2 to your device
5. Double click the white reset button to put it in bootloader mode
6. Click on "Upload" to upload code to the arduino (Might take a few minutes)
7. If any errors pop up you might need to install libraries using the Library Manager. The libraries needed are found under arduino/lib
    *  arduino/lib/Arduino_TensorFlowLite should be the only one that needs to be manually installed

## Notes

1. The current machine learning model is the file "arduino/ml_models/byte_models/dense_nn_experiment_working_v2.cc"
2. The following files contains "#define SMART_DOG_COLLAR_DEBUG" and "#undef SMART_DOG_COLLAR_DEBUG". If you want the debugging values to be shown, make sure to comment out "#undef SMART_DOG_COLLAR_DEBUG" for the files where you want this change to occur
    * smart_dog_collar.ino
    * sensors.cpp
3. The following file contains "#define SHOW_ML_OUTPUT" and "#undef SHOW_ML_OUTPUT". If you want the ML output to be shown, make sure to comment out "#undef SHOW_ML_OUTPUT"
    * smart_dog_collar.ino
