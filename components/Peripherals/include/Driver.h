#ifndef DRIVER_H
#define DRIVER_H

#include "driver/mcpwm.h"
// #include "driver/mcpwm_prelude.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"



// #include "driver/mcpwm_timer.h"
// #include "driver/mcpwm_oper.h"
// #include "driver/mcpwm_cmpr.h"
// #include "driver/mcpwm_gen.h"
// #include "driver/mcpwm_fault.h"
// #include "driver/mcpwm_sync.h"
// #include "driver/mcpwm_cap.h"
// #include "driver/mcpwm_etm.h"

class Driver {
public:
    // Driver(int loPhase1=LO1, int hoPhase1=HO1, int loPhase2=LO2, int hoPhase2=HO2, int loPhase3=LO3, int hoPhase3=HO3);
    Driver(int loPhase1, int hoPhase1, int loPhase2, int hoPhase2, int loPhase3, int hoPhase3);

    void init();
    void setPWMDutyCycle(float dutyA, float dutyB, float dutyC) ;

    void stopDriver();
    void startDriver();




    mcpwm_config_t pwm_config;

private:
    // void setPWM(mcpwm_timer_t timer, int pin, float duty); // Change here

    int _loPhase1;
    int _hoPhase1;
    int _loPhase2;
    int _hoPhase2;
    int _loPhase3;
    int _hoPhase3;
};

#endif // DRIVER_H
