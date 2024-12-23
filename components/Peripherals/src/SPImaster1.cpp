#include "SPImaster1.h"

static const char *TAG = "SPIMaster1"; // Logging tag

SPI::SPI(spi_host_device_t host, int mosi_pin, int miso_pin, int sclk_pin)
    : spi_host(host)
{

    uint16_t command = 0x3FFF; // = 0x3FFF; ///this is for testing purposes only, to send 'read command' to encoder

    tx_data_[0] = (command >> 8) & 0xFF; // High byte
    tx_data_[1] = command & 0xFF;        // Low byte

    spi_bus_config_t bus_cfg = {}; // Initialize all fields to zero
    bus_cfg.mosi_io_num = mosi_pin;
    bus_cfg.miso_io_num = miso_pin;
    bus_cfg.sclk_io_num = sclk_pin;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = 64;

    esp_err_t ret = spi_bus_initialize(spi_host, &bus_cfg, SPI_DMA_DISABLED);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "SPI bus initialized successfully.");
}
   
void SPI::attachDevice(uint32_t clock_speed_hz, spi_device_handle_t *handle,int cs_pin, int spi_mode )
{
    spi_device_interface_config_t dev_cfg = {}; // Initialize all fields to zero
    dev_cfg.clock_speed_hz = static_cast<int>(clock_speed_hz);
    dev_cfg.mode = static_cast<int>(spi_mode);
    dev_cfg.spics_io_num = cs_pin;
    dev_cfg.queue_size = 1;
    dev_cfg.cs_ena_pretrans = 100;
    dev_cfg.cs_ena_posttrans = 100;
    dev_cfg.input_delay_ns = 100;
    dev_cfg.command_bits = 16;

    esp_err_t ret = spi_bus_add_device(spi_host, &dev_cfg, handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to attach SPI device: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "SPI device attached successfully.");
}

uint16_t SPI::transfer(uint8_t *tx_data, uint8_t *rx_data, size_t length, spi_device_handle_t &handle)
{
    spi_transaction_t transaction = {}; // Initialize all fields to zero
    transaction.length = length * 8;    // Length in bits
    transaction.tx_buffer = tx_data;
    transaction.rx_buffer = rx_data;

    esp_err_t ret = spi_device_transmit(handle, &transaction);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SPI transfer failed: %s", esp_err_to_name(ret));

        return 0;
    }
    else
    {

        uint16_t received_value = (rx_data_[0] << 8) | rx_data_[1]; // combaine to get 16 bits value

        

        // uint16_t rawAngle = *rx_data ;
        // (rx_data[0] << 8) | rx_data[1];  // Combine the two bytes to form a 16-bit value

        // Mask the top 2 bits (PAR and EF)


        // uint16_t rawAngle = received_value & 0x3FFF; // This keeps only the lower 14 bits

        // float newAngle_ = ((float)rawAngle / 16384.0) * M_TWOPI; // to radians

        // this->result = received_value;

        ESP_LOGI(TAG, "SPI transfer completed successfully.");

        return received_value;
        // ESP_LOGI(TAG, "Received angle : %f",newAngle_);
        //  ESP_LOGI(TAG, "Received: %f", *rx_data);
    }
}

SPI::~SPI()
{
    esp_err_t ret;

    // // Remove SPI device
    // ret = spi_bus_remove_device(handle);
    // if (ret == ESP_OK)
    // {
    //     ESP_LOGI(TAG, "SPI device removed successfully.");
    // }
    // else
    // {
    //     ESP_LOGE(TAG, "Failed to remove SPI device: %s", esp_err_to_name(ret));
    // }

    // Free SPI bus
    ret = spi_bus_free(spi_host);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "SPI bus freed successfully.");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to free SPI bus: %s", esp_err_to_name(ret));
    }
}
