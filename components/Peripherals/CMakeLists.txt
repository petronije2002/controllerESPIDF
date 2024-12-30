


idf_component_register(SRCS "src/USBcom.cpp"
                            
                            "src/CANcom.cpp"
                            
                            "src/Serialcom.cpp"
                            "src/SPImaster1.cpp"
                            "src/Pinset.cpp"
                            "src/QueueHandler.cpp"
                            "src/tasksDefinitions.cpp"
                            "src/Driver.cpp"
                            "src/AS5048my.cpp"
                            "src/ProfileGenerator.cpp"

                REQUIRES "esp_driver_uart" "freertos" "esp_driver_spi" "tinyusb" "usb" "espressif__esp_tinyusb"
                "esp_driver_gpio" "esp_driver_mcpwm" "driver" "esp_timer" "esp_driver_gptimer"
                


                    INCLUDE_DIRS "include" )
