#ifndef SPImaster_H
#define SPImaster_H

#include "driver/spi_master.h"
#include "driver/spi_common.h"
#include <cstdint>
#include <stdexcept>


#define MISO_PIN 12  // Replace with your chosen MISO pin
#define MOSI_PIN 11  // Replace with your chosen MOSI pin
#define SCK_PIN 13   // Replace with your chosen SCK pin
#define CS_PIN 10   // Replace with your chosen CS pin



class SPI {
private:
    spi_device_handle_t spi_handle;
    spi_host_device_t spi_host;
    int cs_pin;

public:
    SPI(spi_host_device_t host, int mosi_pin, int miso_pin, int sclk_pin, int cs_pin);
    void attachDevice(uint32_t SPEED, int spi_mode = 1);
    void transfer(const uint8_t* tx_data, uint8_t* rx_data, size_t length);

   
    ~SPI();
};

#endif // SPI_H
