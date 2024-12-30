#include "tasksDefinitions.h"
#include <chrono> // For measuring time
                  // extern QueueHandler queueHandler;




// Task handle for the parser
std::string trim(const std::string &str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    return (first == std::string::npos) ? "" : str.substr(first, last - first + 1);
}


void parserTask(void *param)
{
    // Task to wait for notification and parse incoming data
    while (1)
    {
        // Wait for the notification from USB device (blocking)
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Process the data when notified
        if (currentDataLength > 0)
        {

            // Parse the incoming data here
            std::string command(inputBuffer);

            command = trim(command);
            char type = command[0];
            Message msg = {};
            std::regex pattern("^C-?\\d+(\\.\\d+)?\\sP-?\\d+(\\.\\d+)?\\sV-?\\d+(\\.\\d+)?$");

            switch (type)
            {
            case 'C': // Command message
                if (std::regex_match(command, pattern))
                {
                    msg.type = MESSAGE_TYPE_COMMAND;
                    sscanf(command.c_str(), "C%*d P%f V%f",
                           &msg.payload.commandData.position,
                           &msg.payload.commandData.velocity);
                
                    if (!queueHandler.sendMessageToQueue(queueHandler.commandQueue, msg)) {
                        usb_.printf("Failed to send command message to the queue\n");
                    } else {
                        // usb_.printf("Command message sent to the queue successfully\n");
                    };

                } else {
                    usb_.printf("NO MATCH! Invalid command format\n");
                }
                
                break;

            case 'F': // Feedback message
                msg.type = MESSAGE_TYPE_FEEDBACK;
                // Reset to default values
                msg.payload.feedbackData.currentPosition = 0.0f;
                msg.payload.feedbackData.currentVelocity = 0.0f;
                msg.payload.feedbackData.torque = 0.0f;

                sscanf(command.c_str(), "F%*d P%f V%f T%f",
                       &msg.payload.feedbackData.currentPosition,
                       &msg.payload.feedbackData.currentVelocity,
                       &msg.payload.feedbackData.torque);
                printf("Parsed Feedback: Position = %.2f, Velocity = %.2f, Torque = %.2f\n",
                       msg.payload.feedbackData.currentPosition,
                       msg.payload.feedbackData.currentVelocity,
                       msg.payload.feedbackData.torque);
                break;

            case 'E': // Error message
                msg.type = MESSAGE_TYPE_ERROR;
                msg.payload.errorData.errorCode = 0;
                msg.payload.errorData.errorMessage[0] = '\0'; // Empty string

                sscanf(command.c_str(), "E%*d %d '%[^\']'",
                       &msg.payload.errorData.errorCode,
                       msg.payload.errorData.errorMessage);
                printf("Parsed Error: Code = %d, Message = '%s'\n",
                       msg.payload.errorData.errorCode,
                       msg.payload.errorData.errorMessage);
                break;

            default:
                usb_.printf("Unknown message type, message ignored!\n");
                break;
            }

            // After processing, reset the current data length
            currentDataLength = 0;
        }
    }
}






// Testing consumer task: 

void commandQueueConsumerTask(void *param) {
    Message msg;

    while (1) {
        if (queueHandler.receiveMessageFromQueue(queueHandler.commandQueue, &msg, portMAX_DELAY)) {
            usb_.printf(" %.2f,%.2f\n",
                   msg.payload.commandData.position,
                   msg.payload.commandData.velocity);
        } else {

            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
}




   void print_task_stats() {

    char *task_stats =(char*) malloc(1024);
    if (task_stats) {
        vTaskGetRunTimeStats(task_stats);
        printf("Task Stats:\n%s\n", task_stats);
        free(task_stats);
    }
}


void printMultiturnAngleTask(void *param){

   AS5048 *sensor1 = (AS5048 *)param;

    while(1){

        // print_task_stats();




       


     



        
        usb_.printf("PRintTask: %f\n", sensor1->getMultiTurnAngle());

        vTaskDelay(pdMS_TO_TICKS(5000));

    }

}







void  timerCallback(TimerHandle_t xTimer) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Notify the task
    vTaskNotifyGiveFromISR(Encoder.encoderTaskHandle,&xHigherPriorityTaskWoken);

    // Yield to higher-priority task if necessary
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }

   
}


void timer_callback(void* arg) {


    // ESP_LOGI("TIMER", "Timer interrupt triggered!");

     BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Notify the task
    vTaskNotifyGiveFromISR(Encoder.encoderTaskHandle,&xHigherPriorityTaskWoken);

    // Yield to higher-priority task if necessary
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
    // You can also notify a task here if needed
}


void setup_timer() {
    esp_timer_create_args_t timer_args = {};


       timer_args.callback = timer_callback;      // Callback function
       timer_args.name = "my_timer" ;               // Name of the timer
   
        timer_args.dispatch_method = ESP_TIMER_TASK ;
        timer_args.skip_unhandled_events = true;
        timer_args.arg = NULL; 

    // Create the timer
    esp_timer_create(&timer_args, &timer_handle);

    // Start the timer with a period of 1000 microseconds (1 ms)
    esp_timer_start_periodic(timer_handle, 100); // 1000 µs = 1 ms interval
}

