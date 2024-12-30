#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


#include "driver/spi_master.h"
#include "math.h"
#include "esp_log.h"
// #include "driver/uart.h"
#include "tusb.h"

#include "USBcom.h"
#include "SPImaster1.h"
#include "Driver.h"
#include "AS5048my.h"
#include "ProfileGenerator.h"

#include "QueueHandler.h"
#include "tasksDefinitions.h"
#include "freertosconfig.h"





// #define configTICK_RATE_HZ    (8000)

#define BUFFER_SIZE 128
 char inputBuffer[BUFFER_SIZE];  // Buffer for incoming davta
 size_t currentDataLength = 0; 
TaskHandle_t parserTaskHandle;

// TaskHandle_t encoderTaskHandle;




QueueHandler queueHandler= QueueHandler();

esp_timer_handle_t timer_handle;  // Define the variable here


// static const char* TAG = "MAIN_CPP";  // Logging tag

//UARTMaster uart(UART_NUM_0,43,44,115200);

USBDevice usb_ = USBDevice();

SPI spi(SPI2_HOST, MOSI_PIN , MISO_PIN, SCK_PIN);

spi_device_handle_t spi_encoder_handle_;


Driver driver_(36,37,38,39,40,41);


AS5048 Encoder = AS5048(spi , 10 , 10000000,  1 ) ;


ProfileGenerator profGen = ProfileGenerator(10.0,2, 0, 15);


extern "C" void setup(){


    if( xTaskCreatePinnedToCore(
        parserTask,                // Task function
        "Parser Task",             // Task name
        4096,                      // Stack size (adjust as needed)
        NULL,                      // Task parameters (None)
        1,                         // Priority
        &parserTaskHandle,         // Task handle (to refer to it later)
        0                          // Core (0 or 1, based on the core you want to pin the task to)
    ) == pdTRUE){

        ESP_LOGI("parserTask created","d");

    };


    if(xTaskCreate(commandQueueConsumerTask, "CommandQueueConsumerTask", 4096, NULL, 1, NULL) ==pdTRUE){

         ESP_LOGI("CommandQueueConsumerTask created","d");

    };

    vTaskDelay(pdMS_TO_TICKS(500));


    //  if(xTaskCreate(printMultiturnAngleTask, "printMultiTurnTask", 8192,  (void *)&Encoder, 2, NULL) ==pdTRUE){

    //      ESP_LOGI("printMultiturntask created","d");

    //       ESP_LOGI("HEAP", "Free heap: %lu", esp_get_free_heap_size());

    // };

    setup_timer();
    

}

void sendProfilesOverUSB(const std::vector<float> &positionProfile,
                         const std::vector<float> &velocityProfile)
{
    // Ensure the same size for all profiles
    size_t numPoints = velocityProfile.size();

    // Send profiles as CSV-like format: time, position, velocity
    // usb_.printf("Position, Velocity");
    for (size_t i = 0; i < numPoints; ++i)
    {

        if (i % 10 == 0)
        { // Log every 10th value
            ESP_LOGW("TAG", "Position %f, velocity: %f", positionProfile[i], velocityProfile[i]);
        }
        // usb_.printf("%.3f, %.3f\n", positionProfile[i], velocityProfile[i]);

        // ESP_LOGW("TAG","Position %f, velocity: %f", positionProfile[i],velocityProfile[i]);
    }
    // usb_.printf("End of profiles.");
}

extern "C" void app_main()
{

    // setup();

    int64_t start_time = esp_timer_get_time();


    profGen.generateScurveProfile(6,2.8);

    int64_t end_time = esp_timer_get_time();


     int64_t execution_time = end_time - start_time; // Time in microseconds
     ESP_LOGW("ExecutionTime", "generateScurveProfile execution time: %lld microseconds", execution_time);



    // vTaskDelay(pdMS_TO_TICKS(100));

    //  setup_timer();  // Your custom timer setup (e.g., for the encoder)

       int ind_ = 0;


       const std::vector<float>& positionProfile = profGen.getPositionProfile();
       const std::vector<float>& velocityProfile = profGen.getVelocityProfile();
       const std::vector<float>&  timeProfile = profGen.getTimeProfile();
    

    while (1)
    {


        if (ind_ < positionProfile.size())
        {


         ESP_LOGW("Profile", " %f, %f, %f", timeProfile[ind_], positionProfile[ind_],velocityProfile[ind_]);

            // // float position = positionProfile[ind_];
            // ESP_LOGW("Profile", "Position[%d]: %f", ind_, positionProfile[ind_]);
            ind_++;
        }
        else
        {
            ESP_LOGW("Profile", "End of profile reached.");
            break; // Exit the loop or handle as needed
        }


        vTaskDelay(pdMS_TO_TICKS(300));

      

      
    }
    
}