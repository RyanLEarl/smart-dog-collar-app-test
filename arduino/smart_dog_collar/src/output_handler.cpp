#include "../inc/output_handler.h"

#define SMART_DOG_COLLAR_DEBUG
// #undef SMART_DOG_COLLAR_DEBUG

// TODO 4: handle output in some way
void Output_Handler::handleOutput(tflite::ErrorReporter* error_reporter, int activity)
{ 
    // Handle seizure
    if(activity == SEIZURE)
    {
        // Send push notification?

    }

    // Everything that must be done
    // Send status to phone

    #ifdef SMART_DOG_COLLAR_DEBUG
    // Check what was the result of the model
    error_reporter->Report(labels[activity]);
    #endif
}