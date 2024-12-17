#include "driver/gpio.h"
#include "esp_log.h"



class Pin{

private: 

    int pin ;
    int mode;

    
    

public:
  

    Pin(gpio_num_t pinNumber, gpio_mode_t mode);

    ~Pin();


    void digitalWrite(bool status);



};

