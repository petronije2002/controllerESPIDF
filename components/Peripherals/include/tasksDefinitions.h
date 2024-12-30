#ifndef PARSER_TASK_H
#define PARSER_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portable.h"
#include "esp_timer.h"




// Configure the timer for runtime stats



// #include "driver/timer.h"
// #include "driver/gptimer.h"

#include "freertos/timers.h"
#include "stdlib.h"


#define TIMER_GROUP 0           // Use hardware timer group 0
#define TIMER_IDX 0             // Use timer 0 in the group
#define TIMER_INTERVAL_US 100    // 100 microseconds



#include "freertos/semphr.h"
#include "QueueHandler.h"
#include <string>
#include "esp_log.h"


#include "USBcom.h"
#include "AS5048my.h"

#include <regex>
#include <iostream>


extern USBDevice usb_;

extern AS5048 Encoder;


extern  char inputBuffer[];  // Buffer for incoming data
extern  size_t currentDataLength ;
extern TaskHandle_t parserTaskHandle; // Handle for the parser task

extern QueueHandler queueHandler;

extern TaskHandle_t taskHandleEncoder ; // Task handle to notify


extern esp_timer_handle_t timer_handle;

// Function prototype for the parser task
void parserTask(void *param);

void commandQueueConsumerTask(void *param);


void printMultiturnAngleTask(void *param);


void parserTaskQueue(void *param);





void timer_callback(void* arg) ;
void setup_timer() ;

void print_task_stats() ;

#endif // PARSER_TASK_H
