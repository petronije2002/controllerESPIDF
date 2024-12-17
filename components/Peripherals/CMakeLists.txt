


idf_component_register(SRCS "src/USBcom.cpp"
                            "src/ADC.cpp"
                            "src/CANcom.cpp"
                            "src/MCPWM.cpp"
                            "src/Serialcom.cpp"
                            "src/SPImaster.cpp"
                            "src/Pinset.cpp"

                REQUIRES "esp_driver_uart" "freertos" "esp_driver_spi" "tinyusb" "usb" "espressif__esp_tinyusb"
                "esp_driver_gpio"
                


                    INCLUDE_DIRS "include" )
