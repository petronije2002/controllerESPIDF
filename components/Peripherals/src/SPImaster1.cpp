#include "SPImaster1.h"
#include "USBcom.h"



extern USBDevice usb_;

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

    ESP_LOGI(TAG, "Adding SPI device with config: clock=%lu Hz, cs_pin=%d, mode=%d",
         clock_speed_hz, cs_pin, spi_mode);

    // ESP_LOGI(TAG, "SPI device attached successfully.");
}

void SPI::transfer(uint8_t *tx_data, uint8_t *rx_data, size_t length, spi_device_handle_t *handle)
{
    spi_transaction_t transaction = {}; // Initialize all fields to zero
    transaction.length = length * 8;    // Length in bits
    transaction.tx_buffer = tx_data;
    transaction.rx_buffer = rx_data;

    esp_err_t ret = spi_device_transmit( *handle, &transaction);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SPI transfer failed: %s", esp_err_to_name(ret));

       
        return ;
    }
    else
    {
        return ;
      
    }
}

SPI::~SPI()
{
    esp_err_t ret;


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




void SPI::transfer1(uint8_t *tx_data, uint8_t *rx_data, size_t length, spi_device_handle_t *handle)
{
    spi_transaction_t transaction = {}; // Initialize all fields to zero
    transaction.length = length * 8;    // Length in bits
    transaction.tx_buffer = this->tx_data_;
    transaction.rx_buffer = this->rx_data_;

    esp_err_t ret = spi_device_transmit( spi_handle, &transaction);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SPI transfer failed: %s", esp_err_to_name(ret));

       
        return;
    }
    else
    {
        // ESP_LOGI(TAG, "SPI transfer completed:");
        // ESP_LOGI(TAG, "SPI transfer completed successfully.");
        return;
    }
}
