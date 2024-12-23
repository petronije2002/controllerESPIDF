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
        ESP_LOGI("NOTIFIED", "P");

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

                //  start = std::chrono::high_resolution_clock::now();

                if (std::regex_match(command, pattern))
                {

                    msg.type = MESSAGE_TYPE_COMMAND;

                    sscanf(command.c_str(), "C%*d P%f V%f",
                           &msg.payload.commandData.position,
                           &msg.payload.commandData.velocity);

                    // usb_.printf("Message Id: %d, cpmmanded position: %f, commanded velocity: %f, \n", msg.type, msg.payload.commandData.position, msg.payload.commandData.velocity);
                
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
            usb_.printf("Received Command: Position = %.2f, Velocity = %.2f\n",
                   msg.payload.commandData.position,
                   msg.payload.commandData.velocity);
        } else {

            vTaskDelay(pdMS_TO_TICKS(1000));
            // usb_.printf("Failed to receive message from the command queue\n");
        }
    }
}

// Function for the parser task
void parserTaskQueue(void *param)
{

    QueueHandler *handler = static_cast<QueueHandler *>(param);

    char inputBuffer[128]; // Buffer for incoming data
    Message msg;

    while (1)
    {
        // Simulate receiving a string (replace with actual UART/USB reading)
        if (xQueueReceive(handler->commandQueue, &inputBuffer, portMAX_DELAY) == pdTRUE)
        {
            // Parse the incoming message
            std::string command(inputBuffer);
            char type = command[0];
            msg.id = atoi(command.substr(1, 1).c_str());

            switch (type)
            {
            case 'C': // Command message
                msg.type = MESSAGE_TYPE_COMMAND;
                sscanf(command.c_str(), "C%*d P%f V%f",
                       &msg.payload.commandData.position,
                       &msg.payload.commandData.velocity);
                printf("Parsed Command: Position = %.2f, Velocity = %.2f\n",
                       msg.payload.commandData.position,
                       msg.payload.commandData.velocity);
                break;

            case 'F': // Feedback message
                msg.type = MESSAGE_TYPE_FEEDBACK;
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
                sscanf(command.c_str(), "E%*d %d '%[^\']'",
                       &msg.payload.errorData.errorCode,
                       msg.payload.errorData.errorMessage);
                printf("Parsed Error: Code = %d, Message = '%s'\n",
                       msg.payload.errorData.errorCode,
                       msg.payload.errorData.errorMessage);
                break;

            default:
                printf("Unknown message type, message ignored!\n");
                break;
            }
        }
    }
}
