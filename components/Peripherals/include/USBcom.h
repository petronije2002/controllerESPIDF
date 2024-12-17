#ifndef USBCOM_H
#define USBCOM_H

#include "esp_log.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"


class USBDevice {
public:
    USBDevice();
    ~USBDevice();

    static USBDevice* instance;


    void begin(); // Initialize the USB device
    void send(const char* data,size_t len); // Send data over USB
    void onDataReceived(void (*callback)(uint8_t* data, size_t len)); // Set data received callback

private:
    static void cdcRxCallback(int itf, cdcacm_event_t* event); // Internal CDC RX callback
    void processData(uint8_t* data, size_t len); // Data processing method

    
};

#endif // USBCOM_H
