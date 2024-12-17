#include <stdio.h>
#include "Serialcom.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "USBcom.h"

//UARTMaster uart(UART_NUM_0,43,44,115200);

USBDevice usb_ = USBDevice();


extern "C" void setup(){
    //uart.begin();

    usb_.begin();


}




extern "C" void app_main()
{

    setup();



    while (1)
    {
       usb_.send("Just to test something\n", strlen("Just to test something\n"));


       vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}