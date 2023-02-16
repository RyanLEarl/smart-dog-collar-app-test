Notes

1: The following files contain #define BLE_SENSE_BOARD" and "#undef BLE_SENSE_BOARD":

smart_dog_collar.ino

sensors.cpp

sensors.h

When working with the Arduino IoT, make sure to comment out the "#undef BLE_SENSE_BOARD" for every single file listed above


2: If you are using the ble sense arduino and don't have wifi, uncomment "// #undef BLE_SENSE_NO_WIFI" in smart_dog_collar.ino


3: The following files contain "#define SMART_DOG_COLLAR_DEBUG" and "#undef SMART_DOG_COLLAR_DEBUG"

smart_dog_collar.ino

sensors.cpp

If you want the debugging values to be shown, make sure to comment out "#undef SMART_DOG_COLLAR_DEBUG" for the files where you want this change to occur


4: The output_handler.cpp and output_handler.h files are not being used due to the AWS arduino code causing problems. So it has been moved to the smart_dog_collar.ino file. They are kept just in case a way to prevent the problem is found.

5: The BLE Sense board does not work due to not having wifi

6: The warnings about "changing start of section .bss by 4 bytes" shouldn't cause any negative effects
