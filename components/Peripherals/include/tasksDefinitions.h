#ifndef PARSER_TASK_H
#define PARSER_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "QueueHandler.h"
#include <string>
#include "esp_log.h"

#include "USBcom.h"

#include <regex>
#include <iostream>


extern USBDevice usb_;

extern  char inputBuffer[];  // Buffer for incoming data
extern  size_t currentDataLength ;
extern TaskHandle_t parserTaskHandle; // Handle for the parser task

extern QueueHandler queueHandler;


// Function prototype for the parser task
void parserTask(void *param);

void commandQueueConsumerTask(void *param);


void parserTaskQueue(void *param);
#endif // PARSER_TASK_H
