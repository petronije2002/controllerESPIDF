#ifndef USBCOM_H
#define USBCOM_H

#include "esp_log.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include <vector>
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"


extern  const int BUFFER_SIZE ;
extern  char inputBuffer[];  // Buffer for incoming data
extern  size_t currentDataLength ;
extern TaskHandle_t parserTaskHandle; // Handle for the parser task


class USBDevice {
public:
    USBDevice();
    ~USBDevice();

    static USBDevice* instance;

    static constexpr size_t BUFFER_SIZE = 256;  // Declare the buffer size as a constexpr


    // static USBDevice& instance();


    void begin(); // Initialize the USB device
    void send(const char* data,size_t len); // Send data over USB
    void onDataReceived(void (*callback)(uint8_t* data, size_t len)); // Set data received callback

    void printf(const char *format, ...);

private:
    static void cdcRxCallback(int itf, cdcacm_event_t* event); // Internal CDC RX callback
    void processData(uint8_t* data, size_t len); // Data processing method

    
};

#endif // USBCOM_H
