#include "QueueHandler.h"

// Constructor
QueueHandler::QueueHandler() {
    createQueues();  // Call to create queues during initialization
    queueMutex = xSemaphoreCreateMutex();  
}

// Destructor
QueueHandler::~QueueHandler() {
    // If using dynamic allocation, delete the queues here
    // For static queues, no need to delete manually
}

// Create the queues (either static or dynamic)
void QueueHandler::createQueues() {
    // For static queues
    commandQueue = xQueueCreateStatic(10, sizeof(Message), commandQueueStorage, &commandQueueBuffer);
    feedbackQueue = xQueueCreateStatic(10, sizeof(Message), feedbackQueueStorage, &feedbackQueueBuffer);
    errorQueue = xQueueCreateStatic(10, sizeof(Message), errorQueueStorage, &errorQueueBuffer);

    toSerialQueue = xQueueCreateStatic(10, sizeof(Message), toSerialQueueStorage, &toSerialQueueBuffer);

    
    // You can also create dynamic queues if needed (uncomment this if you're using dynamic queues)
    // commandQueue = xQueueCreate(10, sizeof(Message));
    // feedbackQueue = xQueueCreate(10, sizeof(Message));
    // errorQueue = xQueueCreate(10, sizeof(Message));
    
    // Check for queue creation success (use in debugging)
    if (commandQueue == NULL || feedbackQueue == NULL || errorQueue == NULL) {
        // Serial.println("Queue creation failed!");
    }
}

// Send message to a queue
bool QueueHandler::sendMessageToQueue(QueueHandle_t queue, Message &msg) {

 bool success = xQueueSend(queue, &msg, pdMS_TO_TICKS(10)) == pdPASS;

 return success;

 


  
    
}

// Receive message from a queue
bool QueueHandler::receiveMessageFromQueue(QueueHandle_t queue, Message* msg, TickType_t waitTime) {
    
            bool success = xQueueReceive(commandQueue, msg, 1000) == pdPASS;
          
            return success;
     
    
}

// Static queue buffer initialization (if using static queues)
StaticQueue_t QueueHandler::commandQueueBuffer;
StaticQueue_t QueueHandler::feedbackQueueBuffer;
StaticQueue_t QueueHandler::errorQueueBuffer;
StaticQueue_t QueueHandler::toSerialQueueBuffer;

uint8_t QueueHandler::commandQueueStorage[10 * sizeof(Message)];
uint8_t QueueHandler::feedbackQueueStorage[10 * sizeof(Message)];
uint8_t QueueHandler::errorQueueStorage[10 * sizeof(Message)];
uint8_t QueueHandler::toSerialQueueStorage[10 * sizeof(Message)];
