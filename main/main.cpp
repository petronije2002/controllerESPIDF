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

#include "QueueHandler.h"
#include "tasksDefinitions.h"
#include "freertosconfig.h"

// #define configTICK_RATE_HZ    (8000)

#define BUFFER_SIZE 128
 char inputBuffer[BUFFER_SIZE];  // Buffer for incoming davta
 size_t currentDataLength = 0; 
TaskHandle_t parserTaskHandle;



QueueHandler queueHandler= QueueHandler();



// static const char* TAG = "MAIN_CPP";  // Logging tag

//UARTMaster uart(UART_NUM_0,43,44,115200);

USBDevice usb_ = USBDevice();

SPI spi(SPI2_HOST, MOSI_PIN , MISO_PIN, SCK_PIN);

spi_device_handle_t spi_encoder_handle_;


Driver driver_(36,37,38,39,40,41);


AS5048 Encoder = AS5048(spi , 10 , 10000000,  1 ) ;


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


    if(xTaskCreate(commandQueueConsumerTask, "CommandQueueConsumerTask", 4096, NULL, 2, NULL) ==pdTRUE){

         ESP_LOGI("CommandQueueConsumerTask created","d");

    };


     if(xTaskCreate(printMultiturnAngleTask, "printMultiTurnTask", 4096, NULL, 3, NULL) ==pdTRUE){

         ESP_LOGI("printMultiturntask created","d");

    };



   


}




extern "C" void app_main()
{



   

    while (1)
    {


      
        // Encoder.readAngle();


        // usb_.printf("Current angle: %f \n", Encoder.getAngle());

        usb_.printf("Multiturn angle: %f \n", Encoder.getMultiTurnAngle()),


     

     
    //     spi.transfer( spi.tx_data_ , spi.rx_data_,sizeof(spi.tx_data_));

    //     // ESP_LOGI(TAG, "Received: ", spi.rx_data_[0]);


    //     uint16_t rawAngle = spi.result & 0x3FFF;  // This keeps only the lower 14 bits

    //     float newAngle_ = ((float)rawAngle / 16384.0) * M_TWOPI; // to radians 


    //     // char buffer[64];
    //     // snprintf(buffer, sizeof(buffer), "Received angle: %.6f radians\n", newAngle_);

    //     // // snprintf(buffer, sizeof(buffer), "Received: 0x%02X 0x%02X\n", rx_data_[0], rx_data_[1]);

    //     usb_.printf("Current angle: %f\n", newAngle_);
    //     // usb_.send( buffer,sizeof(buffer));
    
        vTaskDelay(pdMS_TO_TICKS(800));
    }
    
}