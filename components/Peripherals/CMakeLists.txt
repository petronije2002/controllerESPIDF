


idf_component_register(SRCS "src/USBcom.cpp"
                            "src/ADC.cpp"
                            "src/CANcom.cpp"
                            "src/MCPWM.cpp"
                            "src/Serialcom.cpp"
                            "src/SPImaster.cpp"

                REQUIRES "esp_driver_uart" "freertos" "esp_driver_spi" "tinyusb" "usb" "espressif__esp_tinyusb"
                


                    INCLUDE_DIRS "include" )
