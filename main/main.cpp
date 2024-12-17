#include <stdio.h>
#include "Serialcom.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "USBcom.h"
#include "SPImaster.h"
#include "driver/spi_master.h"



//UARTMaster uart(UART_NUM_0,43,44,115200);

USBDevice usb_ = USBDevice();

SPI spi(SPI1_HOST, MOSI_PIN , MISO_PIN, SCK_PIN, CS_PIN);


extern "C" void setup(){
    //uart.begin();

    usb_.begin();
    spi.attachDevice(10000000,1 );

}




extern "C" void app_main()
{

    setup();

    uint8_t tx_data_[2] = {0x3F, 0xFF}; // Command to send
    uint8_t rx_data_[2];                // Buffer for received data


    

    while (1)
    {
       usb_.send("Just to test something\n", strlen("Just to test something\n"));

        spi.transfer(tx_data_,rx_data_,sizeof(tx_data_));

    
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}