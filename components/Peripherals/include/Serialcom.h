#ifndef UARTMASTER_H
#define UARTMASTER_H

#include "driver/uart.h"
#include <string>

class UARTMaster {
private:
    uart_port_t uart_num;  // UART port (UART_NUM_0, UART_NUM_1, UART_NUM_2)
    int tx_pin;           // Transmit pin
    int rx_pin;           // Receive pin
    int baud_rate;        // Baud rate
    int buffer_size;      // UART buffer size

public:
    UARTMaster(uart_port_t uart_num, int tx_pin, int rx_pin, int baud_rate = 115200, int buffer_size = 1024);
    void begin();  // Initialize UART
    void send(const std::string& data);  // Send data
    std::string receive(int max_length);  // Receive data
    ~UARTMaster();  // Clean up UART resources
};

#endif // UARTMASTER_H
