#ifndef SPImaster_H
#define SPImaster_H

#include "driver/spi_master.h"
#include <cstdint>
#include <stdexcept>

class SPI {
private:
    spi_device_handle_t spi_handle;
    spi_host_device_t spi_host;
    int cs_pin;

public:
    SPI(spi_host_device_t host, int mosi_pin, int miso_pin, int sclk_pin, int cs_pin);
    void attachDevice(uint32_t clock_speed_hz, int spi_mode = 1);
    void transfer(const uint8_t* tx_data, uint8_t* rx_data, size_t length);
    ~SPI();
};

#endif // SPI_H
