#include "Driver.h"

Driver::Driver(int loPhase1, int hoPhase1, int loPhase2, int hoPhase2, int loPhase3, int hoPhase3)
    : _loPhase1(loPhase1), _hoPhase1(hoPhase1),
      _loPhase2(loPhase2), _hoPhase2(hoPhase2),
      _loPhase3(loPhase3), _hoPhase3(hoPhase3) {


        this->init();
      }


  

void Driver::init() {
   

    this->pwm_config.frequency =  38400; // Set frequency to 20 kHz
    this->pwm_config.cmpr_a = 0; // Initialize duty cycle for high-side PWM
    this->pwm_config.cmpr_b = 0; // Initialize duty cycle for low-side PWM
    this->pwm_config.counter_mode = MCPWM_UP_DOWN_COUNTER;
    this->pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    

    // Phase 1
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, _hoPhase1); // High-side
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, _loPhase1); // Low-side
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_A, MCPWM_DUTY_MODE_0);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_B, MCPWM_DUTY_MODE_1);
    mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, 5, 5);

    // Phase 2
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, _hoPhase2); // High-side
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, _loPhase2); // Low-side
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_GEN_A, MCPWM_DUTY_MODE_0);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_GEN_B, MCPWM_DUTY_MODE_1);
    mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, 5, 5);

    // Phase 3
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &pwm_config);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2A, _hoPhase3); // High-side
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2B, _loPhase3); // Low-side
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_GEN_A, MCPWM_DUTY_MODE_0);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_GEN_B, MCPWM_DUTY_MODE_1);
    mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, 5, 5);


    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_1);
    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_2);


    
   
}



void Driver::setPWMDutyCycle(float dutyA, float dutyB, float dutyC) {


    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, dutyA); // Set duty for high-side
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, dutyA); // Set duty for low-side

    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, dutyB); // Set duty for high-side
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, dutyB); // Set duty for low-side

    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A, dutyC); // Set duty for high-side
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_B, dutyC); // Set duty for low-side



}

void Driver::stopDriver(){
    mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
    mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_1);
    mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_2);

    // gpio_set_direction((gpio_num_t) this->_hoPhase1, GPIO_MODE_OUTPUT);
    // gpio_set_level((gpio_num_t)_hoPhase1, 0);  // Set to LOW


  
};
void Driver::startDriver(){
    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_1);
    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_2);
};
