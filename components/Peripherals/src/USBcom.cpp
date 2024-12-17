#include "USBcom.h"

static const char *TAG = "USBDevice";

USBDevice::USBDevice()
{

    // static USBDevice* instance;
    ESP_LOGI(TAG, "USBDevice instance created");

    USBDevice* instance = this;
}

USBDevice::~USBDevice()
{
    ESP_LOGI(TAG, "USBDevice instance destroyed");
}

void USBDevice::begin()
{
    ESP_LOGI(TAG, "Starting USB initialization...");

    // TinyUSB driver configuration
    tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false

    };

    // Install the TinyUSB driver
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    // Configure the CDC ACM class
    tinyusb_config_cdcacm_t acm_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 64,
        .callback_rx = &USBDevice::cdcRxCallback,
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = NULL,
        .callback_line_coding_changed = NULL};

    // Initialize the CDC ACM interface
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));

    ESP_LOGI(TAG, "USB initialization completed.");
}

void USBDevice::send(const char *data, size_t len)
{
    // Write data to the CDC interface
    // size_t len = strlen(data); // Correct length of the string

    if (tud_cdc_connected())
    {
        // ESP_LOGI(TAG, "Sending data: %s", data); // Log data before sending
        tud_cdc_write(data, len);
        tud_cdc_write_flush();
    }
    else
    {
        ESP_LOGW(TAG, "USB not connected. Unable to send data.");
    }
}


void USBDevice::processData(uint8_t *data, size_t len)
{
    ESP_LOGI(TAG, "Processing %zu bytes of data: %.*s", len, (int)len, data);

    send(reinterpret_cast<const char *>(data), len);

    // Add your data processing logic here
}

void USBDevice::cdcRxCallback(int itf, cdcacm_event_t *event)
{

    if (event->type == CDC_EVENT_RX)
    {
        uint8_t buffer[64]; // Temporary buffer for incoming data
        size_t len = tud_cdc_n_read(itf, buffer, sizeof(buffer));



            if (tud_cdc_connected())
        {
            // ESP_LOGI(TAG, "Sending data: %s", data); // Log data before sending
            tud_cdc_write(buffer, len);
            tud_cdc_write_flush();
        }
        else
        {
            ESP_LOGW(TAG, "USB not connected. Unable to send data.");
        }

        // if (instance != nullptr)
        // {                                       // Ensure the instance is valid
        //     instance->processData(buffer, len); // Call processData on the instance
        // }
        // else
        // {
        //     ESP_LOGE(TAG, "USBDevice instance is null!");
        // }
    }
}
