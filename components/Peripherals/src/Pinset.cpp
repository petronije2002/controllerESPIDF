#include "Pinset.h"

static const char* TAG = "Pin";  // Logging tag


Pin::Pin(gpio_num_t pinNumber, gpio_mode_t mode):pin(static_cast<int>(pinNumber)), mode(static_cast<int>(mode))
{
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    if (mode == GPIO_MODE_OUTPUT) {
        io_conf.mode = GPIO_MODE_OUTPUT;
    } else if (mode == GPIO_MODE_INPUT) {
        io_conf.mode = GPIO_MODE_INPUT;
    } else if (mode == GPIO_MODE_INPUT_OUTPUT) {
        io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
    }

    gpio_config(&io_conf);
}


void Pin::digitalWrite(bool status){

    if(mode==GPIO_MODE_OUTPUT){
         gpio_set_level(static_cast<gpio_num_t>(pin), status);
    }else{
        ESP_LOGE(TAG, "Cannot set value to non output pin" );
    }   

}