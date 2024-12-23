// #include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "math.h"
#include "esp_log.h"
#include "esp_log.h"
// #include "driver/uart.h"
#include "tusb.h"


#include "USBcom.h"
#include "SPImaster1.h"
#include "Driver.h"


#include "QueueHandler.h"
#include "tasksDefinitions.h"


#define BUFFER_SIZE 128
 char inputBuffer[BUFFER_SIZE];  // Buffer for incoming data
 size_t currentDataLength = 0; 
TaskHandle_t parserTaskHandle;



QueueHandler queueHandler= QueueHandler();



// static const char* TAG = "MAIN_CPP";  // Logging tag

//UARTMaster uart(UART_NUM_0,43,44,115200);

USBDevice usb_ = USBDevice();

SPI spi(SPI2_HOST, MOSI_PIN , MISO_PIN, SCK_PIN);




Driver driver_(36,37,38,39,40,41);




extern "C" void setup(){
    //uart.begin();

    // spi.attachDevice(10000000,1 );

    driver_.init();

    driver_.setPWMDutyCycle(20,20,20);


}




extern "C" void app_main()
{




    setup();

    // uint8_t tx_data_[2] = {0x3F, 0xFF}; // Command to send
    // uint8_t rx_data_[2];                // Buffer for received data

     
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


    xTaskCreate(commandQueueConsumerTask, "CommandQueueConsumerTask", 4096, NULL, 2, NULL);


    // spi.tx_data_ = 0x3FFF;

    while (1)
    {
    // //    usb_.send("Just to test something\n", strlen("Just to test something\n"));


       

    //     spi.transfer( spi.tx_data_ , spi.rx_data_,sizeof(spi.tx_data_));

    //     // ESP_LOGI(TAG, "Received: ", spi.rx_data_[0]);


    //     uint16_t rawAngle = spi.result & 0x3FFF;  // This keeps only the lower 14 bits

    //     float newAngle_ = ((float)rawAngle / 16384.0) * M_TWOPI; // to radians 


    //     // char buffer[64];
    //     // snprintf(buffer, sizeof(buffer), "Received angle: %.6f radians\n", newAngle_);

    //     // // snprintf(buffer, sizeof(buffer), "Received: 0x%02X 0x%02X\n", rx_data_[0], rx_data_[1]);

    //     usb_.printf("Current angle: %f\n", newAngle_);
    //     // usb_.send( buffer,sizeof(buffer));
    
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}